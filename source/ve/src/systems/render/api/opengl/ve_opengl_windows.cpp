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
// Constructor.
//    @param window - reference to the fltk window handle
//    @param ownership - set 'true' to destroy window in destructor
OpenGLWindow::OpenGLWindow(Fl_Window& window,
                           bool ownership) : OpenGLWindow(fl_xid(&window), ownership)
{}

// Constructor that is specific for Windows OS.
//    @param hwnd - window handle
//    @param ownership - set 'true' to destroy window in destructor
OpenGLWindow::OpenGLWindow(HWND handle, bool ownership) : hwnd(handle)
                                                      , hdc(GetDC(hwnd))
                                                      , window_ownership(ownership)
{
	// check if context was extracted correctly
	if (!hdc)
	{
		throw ut::Error(ut::error::fail, "Failed to extract DC from OpenGL window.");
	}

	// all windows must have the same pixel format
	ut::Optional<ut::Error> pixel_format_error = SetCompatiblePixelFormat();
	if (pixel_format_error)
	{
		throw ut::Error(ut::error::fail, "Failed to set pixel format for OpenGL window.");
	}
}

// Destructor.
OpenGLWindow::~OpenGLWindow()
{
	Destroy();
}

// Move constructor.
OpenGLWindow::OpenGLWindow(OpenGLWindow&& other) noexcept : hwnd(other.hwnd)
                                                          , hdc(other.hdc)
                                                          , window_ownership(other.window_ownership)
{
	other.hwnd = nullptr;
	other.hdc = nullptr;
	other.window_ownership = false;
}

// Move operator.
OpenGLWindow& OpenGLWindow::operator =(OpenGLWindow&& other) noexcept
{
	Destroy();
	hwnd = other.hwnd;
	hdc = other.hdc;
	window_ownership = other.window_ownership;
	other.hwnd = nullptr;
	other.hdc = nullptr;
	other.window_ownership = false;
	return *this;
}

// Exchanges the front and back buffers.
void OpenGLWindow::SwapBuffer(bool vsync)
{
	if (vsync)
	{
		SwapBuffers(hdc);
	}
}

// Sets correct pixel format for the window associated with OpenGL context.
ut::Optional<ut::Error> OpenGLWindow::SetCompatiblePixelFormat()
{
	// pixel format descriptor for the context
	PIXELFORMATDESCRIPTOR pixel_format_desc;
	ZeroMemory(&pixel_format_desc, sizeof(pixel_format_desc));
	pixel_format_desc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixel_format_desc.nVersion = 1;
	pixel_format_desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixel_format_desc.iPixelType = PFD_TYPE_RGBA;
	pixel_format_desc.cColorBits = 32;
	pixel_format_desc.cDepthBits = 0;
	pixel_format_desc.cStencilBits = 0;
	pixel_format_desc.iLayerType = PFD_MAIN_PLANE;

	// set the pixel format and create the context
	ut::int32 PixelFormat = ChoosePixelFormat(hdc, &pixel_format_desc);
	if (!PixelFormat || !SetPixelFormat(hdc, PixelFormat, &pixel_format_desc))
	{
		return ut::Error(ut::error::fail);
	}

	// success
	return ut::Optional<ut::Error>();
}

