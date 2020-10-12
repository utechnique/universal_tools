//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_viewport_mgr.h"
#include "ve_render_toolset.h"
#include "ve_render_profiler.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Engine is a class that can render (visualize) entities. It must
// be used only in the thread it was created in.
class Engine : public ViewportManager
{
public:
	// Constructor.
	Engine(Device& render_device, ViewportManager viewport_mgr);

	// Renders the whole environment to the internal images and presents
	// the result to user.
	void ProcessNextFrame();

private:
	// Function for recording all commands needed to draw current frame.
	void RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportContainer> >& active_viewports);

	// Executes viewport tasks (resize, close, etc.) in a safe manner.
	void ProcessViewportEvents();

	// render device
	Device& device;

	// set of helper tools to operate with render resources
	Toolset tools;

	// measures performance
	Profiler profiler;

	// test
	ut::UniquePtr<Image> img_2d;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//