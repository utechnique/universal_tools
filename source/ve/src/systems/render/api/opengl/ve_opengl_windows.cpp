//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL && UT_WINDOWS
//----------------------------------------------------------------------------//
#include "FL/x.H" // to get fltk window handle
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
__f = reinterpret_cast<__f##Ptr>(wglGetProcAddress(#__f)); \
if (__f == nullptr) return ut::Error(ut::error::not_supported, #__f);

//----------------------------------------------------------------------------//
// If wglCreateContextAttribsARB succeeds, it initializes the context
// to the initial state defined by the OpenGL specification, and
// returns a handle to it.The handle can be used(via wglMakeCurrent
// or wglMakeContextCurrentARB) with any HDC sharing the same pixel
// format as <hDC>, and created on the same device, subject to
// constraints imposed by the API version.
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

//----------------------------------------------------------------------------//
// Constructor.
OpenGLDummyWindow::OpenGLDummyWindow() : Fl_Window(0, 0, 1, 1)
                                       , hwnd(0)
                                       , hdc(0)
                                       , opengl_context(0)
                                       , initialized(false)
{}

// Destructor. Windows resources are destroyed here.
OpenGLDummyWindow::~OpenGLDummyWindow()
{
	// device context
	if (hdc)
	{
		ReleaseDC(hwnd, hdc);
	}

	// OpenGL context
	if (opengl_context)
	{
		wglDeleteContext(opengl_context);
	}
}

// Initializes global-space opengl functions and an opengl context for
// rendering in VE. This method is called from Fl_Window::draw() between
// gl_start() and gl_finish(). It's a place where one can steal current
// context from fltk and use it as a parent for a new one. Initialization
// code is specific for Windows OS.
ut::Optional<ut::Error> OpenGLDummyWindow::Initialize(OpenGLContextHandle shared_context)
{
	// get a handle of the current window it will be needed in destructor
	hwnd = fl_xid(this);
	if (!hwnd)
	{
		return ut::Error(ut::error::fail, "fl_xid() failed.");
	}

	// get windows device context it is needed for wglMakeCurrent() function
	hdc = wglGetCurrentDC();
	if (!hdc)
	{
		return ut::Error(ut::error::fail, "GetDC() failed.");
	}

	// make shared context current so that wglCreateContextAttribsARB() could be loaded
	if (!wglMakeCurrent(hdc, shared_context))
	{
		return ut::Error(ut::error::fail, "wglMakeCurrent() failed.");
	}

	// load 'wglCreateContextAttribsARB' function
	wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
	if (!wglCreateContextAttribsARB)
	{
		return ut::Error(ut::error::fail, "Failed to load wglCreateContextAttribsARB() function.");
	}

	// reset shared context so that we could create a new (final) one
	if (!wglMakeCurrent(nullptr, nullptr))
	{
		return ut::Error(ut::error::fail, "wglMakeCurrent(0,0) failed.");
	}

	// debug options
#if DEBUG
	int dbg_flag = WGL_CONTEXT_DEBUG_BIT_ARB;
#else
	int dbg_flag = 0;
#endif

	// context attributes (must include minor and major version)
	int attrib_list[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, skMinimumOpenGLVersionMajor,
		WGL_CONTEXT_MINOR_VERSION_ARB, skMinimumOpenGLVersionMinor,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | dbg_flag,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	// create opengl context
	opengl_context = wglCreateContextAttribsARB(hdc, shared_context, attrib_list);
	if (!opengl_context)
	{
		ut::String desc = "Failed to create OpenGL context, version ";
		desc += ut::Print(skMinimumOpenGLVersionMajor) + ut::String(".") + ut::Print(skMinimumOpenGLVersionMinor);
		desc += " is not supported.";
		return ut::Error(ut::error::not_supported, ut::Move(desc));
	}

	// apply context so that we could load opengl functions
	ut::Optional<ut::Error> apply_error = ApplyContext();
	if (apply_error)
	{
		return apply_error.Move();
	}

	// initialize all entry points required by render engine
	VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_INIT_FUNCTION)

	// success
	return ut::Optional<ut::Error>();
}

// Returns current opengl context or error if something failed.
ut::Result<OpenGLContextHandle, ut::Error> OpenGLDummyWindow::GetCurrentContext()
{
	OpenGLContextHandle handle = wglGetCurrentContext();
	if (!handle)
	{
		return ut::MakeError(ut::Error(ut::error::fail, "wglGetCurrentContext() failed."));
	}
	return handle;
}

// Makes managed OpenGL context the calling thread's current rendering context.
//    @return - optional error if failed to set context.
ut::Optional<ut::Error> OpenGLDummyWindow::ApplyContext()
{
	if (!wglMakeCurrent(hdc, opengl_context))
	{
		return ut::Error(ut::error::fail, "wglMakeCurrent() failed.");
	}
	return ut::Optional<ut::Error>();
}

// Resets OpenGL context in a current thread.
//    @return - optional error if failed to reset context.
ut::Optional<ut::Error> OpenGLDummyWindow::ResetContext()
{
	if (!wglMakeCurrent(nullptr, nullptr))
	{
		return ut::Error(ut::error::fail, "wglMakeCurrent(0,0) failed.");
	}
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL && UT_WINDOWS
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//