//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_context.h"
#include "systems/render/api/ve_render_target.h"
#include "systems/render/api/ve_render_display.h"
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/ui/ve_ui.h"
#include "ve_dedicated_thread.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Device class creates rendering resources.
class Device : private PlatformDevice
{
public:
	// Type of a special thread for rendering.
	typedef DedicatedThread<Device> Thread;

	// Constructor.
	Device(ut::SharedPtr<ui::Frontend::Thread> ui_frontend);

	// Move constructor.
	Device(Device&&) noexcept;

	// Move operator.
	Device& operator =(Device&&) noexcept;

	// Copying is prohibited.
	Device(const Device&) = delete;
	Device& operator =(const Device&) = delete;

	// Creates new texture.
	//    @param format - texture format, see ve::render::pixel::Format.
	//    @param width - width of the texture in pixels.
	//    @param height - height of the texture in pixels.
	//    @return - new texture object of error if failed.
	ut::Result<Texture, ut::Error> CreateTexture(pixel::Format format,
	                                             ut::uint32 width,
	                                             ut::uint32 height);

	// Creates platform-specific deferred context, which can record and execute
	// rendering commands. 'Deferred' here means that context can record commands
	// from the other thread than main context does. Use it on platforms
	// supporting multithreaded
	//    @return - new context or error if failed.
	ut::Result<Context, ut::Error> CreateDeferredContext();

	// Creates platform-specific representation of the rendering area inside a UI viewport.
	//    @param viewport - reference to UI viewport containing rendering area.
	//    @return - new display object or error if failed.
	ut::Result<Display, ut::Error> CreateDisplay(ui::DesktopViewport& viewport);

	// Resizes buffers associated with rendering area inside a UI viewport.
	//    @param display - reference to display object.
	//    @param width - new width of the display in pixels.
	//    @param width - new height of the display in pixels.
	//    @return - optional ut::Error if failed.
	ut::Optional<ut::Error> ResizeDisplay(Display& display,
	                                      ut::uint32 width,
	                                      ut::uint32 height);

	// Main context.
	Context context;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
