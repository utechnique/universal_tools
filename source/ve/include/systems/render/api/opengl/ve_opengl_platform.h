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
#include "ut.h"
//----------------------------------------------------------------------------//
// Platform-specific headers.
#if UT_WINDOWS
#include <windows.h>
#elif UT_UNIX
#include <GL/glx.h>
#endif
//----------------------------------------------------------------------------//
// OpenGL main header.
#include <GL/gl.h>
//----------------------------------------------------------------------------//
// Extension interfaces.
#if UT_WINDOWS
#include "wglext.h"
#endif
#include "glext.h"
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

//----------------------------------------------------------------------------//
// Declare all opengl functions.
#define VE_OPENGL_DECLARE_FUNCTION_PTR(__f) extern __f##Ptr __f;
VE_ENUM_OPENGL_ENTRYPOINTS(VE_OPENGL_DECLARE_FUNCTION_PTR)

//----------------------------------------------------------------------------//
// OpenGLWindow is a wrapper around platform-specific widget associated with
// OpenGL context.
struct OpenGLWindow
{
	// Constructor.
#if UT_WINDOWS
	//    @param hwnd - window handle
	//    @param hdc - handle to a device context for the client area of @window
	//    @param ownership - set 'true' to destroy window in destructor
	OpenGLWindow(HWND hwnd, bool ownership = false);
#elif UT_LINUX
	OpenGLWindow();
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

	// Platform-specific members.
#if UT_WINDOWS
	HWND handle;
	HDC context;
	bool window_ownership;
#elif UT_UNIX

#else
#error OpenGLWindow is not implemented.
#endif

private:
	// Destroys managed window if has ownership.
	void Destroy();
};

//----------------------------------------------------------------------------//
// OpenGLContext is a wrapper around platform-specific OpenGL context.
class OpenGLContext
{
public:
	// Constructor.
	OpenGLContext(HGLRC context_handle, OpenGLWindow dummy_wnd);

	// Destructor.
	~OpenGLContext();

	// Move constructor.
	OpenGLContext(OpenGLContext&& other) noexcept;

	// Move operator.
	OpenGLContext& operator =(OpenGLContext&& other) noexcept;

	// Makes managed OpenGL context the calling thread's current rendering context.
	// Uses provided window for drawing.
	void MakeCurrent(OpenGLWindow& opengl_window);

	// Makes managed OpenGL context the calling thread's current rendering context.
	// Uses managed window for drawing.
	inline void MakeCurrent()
	{
		MakeCurrent(window);
	}

	// Copying is prohibited.
	OpenGLContext(const OpenGLContext&) = delete;
	OpenGLContext& operator =(const OpenGLContext&) = delete;

protected:
	// OpenGL context.
	HGLRC opengl_context;

	// Window associated with opengl context.
	OpenGLWindow window;
};

// Sets correct pixel format for the window associated with OpenGL context.
#if UT_WINDOWS
ut::Optional<ut::Error> SetOpenGLViewportPixelFormat(HDC hdc);
#endif

// Creates platform-specific OpenGL context.
ut::Result<OpenGLContext, ut::Error> CreateOpenGLContext();

// Platform-specific check for the desired OpenGL version and initialization
// of the entry points for OpenGL functions.
ut::Optional<ut::Error> InitOpenGLPlatform();

//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//