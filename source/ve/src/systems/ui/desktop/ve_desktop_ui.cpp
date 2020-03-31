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
DesktopFrontend::DesktopFrontend() : fltk_ready(false)
{
	fltk_thread = ut::MakeUnique<ut::Thread>([this] { this->Run(); });
	ut::ScopeLock lock(fltk_mutex);
	while (!fltk_ready)
	{
		fltk_cvar.Wait(lock);
	}
}

//----------------------------------------------------------------------------->
// Destructor
DesktopFrontend::~DesktopFrontend()
{}

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
	ut::UniquePtr<DesktopViewport> viewport;
	viewport = ut::MakeUnique<DesktopViewport>(viewport_id, "1", 0, 0, window->w(), window->h());
	viewport->show();

	// adjust main window
	window->resizable(viewport.Get());

	// add viewport to the map
	if (!viewports.Add(ut::Move(viewport)))
	{
		return ut::Error(ut::error::out_of_memory);
	}

	// finish main window
	window->end();

	// show main window
	window->show(0, nullptr);

	{ // inform main thread that everything is ready
		ut::ScopeLock lock(fltk_mutex);
		fltk_ready = true;
	}
	fltk_cvar.WakeOne();

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

	// viewports are created in the fltk thread,
	// so they must be deleted there too
	viewports.Empty();

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

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
