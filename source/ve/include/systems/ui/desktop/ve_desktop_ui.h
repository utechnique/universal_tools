//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui.h"
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "ve_id_generator.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::DesktopCfg is a class to save/load options and
// preferences for ui widgets.
class DesktopCfg : public ut::meta::Reflective
{
public:
	// Constructor, default values are set here.
	DesktopCfg();

	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	// Saves to file.
	ut::Optional<ut::Error> Save();

	// Loads from file.
	ut::Optional<ut::Error> Load();

	// Generates full local path to the configuration file.
	static ut::String GenerateFullPath();

	// data
	ut::uint32 position_x; // left coordinate of the window
	ut::uint32 position_y; // top coordinate of the window
	ut::uint32 width; // width of the window
	ut::uint32 height; // height of the window

	// default file path to the configuration file
	static const char* skFileName;
};

//----------------------------------------------------------------------------//
// ve::ui::DesktopFrontend class implements user interface for desktop systems.
class DesktopFrontend : public Frontend
{
public:
	// Constructor.
	DesktopFrontend();

	// Destructor
	~DesktopFrontend();

	// Initialization.
	ut::Optional<ut::Error> Initialize();

	// Launches user interface.
	void Run();

	// Saves current ui configuration to the file.
	void SaveCfg();

private:
	// synchronization primitives to detect when widgets
	// are initialized in fltk thread
	bool fltk_ready;
	ut::Mutex fltk_mutex;
	ut::ConditionVariable fltk_cvar;

	// fltk is single threaded, thus all widgets must be created and
	// processed in a separate thread
	ut::UniquePtr<ut::Thread> fltk_thread;

	// main window
	ut::UniquePtr<Fl_Window> window;

	// Id-generator is used to generate unique identifiers for viewports.
	IdGenerator<Viewport::Id> viewport_id_generator;

	// Minimum width of the window
	static const ut::uint32 skMinWidth;

	// Minimum height of the window
	static const ut::uint32 skMinHeight;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//