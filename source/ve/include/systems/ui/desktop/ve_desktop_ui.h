//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::UIConfiguration is a class to save/load options and
// preferences for ui widgets.
class UIConfiguration : public ut::meta::Reflective
{
public:
	// Constructor, default values are set here.
	UIConfiguration();

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
// DesktopUI class implements user interface for desktop systems.
class DesktopUI : public UIDevice
{
public:
	// Launches user interface.
	void Run();

	// Saves current ui configuration to the file.
	void SaveCfg();

private:
	// main window
	ut::UniquePtr<Fl_Double_Window> window;

	// Minimum width of the window
	static const ut::uint32 skMinWidth;

	// Minimum height of the window
	static const ut::uint32 skMinHeight;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//