// Destroys managed window if has ownership.
void OpenGLWindow::Destroy()
{
	if (hdc != nullptr)
	{
		ReleaseDC(hwnd, hdc);
	}

	if (window_ownership && hwnd != nullptr)
	{
		DestroyWindow(hwnd);
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
		wglDeleteContext(opengl_context);
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
	BOOL make_curr_result = wglMakeCurrent(another_window.hdc, opengl_context);
	if (!make_curr_result)
	{
		return ut::Error(ut::error::fail, "wglMakeCurrent() failed.");
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
__f = reinterpret_cast<__f##Ptr>(wglGetProcAddress(#__f)); \
if (__f == nullptr) throw ut::Error(ut::error::not_supported, #__f);

//----------------------------------------------------------------------------//
// If wglCreateContextAttribsARB succeeds, it initializes the context
// to the initial state defined by the OpenGL specification, and
// returns a handle to it.The handle can be used(via wglMakeCurrent
// or wglMakeContextCurrentARB) with any HDC sharing the same pixel
// format as <hDC>, and created on the same device, subject to
// constraints imposed by the API version.
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

// Window procedure for dummy opengl window.
LRESULT CALLBACK DummyOpenGLWindowProc(HWND hWnd, DWORD Message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

// Creates simple hidden window to associate 'windowless' context with it.
ut::Result<OpenGLWindow, ut::Error> CreateOpenGLDummyWindow()
{
	// register a dummy window class.
	const TCHAR* dummy_wnd_class_name = TEXT("dummy_opengl_window");
	static bool initialized_dummy_wnd_class = false;
	if (!initialized_dummy_wnd_class)
	{
		WNDCLASS wc;
		initialized_dummy_wnd_class = true;
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = reinterpret_cast<WNDPROC>(DummyOpenGLWindowProc);
		wc.hbrBackground = (HBRUSH)(COLOR_MENUTEXT);
		wc.lpszClassName = dummy_wnd_class_name;
		ATOM class_atom = ::RegisterClass(&wc);
		if (!class_atom)
		{
			return ut::MakeError(ut::Error(ut::error::fail, "Failed to register class of the dummy window."));
		}
	}

	// create a dummy window
	HWND dummy_wnd = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		dummy_wnd_class_name,
		NULL,
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);
	if (!dummy_wnd)
	{
		return ut::MakeError(ut::Error(ut::error::fail, "Failed to create dummy window."));
	}

	// success
	return OpenGLWindow(dummy_wnd, true);
}

// Creates simple context using no attributes.
ut::Result<OpenGLContext, ut::Error> CreateDummyOpenGLContext()
{
	// create dummy window
	ut::Result<OpenGLWindow, ut::Error> dummy_window_result = CreateOpenGLDummyWindow();
	if (!dummy_window_result)
	{
		return ut::MakeError(dummy_window_result.MoveAlt());
	}

	// create opengl context
	OpenGLContext::Handle opengl_context = wglCreateContext(dummy_window_result.GetResult().hdc);
	if (!opengl_context)
	{
		return ut::MakeError(ut::Error(ut::error::fail, "Failed to create dummy wgl context."));
	}

	// success
	return OpenGLContext(opengl_context, dummy_window_result.MoveResult());
}

// Creates platform-specific OpenGL context.
ut::Result<OpenGLContext, ut::Error> CreateOpenGLContext()
{
	// create dummy window
	ut::Result<OpenGLWindow, ut::Error> dummy_window_result = CreateOpenGLDummyWindow();
	if (!dummy_window_result)
	{
		return ut::MakeError(dummy_window_result.MoveAlt());
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
	OpenGLContext::Handle opengl_context = wglCreateContextAttribsARB(dummy_window_result.GetResult().hdc, nullptr, attrib_list);
	if (!opengl_context)
	{
		ut::String desc = "Failed to create OpenGL context, version ";
		desc += ut::Print(skMinimumOpenGLVersionMajor) + ut::String(".") + ut::Print(skMinimumOpenGLVersionMinor);
		desc += " is not supported.";
		return ut::MakeError(ut::Error(ut::error::not_supported, ut::Move(desc)));
	}

	// success
	return OpenGLContext(opengl_context, dummy_window_result.MoveResult());
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

	// function 'wglCreateContextAttribsARB' must be loaded before creating actual context
	if (!wglCreateContextAttribsARB)
	{
		// create dummy context
		ut::Result<OpenGLContext, ut::Error> dummy_context_result = CreateDummyOpenGLContext();
		if (!dummy_context_result)
		{
			throw ut::Error(dummy_context_result.MoveAlt());
		}

		// apply dummy context
		OpenGLContext& dummy_context = dummy_context_result.GetResult();
		ut::Optional<ut::Error> make_current_error = dummy_context.MakeCurrent();
		if (make_current_error)
		{
			throw ut::Error(make_current_error.Move());
		}

		// load 'wglCreateContextAttribsARB' function
		wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
		if (!wglCreateContextAttribsARB)
		{
			throw ut::Error(ut::error::fail, "Failed to load wglGetProcAddress() function.");
		}

		// detach dummy context
		wglMakeCurrent(nullptr, nullptr);
	}

	// create real context to check opengl version
	ut::Result<OpenGLContext, ut::Error> final_context_result = CreateOpenGLContext();
	if (!final_context_result)
	{
		throw ut::Error(final_context_result.MoveAlt());
	}

	// apply context
	OpenGLContext& final_context = final_context_result.GetResult();
	ut::Optional<ut::Error> make_current_error = final_context.MakeCurrent();
	if (make_current_error)
	{
		throw ut::Error(make_current_error.Move());
	}

	// initialize all entry points required by render engine
	VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_INIT_FUNCTION)

	// fully initialized
	initialized = true;

	// detach context
	wglMakeCurrent(nullptr, nullptr);

	// success
	return final_context_result.MoveResult();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL && UT_WINDOWS
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//