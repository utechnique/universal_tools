//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Creating an OpenGL context is not part of the OpenGL specification. That
// means it is done differently on every platform. This header unifies different
// platform-specific tactics to create and handle an OpenGL context in one
// interface.
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_viewport.h"
//----------------------------------------------------------------------------//
#include <GL/gl.h>
#if UT_UNIX
#include <GL/glx.h>
#endif
//----------------------------------------------------------------------------//
// Extension interfaces.
#if UT_WINDOWS
#include "wglext.h"
#elif UT_UNIX
#include "glxext.h"
#endif
#include "glext.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Minimum opengl version that is supported by VE.
static const int skMinimumOpenGLVersionMajor = 4;
static const int skMinimumOpenGLVersionMinor = 1;

//----------------------------------------------------------------------------//
// List of OpenGL extension functions.
#define VE_ENUM_OPENGL_ENTRYPOINTS(__m)\
__m(glBlitFramebuffer)\
__m(glGenFramebuffers)\
__m(glDeleteFramebuffers)\
__m(glBindFramebuffer)\
__m(glFramebufferTexture)\
__m(glDrawBuffers)\
__m(glFenceSync)\
__m(glDeleteSync)\
__m(glClientWaitSync)\

//----------------------------------------------------------------------------//
// Calling convention for OpenGL functions.
#if UT_WINDOWS
#define VE_GLAPI WINAPI
#else
#define VE_GLAPI
#endif

//----------------------------------------------------------------------------//
// OpenGL function pointer types. Must have name 'OpenGL function name' + 'Ptr'.
typedef void(VE_GLAPI*glBlitFramebufferPtr) (GLint srcX0, GLint srcY0,
                                             GLint srcX1, GLint srcY1,
                                             GLint dstX0, GLint dstY0,
                                             GLint dstX1, GLint dstY1,
                                             GLbitfield mask, GLenum filter);
typedef void(VE_GLAPI*glGenFramebuffersPtr) (GLsizei n, GLuint* ids);
typedef void(VE_GLAPI*glDeleteFramebuffersPtr) (GLsizei n, const GLuint* framebuffers);
typedef void(VE_GLAPI*glBindFramebufferPtr) (GLenum target, GLuint framebuffer);
typedef void(VE_GLAPI*glFramebufferTexturePtr) (GLenum target, GLenum attachment,
                                                GLuint texture, GLint level);
typedef void(VE_GLAPI*glDrawBuffersPtr) (GLsizei n, const GLenum *bufs);
typedef GLsync(VE_GLAPI*glFenceSyncPtr) (GLenum condition, GLbitfield flags);
typedef void(VE_GLAPI*glDeleteSyncPtr) (GLsync sync);
typedef GLenum(VE_GLAPI*glClientWaitSyncPtr) (GLsync sync, GLbitfield flags, GLuint64 timeout);

//----------------------------------------------------------------------------//
// Declare all opengl functions.
#define VE_OPENGL_DECLARE_FUNCTION_PTR(__f) extern __f##Ptr __f;
VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_DECLARE_FUNCTION_PTR)

//----------------------------------------------------------------------------//
// OpenGLWindow is a wrapper around platform-specific widget associated with
// OpenGL context.
class OpenGLWindow
{
public:
	// Constructor.
	//    @param window - reference to the fltk window handle
	//    @param ownership - set 'true' to destroy window in destructor
	OpenGLWindow(Fl_Window& window, bool ownership = false);

	// Platform-specific constructor.
#if UT_WINDOWS
	//    @param window_handle - HWND handle.
	//    @param ownership - set 'true' to destroy window in destructor
	OpenGLWindow(HWND window_handle, bool ownership = false);
#elif UT_LINUX
	//    @param Display - pointer to X11 server.
	//    @param window_handle - X11 window handle.
	//    @param ownership - set 'true' to destroy window in destructor
	OpenGLWindow(Display* display_ptr, Window window_handle, bool ownership = false);
#endif

	// Destructor.
	~OpenGLWindow();

	// Move constructor.
	OpenGLWindow(OpenGLWindow&& other) noexcept;

	// Move operator.
	OpenGLWindow& operator =(OpenGLWindow&& other) noexcept;

	// Copying is prohibited.
	OpenGLWindow(const OpenGLWindow&) = delete;
	OpenGLWindow& operator =(const OpenGLWindow&) = delete;

	// Exchanges the front and back buffers.
	void SwapBuffer(bool vsync);

	// Platform-specific members.
#if UT_WINDOWS
	HWND hwnd;
	HDC hdc;
	bool window_ownership;
#elif UT_UNIX
	Display* display;
	Window handle;
	bool window_ownership;
#else
#error OpenGLWindow is not implemented.
#endif

private:
#if UT_WINDOWS
	// Sets correct pixel format for the window associated with OpenGL context.
	ut::Optional<ut::Error> SetCompatiblePixelFormat();
#endif
	// Destroys managed window if has ownership.
	void Destroy();
};

//----------------------------------------------------------------------------//
// OpenGLContext is a wrapper around platform-specific OpenGL context.
class OpenGLContext
{
public:
	// Platform-specific handle to OpenGL context.
#if UT_WINDOWS
	typedef HGLRC Handle;
#elif UT_LINUX
	typedef GLXContext Handle;
#endif

	// Constructor.
	OpenGLContext(Handle context_handle, OpenGLWindow dummy_wnd);

	// Destructor.
	~OpenGLContext();

	// Move constructor.
	OpenGLContext(OpenGLContext&& other) noexcept;

	// Move operator.
	OpenGLContext& operator =(OpenGLContext&& other) noexcept;

	// Makes managed OpenGL context the calling thread's current rendering context.
	// Uses provided window for drawing.
	ut::Optional<ut::Error> MakeCurrent(OpenGLWindow& opengl_window);

	// Makes managed OpenGL context the calling thread's current rendering context.
	// Uses managed window for drawing.
	inline ut::Optional<ut::Error> MakeCurrent()
	{
		return MakeCurrent(window);
	}

	// Copying is prohibited.
	OpenGLContext(const OpenGLContext&) = delete;
	OpenGLContext& operator =(const OpenGLContext&) = delete;

protected:
	// OpenGL context.
	Handle opengl_context;

	// Window associated with opengl context.
	OpenGLWindow window;
};

// Creates platform-specific OpenGL context and initializes global
// gl functions.
OpenGLContext CreateGLContextAndInitPlatform();

//----------------------------------------------------------------------------//
// Helper class to lock fltk thread in current scope.
class UiScopeLock
{
public:
	UiScopeLock();
	~UiScopeLock();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
