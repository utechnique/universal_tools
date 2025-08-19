//----------------------------------------------------------------------------//
//-----------------------------|  C R A D L E  |----------------------------- //
//----------------------------------------------------------------------------//
#include "cradle_pipeline.h"
#include "ve_config.h"
#include "systems/ui/ve_ui.h"
#include "systems/input/ve_input_poll_system.h"
#include "systems/render/ve_render_system.h"
#include "systems/render/ve_render_camera_system.h"
#include "systems/editor/ve_editor_camera_system.h"
#include "systems/editor/ve_editor_selection_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(cradle)
//----------------------------------------------------------------------------//

ve::Pipeline CreatePipeline()
{
	// load default configuration
	ve::Config<ve::DefaultSettings> config;
	const ut::Optional<ut::Error> load_cfg_error = config.Load();
	if (load_cfg_error)
	{
		config.Save();
	}

	// create ui
	ut::UniquePtr<ve::ui::Frontend> ui_frontend = ut::MakeUnique<ve::ui::PlatformFrontend>();
#if VE_DESKTOP
	ve::ui::DesktopFrontend& desktop_frontend = static_cast<ve::ui::DesktopFrontend&>(ui_frontend.GetRef());
#endif
	ut::SharedPtr<ve::ui::Frontend::Thread> ui_frontend_thread = ut::MakeShared<ve::ui::Frontend::Thread>(ut::Move(ui_frontend));

	// create input manager shared among systems
	ut::SharedPtr<ve::input::Manager> input_mgr = ut::MakeShared<ve::input::Manager>();

	// create render system
	ut::SharedPtr<ve::render::Device::Thread> render_thread = ut::MakeShared<ve::render::Device::Thread>(config.gpu);
	ut::SharedPtr<ve::render::RenderSystem> render_sys = ut::MakeShared<ve::render::RenderSystem>(render_thread);
	ut::SharedPtr<ve::render::CameraSystem> render_camera_sys = ut::MakeShared<ve::render::CameraSystem>();

	// create input system
	ut::SharedPtr<ve::input::PollSystem> input_poll_sys = ut::MakeShared<ve::input::PollSystem>(input_mgr);

	// start main window
#if VE_DESKTOP
	desktop_frontend.Start();
#endif

	// make ui viewports accessible for rendering
	render_sys->BindViewports(ui_frontend_thread);

	// create all systems depending on ui frontend
	ut::SharedPtr<ve::ui::Backend> ui_backend_sys = ut::MakeShared<ve::ui::Backend>(ui_frontend_thread);
	ut::SharedPtr<ve::editor::ViewportCameraSystem> editor_camera_sys =
		ut::MakeShared<ve::editor::ViewportCameraSystem>(ui_frontend_thread, input_mgr);
	ut::SharedPtr<ve::editor::ViewportSelectionSystem> editor_selection_sys =
		ut::MakeShared<ve::editor::ViewportSelectionSystem>(ui_frontend_thread, input_mgr);

	// build a pipeline
	ve::Pipeline pipeline;
	pipeline.AddSerial(ve::Pipeline(ut::Move(input_poll_sys)));
	pipeline.AddSerial(ve::Pipeline(ut::Move(ui_backend_sys)));
	pipeline.AddSerial(ve::Pipeline(ut::Move(editor_camera_sys)));
	pipeline.AddSerial(ve::Pipeline(ut::Move(editor_selection_sys)));
	pipeline.AddSerial(ve::Pipeline(ut::Move(render_camera_sys)));
	pipeline.AddSerial(ve::Pipeline(ut::Move(render_sys)));

	// success
	return pipeline;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(cradle)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//