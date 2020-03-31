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
// Platform-specific handle to an opengl context.
#if UT_WINDOWS
typedef HGLRC OpenGLContextHandle;
#elif UT_LINUX
typedef GLXContext OpenGLContextHandle;
#endif

//----------------------------------------------------------------------------//
// OpenGl context needs a window (windows os) or display (X11) for drawing
// stuff. ve::render::OpenGLDummyWindow class provides a simple window that
// has no purpose other than just hosting a context.
class OpenGLDummyWindow : public Fl_Window
{
public:
	// Constructor.
	OpenGLDummyWindow();

	// Destructor. Platform-specific resources are destroyed here.
	~OpenGLDummyWindow();

	// Initializes global-space opengl functions and an opengl context for
	// rendering in VE. This method is called from Fl_Window::draw() between
	// gl_start() and gl_finish(). It's a place where one can steal current
	// context from fltk and use it as a parent for a new one. Initialization
	// code is platform-specific.
	ut::Optional<ut::Error> Initialize(OpenGLContextHandle shared_context);

	// Returns current opengl context or error if something failed.
	ut::Result<OpenGLContextHandle, ut::Error> GetCurrentContext();

	// Makes managed OpenGL context the calling thread's current rendering context.
	//    @return - optional error if failed to set context.
	ut::Optional<ut::Error> ApplyContext();

	// Resets OpenGL context in a current thread.
	//    @return - optional error if failed to reset context.
	ut::Optional<ut::Error> ResetContext();

	// Returns true if opengl context was created and initialized.
	// This function is thread-safe.
	bool IsInitialized();

	// The only purpose for draw() method is to make fltk create an opengl context.
	// Then this context can be used as a shared parent for our context so that
	// we could draw opengl stuff in other windows (that have own context).
	void draw() override;

	// Copying and moving is prohibited.
	OpenGLDummyWindow(const OpenGLDummyWindow&) = delete;
	OpenGLDummyWindow& operator =(const OpenGLDummyWindow&) = delete;
	OpenGLDummyWindow(OpenGLDummyWindow&&) = delete;
	OpenGLDummyWindow& operator =(OpenGLDummyWindow&&) = delete;

	// OpenGL window can be created and/or deleted only in fltk thread.
	class Deleter
	{
	public:
		void operator()(OpenGLDummyWindow* viewport) const;
	};
	
	// Helper smart pointer for managing dummy window.
	typedef ut::UniquePtr<OpenGLDummyWindow, Deleter> UniquePtr;

private:
	// Platform-specific data.
#if UT_WINDOWS
	HWND hwnd;
	HDC hdc;
#elif UT_LINUX
	Display* display;
#endif

	// OpenGL context that is associated with the current window.
	OpenGLContextHandle opengl_context;

	// This boolean variable is set to 'true' when @opengl_context is initialized.
	ut::Atomic<bool> initialized;
};

//----------------------------------------------------------------------------//
// Creates new opengl context for the current thread. Synchronizes with fltk
// to make different windows(viewports) accessible for this context.
OpenGLDummyWindow::UniquePtr CreateOpenGLDummyWindow();

//----------------------------------------------------------------------------//
// Helper class to lock fltk thread in current scope.
class FltkScopeLock
{
public:
	FltkScopeLock();
	~FltkScopeLock();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
