//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//

class Renderer : public System
{
public:
	Renderer(ut::SharedPtr<Device::Thread> render_thread) : System("render")
	                                                      , thread(ut::Move(render_thread))
	{
		UT_ASSERT(render_thread);
		it = 0;
	}

	// Draws all renderable components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update()
	{
		auto draw_proc = ut::MemberFunction<Renderer, void(Device&)>(this, &Renderer::DrawEnvironment);
		thread->Enqueue(draw_proc);
		return CmdArray();
	}

	void DrawEnvironment(Device& device)
	{
		ut::Optional<Display&> display_result = device.GetDisplay(0);
		if (!display_result)
		{
			return;
		}

		Display& display = display_result.Get();

		bool swap_col = it % 100 < 50;
		float color[4] = { swap_col ? 1.0f : 0.0f, swap_col ? 1.0f : 1.0f, 0.0f, 0.0f };
		//float color[4] = { 0.5f, 0.5f, 0.5f, 0.0f };
		device.context.ClearTarget(display.target, color);

		device.context.Present(display, true);

		it++;
	}

private:
	int it;
	ut::SharedPtr<Device::Thread> thread;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//