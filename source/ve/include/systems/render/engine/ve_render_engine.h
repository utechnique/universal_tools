//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_viewport_mgr.h"
#include "ve_render_toolset.h"
#include "ve_render_profiler.h"
#include "policy/ve_render_view_policy.h"
#include "ve_render_unit_mgr.h"

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

	// Resets all previously added unit links.
	void UnlinkUnits();

	// Adds references to the provided units, these units will participate
	// in the rendering process.
	void LinkUnits(ut::Array< ut::UniquePtr<Unit> >& units);

	// Renders the whole environment to the internal images and presents
	// the result to user.
	void ProcessNextFrame();

private:
	// Function for recording all commands needed to draw current frame.
	void RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports);

	// Renders view units to ui viewports.
	void DisplayToUser(Context& context, ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports);

	// Displays provided image to the provided ui viewport.
	void DisplayImage(Context& context,
	                  Image& image,
	                  ViewportManager::Proxy& viewport,
	                  bool display_profiler);

	// Executes viewport tasks (resize, close, etc.) in a safe manner.
	void ProcessViewportEvents();

	// render device
	Device& device;

	// set of helper tools to operate with render resources
	Toolset tools;

	// unit manager renders and maintains units
	UnitManager unit_mgr;

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
