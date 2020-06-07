//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_frontend.h"
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "ve_id_generator.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Double_Window.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::MainWindow is the top-most window, container for all other
// windows.
class MainWindow : public Fl_Window
{
public:
    // Constructor.
    MainWindow(int x, int y,
               int w, int h,
               const char* title,
               ut::Atomic<bool>& ini_ref);

    // Overriden handle method. Frontend thread starts only after this
	// method receives focus event message.
    int handle(int event) override;

private:
    ut::Atomic<bool>& initialized;
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
	// Exit callback that is called before closing the main window.
	static void OnCloseCallback(Fl_Widget* widget, void* data);

	// Destroys internal resources before closing.
	void Close();

	// Synchronization variable to detect when widgets
	// are initialized in fltk thread. It's triggered only when
	// the main window receives focus. It's the only point where
	// all possible fltk resources are guaranteed to be initialized.
	ut::Atomic<bool> window_ready;

	// fltk is single threaded, thus all widgets must be created and
	// processed in a separate thread
	ut::UniquePtr<ut::Thread> fltk_thread;

	// main window
	ut::UniquePtr<MainWindow> window;

	// Id-generator is used to generate unique identifiers for viewports.
	IdGenerator<Viewport::Id> viewport_id_generator;

	// Minimum width of the window
	static const ut::uint32 skMinWidth;

	// Minimum height of the window
	static const ut::uint32 skMinHeight;
};

// Desktop is the current UI frontend.
typedef DesktopFrontend PlatformFrontend;

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
