//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL && UT_LINUX
//----------------------------------------------------------------------------//
#include "FL/x.H" // to get fltk display handle
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Define all opengl functions.
#define VE_OPENGL_DEFINE_FUNCTION_PTR(__f) __f##Ptr __f = nullptr;
VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_DEFINE_FUNCTION_PTR)

//----------------------------------------------------------------------------//
// Macro to initialize OpenGL function.
#define VE_OPENGL_INIT_FUNCTION(__f) \
__f = reinterpret_cast<__f##Ptr>(glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(#__f))); \
if (__f == nullptr) return ut::Error(ut::error::not_supported, #__f);

//----------------------------------------------------------------------------//
// If glXCreateContextAttribsARB succeeds, it initializes the context
// to the initial state defined by the OpenGL specification, and
// returns a handle to it.This handle can be used to render to any GLX
// surface(window, pixmap, or pbuffer) compatible with <config>,
// subject to constraints imposed by the OpenGL API version of the
// context.
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;

//----------------------------------------------------------------------------//
// Helper function to handle OpenGL errors.
static bool g_opengl_error_occured = false;
static int OpenGLErrorHandler(Display* dpy, XErrorEvent* ev)
{
	g_opengl_error_occured = true;
	return 0;
}

// Helper function to check for extension string presence.
static bool IsGLExtensionSupported(const char *ext_list, const char *extension)
{
	const char *start;
	const char *place, *terminator;

	// Extension names should not have spaces.
	place = strchr(extension, ' ');
	if (place || *extension == '\0')
	{
		return false;
	}

	// It takes a bit of care to be fool-proof about parsing the
	// OpenGL extensions string. Don't be fooled by sub-strings,
	// etc.
	for (start = ext_list;;)
	{
		place = strstr(start, extension);
		if (!place)
		{
			break;
		}

		terminator = place + strlen(extension);

		if (place == start || *(place - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0')
			{
				return true;
			}
		}

		start = terminator;
	}

	return false;
}

//----------------------------------------------------------------------------//
// Constructor.
OpenGLDummyWindow::OpenGLDummyWindow() : Fl_Window(0, 0, 1, 1)
                                       , display(nullptr)
                                       , opengl_context(0)
                                       , initialized(false)
{}

// Destructor. Platform-specific resources are destroyed here.
OpenGLDummyWindow::~OpenGLDummyWindow()
{
	if (display && opengl_context)
	{
		glXDestroyContext(display, opengl_context);
	}
}

// Initializes global-space opengl functions and an opengl context for
// rendering in VE. This method is called from Fl_Window::draw() between
// gl_start() and gl_finish(). It's a place where one can steal current
// context from fltk and use it as a parent for a new one. Initialization
// code is specific for Linux OS.
ut::Optional<ut::Error> OpenGLDummyWindow::Initialize(OpenGLContextHandle shared_context)
{
	// enable multithreading for xlib
    Status x_thread_status = XInitThreads();
    if(!x_thread_status)
    {
        return ut::Error(ut::error::fail, "XInitThreads() failed.");
    }

	// get a display from fltk
	display = fl_display;
	if (!display)
	{
		return ut::Error(ut::error::fail, "fl_display has invalid value.");
	}

	// get a matching FB config
	static int visual_attribs[] =
	{
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		//GLX_SAMPLE_BUFFERS  , 1,
		//GLX_SAMPLES         , 4,
		None
	};

	// getting matching framebuffer configs
	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
	if (!fbc)
	{
		return ut::Error(ut::error::fail, "glXChooseFBConfig() failed.");
	}

	// pick the FB config/visual with the most samples per pixel
	int best_fbc_id = -1, worst_fbc_id = -1, best_num_samp = -1, worst_num_samp = 999, fb_iterator;
	for (fb_iterator = 0; fb_iterator < fbcount; ++fb_iterator)
	{
		XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[fb_iterator]);
		if (vi)
		{
			int samp_buf, samples;
			glXGetFBConfigAttrib(display, fbc[fb_iterator], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(display, fbc[fb_iterator], GLX_SAMPLES, &samples);

			if (best_fbc_id < 0 || samp_buf && samples > best_num_samp)
			{
				best_fbc_id = fb_iterator, best_num_samp = samples;
			}

			if (worst_fbc_id < 0 || !samp_buf || samples < worst_num_samp)
			{
				worst_fbc_id = fb_iterator, worst_num_samp = samples;
			}
		}
		XFree(vi);
	}
	GLXFBConfig best_fbc = fbc[best_fbc_id];

	// be sure to free the FBConfig list allocated by glXChooseFBConfig()
	XFree(fbc);

	// Install an X error handler so the application won't exit if GL 3.0
	// context allocation fails.
	//
	// Note this error handler is global.  All display connections in all threads
	// of a process use the same error handler, so be sure to guard against other
	// threads issuing X commands while this code is running.
	g_opengl_error_occured = false;
	int(*old_error_handler)(Display*, XErrorEvent*) = XSetErrorHandler(&OpenGLErrorHandler);

	// get the fltk screen's GLX extension list
	const char *glx_exts = glXQueryExtensionsString(display, DefaultScreen(display));

	// check if glXCreateContextAttribsARB is supported
	bool arb_context_supported = IsGLExtensionSupported(glx_exts, "GLX_ARB_create_context");
	if (!arb_context_supported)
	{
		return ut::Error(ut::error::not_supported, "glXCreateContextAttribsARB is not supported.");
	}

	// load glXCreateContextAttribsARB function
	const GLubyte* proc_name = reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB");
	glXCreateContextAttribsARB = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(glXGetProcAddressARB(proc_name));
	if (!glXCreateContextAttribsARB)
	{
		return ut::Error(ut::error::not_supported, "glXCreateContextAttribsARB not found.");
	}

	// context attributes (must include minor and major version)
	int context_attribs[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, skMinimumOpenGLVersionMajor,
		GLX_CONTEXT_MINOR_VERSION_ARB, skMinimumOpenGLVersionMinor,
		GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	// finally opengl context can be created
	opengl_context = glXCreateContextAttribsARB(display, best_fbc, shared_context, True, context_attribs);

	// sync to ensure any errors generated are processed.
	XSync(display, False);
	if (g_opengl_error_occured || !opengl_context)
	{
		return ut::Error(ut::error::fail, "glXCreateContextAttribsARB() failed (unsupported OpenGL version).");
	}

    // restore the original error handler
	XSetErrorHandler(old_error_handler);

	// initialize all entry points required by render engine
	VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_INIT_FUNCTION)

	// success
	return ut::Optional<ut::Error>();
}

// Returns current opengl context or error if something failed.
ut::Result<OpenGLContextHandle, ut::Error> OpenGLDummyWindow::GetCurrentContext()
{
	OpenGLContextHandle handle = glXGetCurrentContext();
	if (!handle)
	{
		return ut::MakeError(ut::Error(ut::error::fail, "glXGetCurrentContext() failed."));
	}
	return handle;
}

// Makes managed OpenGL context the calling thread's current rendering context.
ut::Optional<ut::Error> OpenGLDummyWindow::ApplyContext()
{
	FltkScopeLock lock; // fltk thread must be locked, xlib isn't thread-safe
    Window wnd = fl_xid(this);
	if (!glXMakeCurrent(fl_display, None, opengl_context))
	{
		return ut::Error(ut::error::fail, "glXMakeCurrent() failed.");
	}
	return ut::Optional<ut::Error>();
}

// Resets OpenGL context in a current thread.
//    @return - optional error if failed to reset context.
ut::Optional<ut::Error> OpenGLDummyWindow::ResetContext()
{
	FltkScopeLock lock; // fltk thread must be locked, xlib isn't thread-safe
	if (!glXMakeCurrent(display, None, None))
	{
		return ut::Error(ut::error::fail, "glXMakeCurrent(0,0) failed.");
	}
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL && UT_LINUX
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
