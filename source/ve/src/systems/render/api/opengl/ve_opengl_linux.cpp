//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL && UT_LINUX
//----------------------------------------------------------------------------//
#include "FL/x.H" // to get fltk display
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
//    @param window - reference to the fltk window handle
//    @param ownership - set 'true' to destroy window in destructor
OpenGLWindow::OpenGLWindow(Fl_Window& window,
                           bool ownership) : OpenGLWindow(fl_display,
                                                          fl_xid(&window),
                                                          ownership)
{}

// Constructor that is specific for X11 on Linux.
//    @param Display - pointer to X11 server.
//    @param window_handle - X11 window handle.
//    @param ownership - set 'true' to destroy window in destructor
OpenGLWindow::OpenGLWindow(Display* display_ptr,
                           Window window_handle,
                           bool ownership) : display(display_ptr)
                                           , handle(window_handle)
                                           , window_ownership(ownership)
{}

// Destructor.
OpenGLWindow::~OpenGLWindow()
{
	Destroy();
}

// Move constructor.
OpenGLWindow::OpenGLWindow(OpenGLWindow&& other) noexcept : handle(other.handle)
                                                          , display(other.display)
                                                          , window_ownership(other.window_ownership)
{
	other.window_ownership = false;
}

// Move operator.
OpenGLWindow& OpenGLWindow::operator =(OpenGLWindow&& other) noexcept
{
	Destroy();
	display = other.display;
	handle = other.handle;
	window_ownership = other.window_ownership;
	other.window_ownership = false;
	return *this;
}

// Exchanges the front and back buffers.
void OpenGLWindow::SwapBuffer(bool vsync)
{
	if (vsync)
	{
		glXSwapBuffers(display, handle);
	}
}

// Destroys managed window if has ownership.
void OpenGLWindow::Destroy()
{
	if (window_ownership && handle)
	{
		XDestroyWindow(display, handle);
	}
}

//----------------------------------------------------------------------------//
// Constructor.
OpenGLContext::OpenGLContext(Handle context_handle,
                             OpenGLWindow dummy_wnd) : opengl_context(context_handle)
                                                     , window(ut::Move(dummy_wnd))
{}

// Destructor.
OpenGLContext::~OpenGLContext()
{
	if (opengl_context != nullptr)
	{
		glXDestroyContext(window.display, opengl_context);
	}
}

// Move constructor.
OpenGLContext::OpenGLContext(OpenGLContext&& other) noexcept : opengl_context(other.opengl_context)
                                                             , window(ut::Move(other.window))
{
	other.opengl_context = nullptr;
}

// Move operator.
OpenGLContext& OpenGLContext::operator =(OpenGLContext&& other) noexcept
{
	opengl_context = other.opengl_context;
	window = ut::Move(other.window);
	other.opengl_context = nullptr;
	return *this;
}

// Makes managed OpenGL context the calling thread's current rendering context.
// Uses provided window for drawing.
ut::Optional<ut::Error> OpenGLContext::MakeCurrent(OpenGLWindow& another_window)
{
	if (!glXMakeCurrent(another_window.display, another_window.handle, opengl_context))
	{
		return ut::Error(ut::error::fail, "glXMakeCurrent() failed.");
	}
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
// Define all opengl functions.
#define VE_OPENGL_DEFINE_FUNCTION_PTR(__f) __f##Ptr __f = nullptr;
VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_DEFINE_FUNCTION_PTR)

