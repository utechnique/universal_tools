//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_default.h"
#include "systems/ui/ve_ui.h"
#include "systems/input/ve_input_poll_system.h"
#include "systems/render/ve_render_system.h"
#include "systems/render/ve_render_camera_system.h"
#include "systems/editor/ve_editor_camera_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Generates default pipeline tree.
Pipeline GenDefaultPipeline()
{
	// create ui window
	ut::UniquePtr<ui::Frontend> ui_frontend = ut::MakeUnique<ui::PlatformFrontend>();
	ut::SharedPtr<ui::Frontend::Thread> ui_frontend_thread = ut::MakeShared<ui::Frontend::Thread>(ut::Move(ui_frontend));

    // create render thread
	ut::SharedPtr<render::Device::Thread> render_thread = ut::MakeShared<render::Device::Thread>(ui_frontend_thread);

	// create input state shared among systems
	ut::SharedPtr<input::Manager> input_mgr = ut::MakeShared<input::Manager>();

	// build a pipeline
	Pipeline pipeline;
	pipeline.AddSerial(Pipeline(ut::MakeShared<input::PollSystem>(input_mgr)));
	pipeline.AddSerial(Pipeline(ut::MakeShared<ui::Backend>(ui_frontend_thread)));
	pipeline.AddSerial(Pipeline(ut::MakeShared<editor::ViewportCameraSystem>(ui_frontend_thread, input_mgr)));
	pipeline.AddSerial(Pipeline(ut::MakeShared<render::CameraSystem>()));
	pipeline.AddSerial(Pipeline(ut::MakeShared<render::RenderSystem>(render_thread, ui_frontend_thread)));

	// success
	return pipeline;
}

// Searches for a file using provided filename, if desired file wasn't found,
// searches for it in ve::skRc and ve::skRcAlt directories, and if it wasn't
// found even there - returns an error.
//    @param filename - path to the file.
//    @return - path to the file or ut::Error if failed.
ut::Optional<ut::String> FindResourceFile(const ut::String& filename)
{
	ut::File file;
	ut::String path = filename;
	ut::Optional<ut::Error> open_file_error = file.Open(path, ut::file_access_read);
	if (open_file_error)
	{
		// if didn't work - try default resources directory
		path = ut::String(directories::skRc) + ut::skFileSeparator + filename;
		open_file_error = file.Open(path, ut::file_access_read);
		if (open_file_error)
		{
			// finally try alternative resources directory
			path = ut::String(directories::skRcAlt) + ut::skFileSeparator + filename;
			open_file_error = file.Open(path, ut::file_access_read);
			if (open_file_error)
			{
				return ut::Optional<ut::String>();
			}
		}
	}
	return path;
}

// Opens a file using ve::FindResourceFile() function.
//    @param filename - path to the file.
//    @return - opened file or ut::Error if failed.
ut::Result<ut::File, ut::Error> OpenResourceFile(const ut::String& filename)
{
	ut::Optional<ut::String> path = FindResourceFile(filename);
	if (!path)
	{
		return ut::MakeError(ut::error::not_found);
	}

	ut::File file;
	ut::Optional<ut::Error> open_file_error = file.Open(path.Get(), ut::file_access_read);
	if (open_file_error)
	{
		return ut::MakeError(open_file_error.Move());
	}
	return file;
}

// Reads data from file using ve::FindResourceFile() function.
//    @param filename - path to the file.
//    @return - contents of the desired file or ut::Error if failed.
ut::Result<ut::Array<ut::byte>, ut::Error> ReadResourceFile(const ut::String& filename)
{
	ut::Optional<ut::String> path = FindResourceFile(filename);
	if (!path)
	{
		return ut::MakeError(ut::error::not_found);
	}
	return ut::ReadFile(path.Get());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
