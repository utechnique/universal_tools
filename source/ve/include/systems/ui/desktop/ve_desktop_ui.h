//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_frontend.h"
#include "systems/ui/desktop/ve_entity_browser.h"
#include "systems/ui/desktop/ve_desktop_menu_bar.h"
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/ui/desktop/ve_viewport_area.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Double_Window.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::DesktopFrontend class implements user interface for desktop systems.
class DesktopFrontend : public Frontend
{
	// ve::ui::MainWindow is the top-most window, container for all other
	// windows.
	class MainWindow : public Window
	{
	public:
		// Constructor.
		MainWindow(int x, int y,
		           int w, int h,
		           const char* title,
		           DesktopFrontend& desktop_frontend,
		           const Theme& theme);

		// Overriden handle method. Frontend thread starts only after this
		// method receives focus event message.
		int handle(int event) override;

		// Hides this window and deactivates viewports.
		void hide() override;

		// Virtual destructor for the polymorphic type.
		virtual ~MainWindow() override = default;
	private:
		class DesktopFrontend& frontend;
	};

	friend MainWindow;

public:
	// Constructor.
	DesktopFrontend();

	// Destructor, hides main window and waits until fltk thread is finished.
	~DesktopFrontend();

	// Initialization.
	ut::Optional<ut::Error> Initialize();

	// Launches user interface.
	void Run();

	// Hides all child windows.
	void HideChildWindows();

	// Loads ui configuration from the file.
	ut::Result<Config<Settings>, ut::Error> LoadCfg();

	// Saves current ui configuration to the file.
	void SaveCfg();

	// Processes UI events.
	System::Result Update(ComponentAccess& access) override;

	// One can start iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::Ref<Viewport> >::Iterator BeginViewports() override;

	// One can end iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::Ref<Viewport> >::Iterator EndViewports() override;

	// One can start iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::Ref<Viewport> >::ConstIterator BeginViewports() const override;

	// One can end iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::Ref<Viewport> >::ConstIterator EndViewports() const override;

private:
	// Exit callback that is called before closing the main window.
	static void OnCloseCallback(Fl_Widget* widget, void* data);

	// Destroys internal resources before closing.
	void Close();

	// Synchronization variable to detect when widgets
	// are initialized in fltk thread. It's triggered only when
	// the main window receives a focus. It's the only point where
	// all possible fltk resources are guaranteed to be initialized.
	ut::Atomic<bool> window_ready;

	// Indicates that UI is closed completely.
	ut::Atomic<bool> exit;

	// fltk is single threaded, thus all widgets must be created and
	// processed in a separate thread
	ut::UniquePtr<ut::Thread> fltk_thread;

	// main window
	ut::UniquePtr<MainWindow> window;

	// ui modules
	ut::UniquePtr<EntityBrowser> entity_browser;
	ut::UniquePtr<Window> wnd;

	// system menu
	ut::UniquePtr<MenuBar> menu;

	// container for all viewports
	ut::UniquePtr<ViewportArea> viewport_area;

	// array of viewport references
	ut::Array< ut::Ref<Viewport> > viewports;

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
