//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_default.h"
#include "systems/ui/ve_ui.h"
#include "systems/ui/desktop/ve_desktop_ui.h"
#include "systems/render/ve_render.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Generates default pipeline tree.
Pipeline GenDefaultPipeline()
{
	// create render thread
	ut::SharedPtr<render::Device::Thread> render_thread = ut::MakeShared<render::Device::Thread>();

	// create ui window
	ut::UniquePtr<ui::Frontend> ui_frontend = ut::MakeUnique<ui::DesktopFrontend>();
	ut::SharedPtr<ui::Frontend::Thread> ui_frontend_thread = ut::MakeShared<ui::Frontend::Thread>(ut::Move(ui_frontend));

	// build a pipeline
	Pipeline pipeline(ut::MakeShared<ui::Backend>(ui_frontend_thread));
	pipeline.AddSerial(Pipeline(ut::MakeShared<render::Renderer>(render_thread, ui_frontend_thread)));

	// success
	return pipeline;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//