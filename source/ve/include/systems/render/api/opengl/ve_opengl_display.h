//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
#include <FL/Fl_Gl_Window.H>
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
#include "systems/ui/desktop/ve_desktop_viewport.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// OpenGL display.
class PlatformDisplay
{
	friend class Context;
	friend class Device;
public:
	// Constructor.
	PlatformDisplay(Fl_Gl_Window* opengl_window);

	// Move constructor.
	PlatformDisplay(PlatformDisplay&& other) noexcept;

	// Move operator.
	PlatformDisplay& operator =(PlatformDisplay&&) noexcept;

	// Copying is prohibited.
	PlatformDisplay(const PlatformDisplay&) = delete;
	PlatformDisplay& operator =(const PlatformDisplay&) = delete;

protected:
	// Platform-specific window containing this display.
	Fl_Gl_Window* window;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//