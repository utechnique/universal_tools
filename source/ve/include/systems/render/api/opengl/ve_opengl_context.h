//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// OpenGL context.
class PlatformContext
{
	friend class Device;
public:
	// Constructor.
	PlatformContext(OpenGLDummyWindow::UniquePtr window);

	// Destructor.
	~PlatformContext();

	// Move constructor.
	PlatformContext(PlatformContext&&) noexcept;

	// Move operator.
	PlatformContext& operator =(PlatformContext&&) noexcept;

	// Copying is prohibited.
	PlatformContext(const PlatformContext&) = delete;
	PlatformContext& operator =(const PlatformContext&) = delete;

protected:
	GLuint framebuffer;

private:
	// Destroys managed objects.
	void Destroy();

	OpenGLDummyWindow::UniquePtr dummy_window;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//