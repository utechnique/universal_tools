//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "ve_render_api.h"
#include "systems/ui/ve_ui.h"
#include "engine/ve_render_engine.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::RenderSystem is a system responsible for drawing visible
// components and displaying the result to user.
class RenderSystem : public System
{
public:
	// Constructor.
	RenderSystem(ut::SharedPtr<Device::Thread> in_render_thread,
	             ut::SharedPtr<ui::Frontend::Thread> in_ui_thread);

	// Destructor. Engine is destructed in the render thread.
	~RenderSystem();

	// Draws all renderable components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update();

private:
	ut::SharedPtr<ui::Frontend::Thread> ui_thread;
	ut::SharedPtr<Device::Thread> render_thread;
	ut::UniquePtr<Engine> engine;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
