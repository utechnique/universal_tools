//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui.h"
#include "ve_default.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Minimum width of the window
const ut::uint32 DesktopFrontend::skMinWidth = 320;

// Minimum height of the window
const ut::uint32 DesktopFrontend::skMinHeight = 320;

// Default local file path to the configuration file.
const char* DesktopCfg::skFileName = "ui.json";

//----------------------------------------------------------------------------//
// Constructor, default values are set here.
DesktopCfg::DesktopCfg()
{
	position_x = 0;
	position_y = 0;
	width = 640;
	height = 480;
}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void DesktopCfg::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(position_x, "position_x");
	snapshot.Add(position_y, "position_y");
	snapshot.Add(width, "width");
	snapshot.Add(height, "height");
}

// Saves to file.
ut::Optional<ut::Error> DesktopCfg::Save()
{
	// create cfg directory if it doesn't exist
	ut::CreateFolder(directories::skCfg);

	// open file for writing
	ut::File cfg_file;
	ut::Optional<ut::Error> open_error = cfg_file.Open(GenerateFullPath(),
	                                                   ut::file_access_write);
	if (open_error)
	{
		return open_error;
	}
	else
	{
		ut::JsonDoc json_doc;
		ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(*this);
		cfg_file << (json_doc << cfg_snapshot);
		cfg_file.Close();
		ut::log << "UI Cfg file was updated." << ut::cret;
	}

	// success
	return ut::Optional<ut::Error>();
}

// Loads from file.
ut::Optional<ut::Error> DesktopCfg::Load()
{
	// open file
	ut::File cfg_file;
	ut::Optional<ut::Error> open_cfg_error = cfg_file.Open(GenerateFullPath(),
	                                                       ut::file_access_read);
	if (open_cfg_error)
	{
		return ut::Error(ut::error::no_such_file);
	}

	// deserialize cfg from the text document
	ut::JsonDoc json_doc;
	ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(*this);
	cfg_file >> json_doc >> cfg_snapshot;
	cfg_file.Close();
	ut::log << "Loaded UI config file " << skFileName << "." << ut::cret;

	// success
	return ut::Optional<ut::Error>();
}

// Generates full local path to the configuration file.
ut::String DesktopCfg::GenerateFullPath()
{
	return ut::String(directories::skCfg) + ut::fsep + skFileName;
}

//----------------------------------------------------------------------------//
// Constructor.
DesktopFrontend::DesktopFrontend(ut::SharedPtr<render::Device::Thread> in_render_thread) : render_thread(ut::Move(in_render_thread))
{
	fltk_thread = ut::MakeUnique<ut::Thread>([this] { this->Run(); });
}

//----------------------------------------------------------------------------->
// Destructor
DesktopFrontend::~DesktopFrontend()
{
	// close all viewports
	const size_t display_count = viewports.GetNum();
	for (size_t i = 0; i < display_count; i++)
	{
		render_thread->Enqueue([&](render::Device& device) { CloseRenderDisplay(device, viewports[i].first); });
	}
}

//----------------------------------------------------------------------------->
// Initialization.
ut::Optional<ut::Error> DesktopFrontend::Initialize()
{
	// get configuration copy
	DesktopCfg cfg;
	ut::Optional<ut::Error> load_error = cfg.Load();
	if (load_error)
	{
		const ut::error::Code error_code = load_error.Get().GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "UI config file is absent. Using default configuration..." << ut::cret;
		}
		else
		{
			ut::log << "Fatal error while loading UI config file." << ut::cret;
			return load_error.Move();
		}
	}

	// set scheme
	Fl::scheme("plastic");

	// create main window
	window = ut::MakeUnique<Fl_Double_Window>(cfg.position_x,
	                                          cfg.position_y,
	                                          cfg.width,
	                                          cfg.height,
	                                          skTitle);
	window->size_range(skMinWidth, skMinHeight);

	// create render area
	Viewport::Id viewport_id = viewport_id_generator.Generate();
	ut::UniquePtr<Viewport> viewport;
	viewport = ut::MakeUnique<Viewport>(viewport_id, "1", 0, 0, window->w(), window->h());
	auto resize_callback = ut::MemberFunction<DesktopFrontend, void(Viewport::Id, ut::uint32, ut::uint32)>(this, &DesktopFrontend::ResizeRenderDisplay);
	viewport->ConnectResizeSignalSlot(ut::Move(resize_callback));
	viewport->show();
	window->resizable(viewport.Get());

	// finish main window
	window->end();

	// show main window
	window->show(0, nullptr);

	// add viewport to the map
	if (!viewports.Insert(viewport_id, ut::Move(viewport)))
	{
		return ut::Error(ut::error::out_of_memory);
	}

	// register viewport in render thread
	render_thread->Enqueue([this, viewport_id](render::Device& device) { this->RegisterRenderDisplay(device, viewport_id); });

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Launches user interface.
void DesktopFrontend::Run()
{
	// initialize all widgets
	ut::Optional<ut::Error> init_error = Initialize();
	if (init_error)
	{
		ut::log << init_error.Get().GetDesc() + ut::cret;
		throw ut::Error(init_error.Move());
	}

	// lock fltk so that we could pass commands to the main thread
	// using Fl::awake(cmd) function
	Fl::lock();

	// run user interface routine
	Fl::run();

	// unlock fltk to gracefully exit
	Fl::unlock();

	// save config file
	SaveCfg();

	// exit signal
	exit_signal();
}

//----------------------------------------------------------------------------->
// Saves current ui configuration to the file.
void DesktopFrontend::SaveCfg()
{
	DesktopCfg cfg;

	// main window parameters
	cfg.position_x = window->x();
	cfg.position_y = window->y();
	cfg.width = window->w();
	cfg.height = window->h();

	// save to file
	cfg.Save();
}

//----------------------------------------------------------------------------->
// Creates ve::render::Display for the viewport with specified id
// in render thread.
//    @param device - reference to render device.
//    @param id - identifier of the viewport to register.
void DesktopFrontend::RegisterRenderDisplay(render::Device& device, Viewport::Id id)
{
	ut::Optional<ut::UniquePtr<Viewport>&> viewport = viewports.Find(id);
	if (!viewport)
	{
		throw ut::Error(ut::error::not_found);
	}

	ut::UniquePtr<Viewport>& viewport_ptr = viewport.Get();
	ut::Optional<ut::Error> add_error = device.AddViewport(viewport_ptr.GetRef());
	if (add_error)
	{
		throw ut::Error(add_error.Move());
	}
}

//----------------------------------------------------------------------------->
// Closes ve::render::Display object associated with specified
// viewport in render thread.
//    @param device - reference to render device.
//    @param id - identifier of the viewport to close.
void DesktopFrontend::CloseRenderDisplay(render::Device& device, Viewport::Id id)
{
	ut::Optional<ut::UniquePtr<Viewport>&> viewport = viewports.Find(id);
	if (!viewport)
	{
		throw ut::Error(ut::error::not_found);
	}

	device.RemoveViewport(id);
}

//----------------------------------------------------------------------------->
// Sends a request to the render thread to resize desired display.
//    @param id - id of the viewport to resize.
//    @param width - new width in pixels.
//    @param height - new height in pixels.
void DesktopFrontend::ResizeRenderDisplay(Viewport::Id id, ut::uint32 width, ut::uint32 height)
{
	render_thread->Enqueue([&](render::Device& device) {
		ut::Optional<ut::Error> resize_error = device.ResizeViewport(id, width, height);
		if (resize_error)
		{
			throw ut::Error(resize_error.Move());
		}
	});
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//