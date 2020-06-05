//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
RenderSystem::RenderSystem(ut::SharedPtr<Device::Thread> in_render_thread,
                           ut::SharedPtr<ui::Frontend::Thread> in_ui_thread) : System("render")
                                                                             , render_thread(ut::Move(in_render_thread))
                                                                             , ui_thread(ut::Move(in_ui_thread))
{
	UT_ASSERT(ui_thread);
	UT_ASSERT(render_thread);

	// create viewport manager
	ViewportManager vp_mgr(in_render_thread);

	// initialize render engine
	render_thread->Enqueue([&](Device& device) { engine = ut::MakeUnique<Engine>(device, ut::Move(vp_mgr)); });

	// ask render thread to create display, but the task must be scheduled from the ui thread
	// to synchronize them both
	ui_thread->Enqueue([&](ui::Frontend& frontend) { engine->OpenViewports(frontend); });
}

// Destructor. Engine is destructed in the render thread.
RenderSystem::~RenderSystem()
{
	render_thread->Enqueue([&](Device& device) { ut::this_thread::Sleep(2000); device.WaitIdle(); engine.Delete(); });
}

// Draws all renderable components.
//    @return - array of commands to be executed by owning environment,
//              or ut::Error if system encountered fatal error.
System::Result RenderSystem::Update()
{
	// draw scene in render thread
	render_thread->Enqueue([&](Device& device) { engine->ProcessNextFrame(device); });

	return CmdArray();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//