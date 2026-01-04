//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_viewport_mgr.h"
#include "ve_render_toolset.h"
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

	// Includes provided units to the rendering scene.
	void RegisterEntity(Entity::Id id,
	                    ut::Array< ut::UniquePtr<Unit> >& units);

	// Excludes units of the specified entity from the rendering scene.
	void UnregisterEntity(Entity::Id id);

	// Initializes a unit with the correct policy.
	void InitializeUnit(Unit& unit);

	// Renders the whole environment to the internal images and presents
	// the result to user.
	//    @param time_step_ms - time step in milliseconds.
	void ProcessNextFrame(System::Time time_step_ms);
	
	// Returns a reference to the rendering thread pool, 
	// use it to parallelize cpu work.
	ut::ThreadPool<void>& GetThreadPool();

	// Returns a reference to the profiler object,
	// use it to measure performance.
	Profiler& GetProfiler();

private:
	// Updates the information about the current frame and returns the
	// reference to this frmae.
	Frame& UpdateCurrentFrameInfo();

	// Function for recording all commands needed to draw current frame.
	void RecordFrameCommands(Context& context,
	                         ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports,
	                         System::Time time_step_ms);

	// Renders view units to ui viewports.
	void DisplayToUser(Context& context, ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports);

	// Displays provided image to the provided ui viewport.
	void DisplayImage(Context& context,
	                  Image& image,
	                  ViewportManager::Proxy& viewport,
	                  bool display_profiler);

	// Executes viewport tasks (resize, close, etc.) in a safe manner.
	void ProcessViewportEvents();

	// Creates a new display and all associated render resources for the
	// provided viewport.
	//    @param viewport - reference to the viewport.
	//    @return - container with all render resources, or error if failed.
	ut::Result<Proxy, ut::Error> CreateDisplay(ui::PlatformViewport& viewport) override;

	// render device
	Device& device;

	// set of helper tools to operate with render resources
	Toolset tools;

	// unit manager renders and maintains units
	UnitManager unit_mgr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
