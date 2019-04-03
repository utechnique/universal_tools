//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie.h"
#include "ui.h"
#include "meta_editor.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP // This file is only valid for desktop systems.
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// CfgEditor class implements interface for editing configuration. It is able
// to change configuration parameters, also reset and save configuration data.
class CfgEditor
{
public:
	// Constructor, reference to the parent ui object must be provided.
	//    @param parent_ui - reference to the parent (owning) UI object.
	CfgEditor(class DesktopUI& parent_ui);

	// Creates and opens a new window for the first call, or shows
	// hidden window otherwise. In both cases, entity tree will be rebuilt.
	//    @param cfg - reference to the configuration object to be edited
	//    @return - error if failed.
	ut::Optional<ut::Error> Open(Configuration& cfg);

	// Call this method to reset all values to the default ones
	// in the opened configuration window.
	//    @return - error if failed.
	ut::Optional<ut::Error> ResetValues();

	// Saves all widget data into intermediate text document,
	// then updates configuration object of the parent application with this data.
	//    @return - error if failed.
	ut::Optional<ut::Error> SaveCfg();

private:
	// reference to the parent (owning) UI object
	class DesktopUI& desktop_ui;

	// main window of the editor
	ut::UniquePtr<Fl_Double_Window> window;

	// container window for the MetaEditor data
	ut::UniquePtr<Fl_Double_Window> editor_container;

	// editor for the meta data,
	// allocated inside @editor_container
	MetaEditor meta_editor;

	// bottom container for the buttons
	ut::UniquePtr<Fl_Double_Window> button_container;

	// button to save cfg changes,
	// allocated inside @button_container
	ut::UniquePtr<Fl_Button> save_button;

	// button to save cfg values inside a tree,
	// allocated inside @button_container
	ut::UniquePtr<Fl_Button> reset_button;

	// width and height of the cfg editor window
	static const int skCfgEditorSizeX;
	static const int skCfgEditorSizeY;

	// metrics for the bottom button panel
	static const int skBottomOffset;
	static const int skButtonOffset;
	static const int skButtonWidth;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//