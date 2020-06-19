//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_engine.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Engine::Engine(Device& device, ViewportManager viewport_mgr) : ViewportManager(ut::Move(viewport_mgr))
{
	// load configuration file
	ut::Optional<ut::Error> load_cfg_error = config.Load();
	if (load_cfg_error)
	{
		const ut::error::Code error_code = load_cfg_error->GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "Render config file is absent. Using default configuration..." << ut::cret;
			config.Save();
		}
		else
		{
			ut::log << "Fatal error while loading render config file." << ut::cret;
			throw load_cfg_error.Move();
		}
	}

	// fill command buffer info
	CmdBufferInfo cmd_buffer_info;
	cmd_buffer_info.usage = CmdBufferInfo::usage_once;

	// create dynamic "per-frame" buffer
	ut::Result<CmdBuffer, ut::Error> cmd_buffer_result = device.CreateCmdBuffer(cmd_buffer_info);
	if (!cmd_buffer_result)
	{
		throw cmd_buffer_result.MoveAlt();
	}
	cmd_buffer = ut::MakeUnique<render::CmdBuffer>(cmd_buffer_result.MoveResult());

	// initialize color to clear display with
	backbuffer_clear_values.Add(ut::Color<4>(0, 0, 0, 0));
}

// Renders the whole environment to the internal images and presents
// the result to user.
void Engine::ProcessNextFrame(Device& device)
{
	// wait for the previous frame to finish and reset all dynamic 
	// command buffers so that they could be re-recorded once again
	device.ResetDynamicCmdPool();

	// synchronize all viewport events here
	SyncViewportEvents();

	// generate an array of active displays
	ut::Array< ut::Ref<Display> > display_array;
	for (size_t i = 0; i < viewports.GetNum(); i++)
	{
		display_array.Add(viewports[i].Get<Display>());
	}

	// record all commands for the current frame
	device.Record(cmd_buffer.GetRef(), [this](Context& context) { this->RecordFrameCommands(context); });

	// submit commands and enqueue display presentation
	device.Submit(cmd_buffer.GetRef(), display_array);
}

// Function for recording all commands needed to draw current frame.
void Engine::RecordFrameCommands(Context& context)
{
	for (size_t i = 0; i < viewports.GetNum(); i++)
	{
		Display& display = viewports[i].Get<Display>();
		RenderPass& rp = viewports[i].Get<RenderPass>();

		static int it = 0;
		it++;
		bool swap_col = it % 100 < 50;
		if (swap_col)
		{
			switch (i)
			{
			case 0: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 0, 1); break;
			case 1: backbuffer_clear_values[0] = ut::Color<4>(0, 1, 0, 1); break;
			case 2: backbuffer_clear_values[0] = ut::Color<4>(0, 0, 1, 1); break;
			case 3: backbuffer_clear_values[0] = ut::Color<4>(1, 1, 0, 1); break;
			}
		}
		else
		{
			switch (i)
			{
			case 0: backbuffer_clear_values[0] = ut::Color<4>(0, 0, 1, 1); break;
			case 1: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 1, 1); break;
			case 2: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 0, 1); break;
			case 3: backbuffer_clear_values[0] = ut::Color<4>(0, 1, 1, 1); break;
			}
		}

		ut::Array<Framebuffer>& framebuffers = viewports[i].Get< ut::Array<Framebuffer> >();

		Framebuffer& framebuffer = framebuffers[display.GetCurrentBufferId()];
		const FramebufferInfo& fb_info = framebuffer.GetInfo();
		ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);

		context.BeginRenderPass(rp, framebuffer, render_area, backbuffer_clear_values);

		context.EndRenderPass();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//