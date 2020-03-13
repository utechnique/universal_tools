//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui.h"
#include "ve_default.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Minimum width of the window
const ut::uint32 DesktopUI::skMinWidth = 320;

// Minimum height of the window
const ut::uint32 DesktopUI::skMinHeight = 320;

// Default local file path to the configuration file.
const char* UIConfiguration::skFileName = "ui.json";

//----------------------------------------------------------------------------//
// Constructor, default values are set here.
UIConfiguration::UIConfiguration()
{
	position_x = 0;
	position_y = 0;
	width = 640;
	height = 480;
}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void UIConfiguration::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(position_x, "position_x");
	snapshot.Add(position_y, "position_y");
	snapshot.Add(width, "width");
	snapshot.Add(height, "height");
}

// Saves to file.
ut::Optional<ut::Error> UIConfiguration::Save()
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
ut::Optional<ut::Error> UIConfiguration::Load()
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
ut::String UIConfiguration::GenerateFullPath()
{
	return ut::String(directories::skCfg) + ut::fsep + skFileName;
}

//----------------------------------------------------------------------------//
// Launches user interface.
void DesktopUI::Run()
{
	// get configuration copy
	UIConfiguration cfg;
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
			throw load_error.Move();
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

	// finish main window
	window->end();

	// show main window
	window->show(0, nullptr);

	// run user interface routine
	Fl::run();

	// save config file
	SaveCfg();

	// exit signal
	exit_signal();
}

//----------------------------------------------------------------------------->
// Saves current ui configuration to the file.
void DesktopUI::SaveCfg()
{
	UIConfiguration cfg;

	// main window parameters
	cfg.position_x = window->x();
	cfg.position_y = window->y();
	cfg.width = window->w();
	cfg.height = window->h();

	// save to file
	cfg.Save();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//