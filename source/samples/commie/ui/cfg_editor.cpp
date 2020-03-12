//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "desktop_ui.h"
#include "../rc.h"
#include <FL/fl_ask.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
// width and height of the cfg editor window
const int CfgEditor::skCfgEditorSizeX = 480;
const int CfgEditor::skCfgEditorSizeY = 480;

// metrics for the bottom button panel
const int CfgEditor::skBottomOffset = 48;
const int CfgEditor::skButtonOffset = 4;
const int CfgEditor::skButtonWidth = 64;

//----------------------------------------------------------------------------//
// This callback saves configuration after pressing appropriate button.
//    @param widget - pointer to the button widget.
//    @param data - expected to be a pointer to the CfgEditor object.
void CfgEditorSaveCB(Fl_Widget* w, void* data)
{
	UT_ASSERT(data != nullptr);
	CfgEditor* editor = static_cast<CfgEditor*>(data);
	ut::Optional<ut::Error> save_cfg_error = editor->SaveCfg();
	if (save_cfg_error)
	{
		ut::log << "Failed to save configuration file:" << ut::CRet();
		ut::log << save_cfg_error.Get().GetDesc() << ut::CRet();
	}

	fl_alert("You must restart the application to apply these changes.");
}

// This callback resets all widget values to defaul ones
// after pressing appropriate button inside editor window.
//    @param widget - pointer to the button widget.
//    @param data - expected to be a pointer to the CfgEditor object.
void CfgEditorResetCB(Fl_Widget* w, void* data)
{
	UT_ASSERT(data != nullptr);
	CfgEditor* editor = static_cast<CfgEditor*>(data);
	ut::Optional<ut::Error> reset_error = editor->ResetValues();
	if (reset_error)
	{
		ut::log << "Failed to reset configuration file:" << ut::CRet();
		ut::log << reset_error.Get().GetDesc() << ut::CRet();
	}
}

//----------------------------------------------------------------------------//
// Constructor, reference to the parent ui object must be provided.
CfgEditor::CfgEditor(class DesktopUI& parent_ui) : desktop_ui(parent_ui)
{ }

//----------------------------------------------------------------------------->
// Creates and opens a new window for the first call, or shows
// hidden window otherwise. In both cases, entity tree will be rebuilt.
//    @param cfg - reference to the configuration object to be edited
//    @return - error if failed.
ut::Optional<ut::Error> CfgEditor::Open(Configuration& cfg)
{
	// serialize configuration to the text document
	ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(cfg);
	ut::JsonDoc json_doc;
	json_doc << cfg_snapshot;

	// open cfg window
	if (window != nullptr)
	{
		// if window has been already created - just rebuild a tree
		ut::Optional<ut::Error> rebuild_error = meta_editor.Rebuild(json_doc);
		if (rebuild_error)
		{
			return rebuild_error;
		}

		// and show it (if it was hidden)
		window->show(0, nullptr);
	}
	else
	{
		// create main window
		window = ut::MakeUnique<Fl_Double_Window>(0, 0, skCfgEditorSizeX, skCfgEditorSizeY, "Configuration");
		window->size_range(skCfgEditorSizeX, 120, skCfgEditorSizeX);

		// create editor container window
		editor_container = ut::MakeUnique<Fl_Double_Window>(0, 0, window->w(), window->h() - skBottomOffset, "Cfg Editor Container");
		editor_container->resizable(editor_container.Get());

		// create text node editor
		ut::Optional<ut::Error> meta_editor_error = meta_editor.Create(editor_container.Get(), json_doc);
		if (meta_editor_error)
		{
			return meta_editor_error;
		}

		// finish editor container window
		editor_container->end();

		// create bottom button panel
		button_container = ut::MakeUnique<Fl_Double_Window>(0, window->h() - skBottomOffset, window->w(), skBottomOffset, "Cfg Button Container");
		button_container->size_range(320, skBottomOffset, 0, skBottomOffset);
		button_container->resizable(window.Get());

		// create reset button
		reset_button = ut::MakeUnique<Fl_Button>(skButtonOffset,
		                                         skButtonOffset,
		                                         skButtonWidth,
		                                         button_container->h() - skButtonOffset * 2, "Reset");
		reset_button->callback(CfgEditorResetCB, this);
		reset_button->when(FL_WHEN_RELEASE);
		reset_button->visible_focus(0);

		// create save button
		save_button = ut::MakeUnique<Fl_Button>(button_container->w() - skButtonWidth - skButtonOffset,
		                                        skButtonOffset,
		                                        skButtonWidth,
		                                        button_container->h() - skButtonOffset * 2, "Save");
		save_button->callback(CfgEditorSaveCB, this);
		save_button->when(FL_WHEN_RELEASE);
		save_button->visible_focus(0);

		// finish bottom button panel
		button_container->end();

		// set editor container as a resizable widget
		window->resizable(editor_container.Get());

		// finish main window
		window->end();
		window->focus(window.Get());
		window->show(0, nullptr);
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Call this method to reset all values to the default ones
// in the opened configuration window.
//    @return - error if failed.
ut::Optional<ut::Error> CfgEditor::ResetValues()
{
	// serialize default cfg object
	Configuration cfg;
	ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(cfg);
	ut::XmlDoc document;
	document << cfg_snapshot;

	// rebuild tree
	ut::Optional<ut::Error> rebuild_error = meta_editor.Rebuild(document);
	if (rebuild_error)
	{
		return rebuild_error;
	}

	// redraw container window
	editor_container->redraw();

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Saves all widget data into intermediate text document,
// then updates configuration object of the parent application with this data.
//    @return - error if failed.
ut::Optional<ut::Error> CfgEditor::SaveCfg()
{
	// set focus back to the editor and close
	window->focus(editor_container.Get());
	window->hide();

	// create document
	ut::Result<ut::XmlDoc, ut::Error> doc_result = meta_editor.Save();
	if (!doc_result)
	{
		return ut::Error(doc_result.MoveAlt());
	}

	// deserialize configutation from the document
	ut::XmlDoc doc(doc_result.MoveResult());
	return desktop_ui.SaveCfg(doc);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//