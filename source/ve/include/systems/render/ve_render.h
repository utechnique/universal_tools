//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "ve_render_api.h"
#include "ve_render_viewport_mgr.h"
#include "systems/ui/ve_ui.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//

class Renderer : public System, public ViewportManager
{
public:
	// Constructor.
	Renderer(ut::SharedPtr<Device::Thread> in_render_thread,
	         ut::SharedPtr<ui::Frontend::Thread> in_ui_thread) : System("render")
	                                                           , ViewportManager(in_render_thread)
	                                                           , render_thread(ut::Move(in_render_thread))
		                                                       , ui_thread(ut::Move(in_ui_thread))
	{
		UT_ASSERT(ui_thread);
		UT_ASSERT(render_thread);

		// ask render thread to create display, but the task must be scheduled from the ui thread
		// to synchronize them both
		ui_thread->Enqueue([this](ui::Frontend& frontend) {this->OpenViewports(frontend); });
	}

	// Draws all renderable components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update()
	{
        // execute viewport-related task in this thread
        ExecuteViewportTasks();

		// render scene
		render_thread->Enqueue([this](render::Device& device) { this->DrawEnvironment(device); });
		return CmdArray();
	}

private:
	void DrawEnvironment(Device& device)
	{
        ut::ScopeLock viewport_lock(viewport_guard);

		if (viewports.GetNum() == 0)
		{
			return;
		}

		Display& display = viewports.GetFirst().Get<Display>();
		ui::DesktopViewport& viewport = viewports.GetFirst().Get<ui::DesktopViewport&>();

		static int it = 0;
		it++;
		bool swap_col = it % 10000 < 5000;
		float color[4] = { swap_col ? 1.0f : 0.0f, 1.0f, 0.0f, 0.0f };
		device.context.ClearTarget(display.target, color);

		device.context.Present(display, false);
	}

	ut::SharedPtr<ui::Frontend::Thread> ui_thread;
	ut::SharedPtr<Device::Thread> render_thread;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
