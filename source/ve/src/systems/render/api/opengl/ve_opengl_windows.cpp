//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
// Constructor.
//    @param hwnd - window handle
//    @param hdc - handle to a device context for the client area of @window
//    @param ownership - set 'true' to destroy window in destructor
OpenGLWindow::OpenGLWindow(HWND hwnd, bool ownership) : handle(hwnd)
                                                      , context(GetDC(hwnd))
                                                      , window_ownership(ownership)
{
	// check if context was extracted correctly
	if (!context)
	{
		throw ut::Error(ut::error::fail, "Failed to create DC for OpenGL window.");
	}

	// all windows must have the same pixel format
	ut::Optional<ut::Error> pixel_format_error = SetOpenGLViewportPixelFormat(context);
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
OpenGLWindow::OpenGLWindow(OpenGLWindow&& other) noexcept : handle(other.handle)
                                                          , context(other.context)
                                                          , window_ownership(other.window_ownership)
{
	other.handle = nullptr;
	other.context = nullptr;
	other.window_ownership = false;
}

// Move operator.
OpenGLWindow& OpenGLWindow::operator =(OpenGLWindow&& other) noexcept
{
	Destroy();
	handle = other.handle;
	context = other.context;
	window_ownership = other.window_ownership;
	other.handle = nullptr;
	other.context = nullptr;
	other.window_ownership = false;
	return *this;
}

// Destroys managed window if has ownership.
void OpenGLWindow::Destroy()
{
	if (context != nullptr)
	{
		ReleaseDC(handle, context);
	}

	if (window_ownership && handle != nullptr)
	{
		DestroyWindow(handle);
	}
}

//----------------------------------------------------------------------------//
// Constructor.
OpenGLContext::OpenGLContext(HGLRC context_handle,
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
void OpenGLContext::MakeCurrent(OpenGLWindow& another_window)
{
	BOOL make_curr_result = wglMakeCurrent(another_window.context, opengl_context);
	if (!make_curr_result)
	{
		make_curr_result = wglMakeCurrent(nullptr, nullptr);
	}
}

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
	HGLRC opengl_context = wglCreateContext(dummy_window_result.GetResult().context);
	if (!opengl_context)
	{
		return ut::MakeError(ut::Error(ut::error::fail, "Failed to create dummy wgl context."));
	}

	// success
	return OpenGLContext(opengl_context, dummy_window_result.MoveResult());
}

// Sets correct pixel format for the window associated with OpenGL context.
ut::Optional<ut::Error> SetOpenGLViewportPixelFormat(HDC hdc)
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

	// context attributes
	int AttribList[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, skMinimumOpenGLVersionMajor,
		WGL_CONTEXT_MINOR_VERSION_ARB, skMinimumOpenGLVersionMinor,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | dbg_flag,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	// create opengl context
	HGLRC opengl_context = wglCreateContextAttribsARB(dummy_window_result.GetResult().context, nullptr, AttribList);
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

// Platform-specific check for the desired OpenGL version and initialization
// of the entry points for OpenGL functions.
ut::Optional<ut::Error> InitOpenGLPlatform()
{
	static bool initialized = false;
	if (!initialized)
	{
		// function 'wglCreateContextAttribsARB' must be loaded before creating actual context
		if (!wglCreateContextAttribsARB)
		{
			// create dummy context
			ut::Result<OpenGLContext, ut::Error> dummy_context_result = CreateDummyOpenGLContext();
			if (!dummy_context_result)
			{
				return dummy_context_result.MoveAlt();
			}

			// apply dummy context
			OpenGLContext& dummy_context = dummy_context_result.GetResult();
			dummy_context.MakeCurrent();

			// load 'wglCreateContextAttribsARB' function
			wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
			if (!wglCreateContextAttribsARB)
			{
				return ut::Error(ut::error::fail, "Failed to load wglGetProcAddress() function.");
			}
		}

		// create real context to check opengl version
		ut::Result<OpenGLContext, ut::Error> test_context_result = CreateOpenGLContext();
		if (!test_context_result)
		{
			return test_context_result.MoveAlt();
		}

		// apply context
		OpenGLContext& test_context = test_context_result.GetResult();
		test_context.MakeCurrent();

		// initialize all entry points required by render engine
		VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_INIT_FUNCTION)

		// fully initialized
		initialized = true;
	}

	// detach test context
	wglMakeCurrent(nullptr, nullptr);

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//