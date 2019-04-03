//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie.h"
#include "../cfg.h"
#include "ui.h"
#include "ui_cmd.h"
#include "cfg_editor.h"
#include "server_ui.h"
#include "client_ui.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
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
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// DesktopUI class implements user interface for desktop systems.
class DesktopUI : public UI, public ut::NonCopyable
{
public:
	// Constructor, parent application must be provided.
	// All widgets are initialized here.
	//    @param application - reference to parent (owning) application object
	DesktopUI(Application& application);

	// Launches user interface
	void Run();

	// Opens configuration editor.
	//    @return - error if failed to open the editor
	ut::Optional<ut::Error> OpenCfgEditor();

	// Synchronizes internal UI data, options and preferences with
	// main application. Just save your data into provided cfg reference.
	//    @param cfg - reference to the configuration object
	//                 of the main application
	void SyncCfg(Configuration& cfg);

	// Forces main application to update it's configuration object
	// by deserializing the provided text document.
	//    @param doc - reference to the source text document.
	//    @return - error if failed to save configuration.
	ut::Optional<ut::Error> SaveCfg(ut::text::Document& doc) const;

	// Prints provided text to the output box widget.
	//    @param text - reference to the string to be displayed.
	//    @return - error if failed.
	ut::Optional<ut::Error> DisplayText(const ut::String& text);

	// Updates client list widget.
	//    @param list - list of clients.
	//    @return - error if failed.
	ut::Optional<ut::Error> UpdateClientList(const ClientList& list);

	// Enables or disables encryption for messages.
	//    @param status - set 'true' to encrypt messages.
	//    @param display_info - set 'true' to display information about
	//                          cahnging encryption status in output widget.
	//    @return - error if failed.
	ut::Optional<ut::Error> ToggleEncryption(bool status, bool display_info = true);

	// Moves ui commands from pull to the dispather. Thread Safe.
	ut::Array< ut::UniquePtr<UiCmd> > DispatchCommands();

private:
	// Callback when client message is to be sent.
	void MessageSent(const ut::String& message, const ut::net::HostAddress& address);

	// main window
	ut::UniquePtr<Fl_Double_Window> window;

	// toolbar
	struct Toolbar
	{
		// toolbar container is needed to fix the size of toolbar elements
		// otherwise elements would be horizontally resizable (resizing with the main window)
		ut::UniquePtr<Fl_Double_Window> container;

		// configuration button
		ut::UniquePtr<Fl_Button> configuration;
		ut::UniquePtr<Fl_RGB_Image> configuration_image;

		// encryption button
		ut::UniquePtr<Fl_Button> encryption_on;
		ut::UniquePtr<Fl_RGB_Image> encryption_on_image;
		ut::UniquePtr<Fl_Button> encryption_off;
		ut::UniquePtr<Fl_RGB_Image> encryption_off_image;

		// callbacks
		static void CfgButtonCallback(Fl_Widget* widget, void* data);
		static void ToggleEncryptionCallback(Fl_Widget* widget, void* data);
		static void TurnOffEncryptionCallback(Fl_Widget* widget, void* data);
	} toolbar;

	// window, containing server or client ui data
	ut::UniquePtr<Fl_Group> body_container;
	ut::UniquePtr<BodyUI> body;

	// configuration window
	CfgEditor cfg_editor;

	// cfg reference
	Application& app;

	ut::Synchronized< ut::Array< ut::UniquePtr<UiCmd> > > commands;

	ut::UniquePtr<ut::Thread> cmd_dispatcher_thread;

	// boolean whether to encrypt messages or not
	bool encrypt_messages;

	// minimum width and height of the main window
	static const int skRangeX;
	static const int skRangeY;

	// height of the toolbar
	static const int skToolbarHeight;

	// size of the toolbar element
	static const int skToolbarElementSize;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//