//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_cfg.h"
#include "ve_render_viewport_mgr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Engine is a class that can render (visualize) entities.
class Engine : public ViewportManager
{
public:
	// Constructor.
	Engine(Device& device, ViewportManager viewport_mgr);

	// Renders the whole environment to the internal images and presents
	// the result to user.
	void ProcessNextFrame(Device& device);

	// Function for recording all commands needed to draw current frame.
	void RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportContainer> >& active_viewports);

private:
	ut::UniquePtr<render::CmdBuffer> cmd_buffer;
	ut::Array< ut::Color<4> > backbuffer_clear_values;
	Config<Settings> config;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