//----------------------------------------------------------------------------//
// Macro to initialize OpenGL function.
#define VE_OPENGL_INIT_FUNCTION(__f) \
__f = reinterpret_cast<__f##Ptr>(glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(#__f))); \
if (__f == nullptr) throw ut::Error(ut::error::not_supported, #__f);

//----------------------------------------------------------------------------//
// If glXCreateContextAttribsARB succeeds, it initializes the context
// to the initial state defined by the OpenGL specification, and
// returns a handle to it.This handle can be used to render to any GLX
// surface(window, pixmap, or pbuffer) compatible with <config>,
// subject to constraints imposed by the OpenGL API version of the
// context.
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;

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
	const char *where, *terminator;

	// Extension names should not have spaces.
	where = strchr(extension, ' ');
	if (where || *extension == '\0')
	{
		return false;
	}

	// It takes a bit of care to be fool-proof about parsing the
	// OpenGL extensions string. Don't be fooled by sub-strings,
	// etc.
	for (start = ext_list;;)
	{
		where = strstr(start, extension);

		if (!where)
		{
			break;
		}

		terminator = where + strlen(extension);

		if (where == start || *(where - 1) == ' ')
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

// Creates platform-specific OpenGL context.
ut::Result<OpenGLContext, ut::Error> CreateOpenGLContext()
{
	// fltk window data
	Display* display = fl_display;

	// Get a matching FB config
	static int visual_attribs[] =
	{
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		//GLX_ALPHA_SIZE      , 8,
		//GLX_DEPTH_SIZE      , 24,
		//GLX_STENCIL_SIZE    , 8,
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
		return ut::MakeError(ut::Error(ut::error::fail, "glXChooseFBConfig() failed."));
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

	// uncomment this section to create dummy window
	/*
	// create color map for dummy window
	XVisualInfo *vi = glXGetVisualFromFBConfig(display, best_fbc);
	XSetWindowAttributes swa;
	Colormap cmap;
	swa.colormap = cmap = XCreateColormap(display, RootWindow(display, vi->screen),
	vi->visual, AllocNone);
	swa.background_pixmap = None;
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask;

	// create dummy window
	Window window = XCreateWindow(display, RootWindow(display, vi->screen),
        0, 0, 100, 100, 0, vi->depth, InputOutput,
        vi->visual,
        CWBorderPixel | CWColormap | CWEventMask, &swa);
	if (!window)
	{
        return ut::MakeError(ut::Error(ut::error::fail, "Failed to create OpenGL dummy window."));
	}

	// done with the visual info data
	XFree(vi);

	// finish dummy window
	XStoreName(display, window, "GL Dummy Window");
	XMapWindow(display, window);
    */

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
		return ut::MakeError(ut::Error(ut::error::not_supported, "glXCreateContextAttribsARB is not supported."));
	}

	// load glXCreateContextAttribsARB function
	const GLubyte* proc_name = reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB");
	glXCreateContextAttribsARB = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(glXGetProcAddressARB(proc_name));
	if (!glXCreateContextAttribsARB)
	{
		return ut::MakeError(ut::Error(ut::error::not_supported, "glXCreateContextAttribsARB not found."));
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
	GLXContext opengl_context = glXCreateContextAttribsARB(display, best_fbc, 0, True, context_attribs);

	// sync to ensure any errors generated are processed.
	XSync(display, False);
	if (g_opengl_error_occured || !opengl_context)
	{
		return ut::MakeError(ut::Error(ut::error::fail, "glXCreateContextAttribsARB() failed (unsupported OpenGL version)."));
	}

	// restore the original error handler
	XSetErrorHandler(old_error_handler);

	// initialize dummy window wrapper
	OpenGLWindow dummy_window(display, None, true);

	// success
	return OpenGLContext(opengl_context, ut::Move(dummy_window));
}

// Creates platform-specific OpenGL context and initializes global
// gl functions.
OpenGLContext CreateGLContextAndInitPlatform()
{
    static bool initialized = false;
	if (initialized)
	{
		throw ut::Error(ut::error::already_exists);
	}

    // Warning! FLTK must be initialized up to here!
	ui::DisplayScopeLock ui_lock;

	// FBConfigs were added in GLX version 1.3
	int glx_major, glx_minor;
	if (!glXQueryVersion(fl_display, &glx_major, &glx_minor) ||
		((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
	{
		throw ut::Error(ut::error::not_supported, "GLX version is too old.");
	}

	// initialize all entry points required by render engine
	VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_INIT_FUNCTION)

    // create context
	ut::Result<OpenGLContext, ut::Error> context = CreateOpenGLContext();
	if (!context)
	{
		throw ut::Error(context.MoveAlt());
	}

    // fully initialized
	initialized = true;

    // success
	return context.MoveResult();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL && UT_LINUX
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
