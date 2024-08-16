//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui.h"
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
#include "commands/ve_cmd_exit.h"
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

//----------------------------------------------------------------------------//
// Callback to hide an fltk window.
void HideWindow(void* wnd_ptr)
{
	static_cast<Fl_Window*>(wnd_ptr)->hide();
}

//----------------------------------------------------------------------------//
// Constructor.
DesktopFrontend::MainWindow::MainWindow(int x, int y,
                                        int w, int h,
                                        const char* title,
                                        DesktopFrontend& desktop_frontend,
                                        const Theme& theme) : Window(x, y, w, h, title, theme)
                                                            , frontend(desktop_frontend)
{}

// Overriden handle method.
int DesktopFrontend::MainWindow::handle(int event)
{
	frontend.window_ready.Store(true);
    return Window::handle(event);
}

// Hides this window and deactivates viewports.
void DesktopFrontend::MainWindow::show()
{
	hidden_maximized = false;
	Window::show();
}

// Hides this window and deactivates viewports.
void DesktopFrontend::MainWindow::hide()
{
	hidden_maximized = IsMaximized();

	const size_t viewport_count = frontend.viewports.Count();
	for (size_t i = 0; i < viewport_count; i++)
	{
		Viewport::Mode mode = frontend.viewports[i]->GetMode();
		mode.is_active = false;
		frontend.viewports[i]->SetMode(mode);
	}

	frontend.HideChildWindows();
	Window::hide();
}

// Returns true if this window is in maximized state.
bool DesktopFrontend::MainWindow::IsMaximized() const
{
	if (hidden_maximized)
	{
		return true;
	}

#if UT_WINDOWS
	CONST HWND hwnd = fl_xid(this);

	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);

	if (!GetWindowPlacement(hwnd, &placement))
	{
		return false;
	}

	return placement.showCmd == SW_MAXIMIZE;
#else
	return false;
#endif
}

// Maximizes current window.
void DesktopFrontend::MainWindow::InitializeMaximized()
{
#if UT_WINDOWS
	CONST HWND hwnd = fl_xid(this);

	ShowWindow(hwnd, SW_SHOWMAXIMIZED);

	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(hwnd, &placement))
	{
		placement.rcNormalPosition.left = Fl::w() / 4;
		placement.rcNormalPosition.top = Fl::h() / 4;
		placement.rcNormalPosition.right = placement.rcNormalPosition.left + Fl::w() / 2;
		placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + Fl::h() / 2;

		SetWindowPlacement(hwnd, &placement);
	}
#endif
}

//----------------------------------------------------------------------------//
// Constructor.
DesktopFrontend::DesktopFrontend() : window_ready(false), exit(false)
{
	fltk_thread = ut::MakeUnique<ut::Thread>([this] { this->Run(); });

	// wait until the main window appears on screen
	while (!window_ready.Read())
	{
		ut::this_thread::Sleep(1);
	}
}

// Destructor, hides main window and waits until fltk thread is finished.
DesktopFrontend::~DesktopFrontend()
{
	Fl::awake(HideWindow, window.Get());
	fltk_thread->Join();
}

//----------------------------------------------------------------------------->
// Initialization.
ut::Optional<ut::Error> DesktopFrontend::Initialize()
{
    // enable multithreading for XLib
#if VE_X11
    XInitThreads();
#endif

	// get configuration copy
	ut::Result<Config<Settings>, ut::Error> cfg = LoadCfg();
	if (!cfg)
	{
		return cfg.MoveAlt();
	}

	// get theme
	const Theme& theme = cfg->theme;

	// set scheme
	Fl::scheme("base");

	// set colors
	Fl::background(theme.background_color.R(),
	               theme.background_color.G(),
	               theme.background_color.B());

	Fl::background2(theme.background_color.R(),
	                theme.background_color.G(),
	                theme.background_color.B());

	Fl::foreground(theme.foreground_color.R(),
	               theme.foreground_color.G(),
	               theme.foreground_color.B());

	Fl::set_color(FL_LIGHT3,
	              static_cast<ut::byte>(theme.foreground_color.R() * 0.7f),
	              static_cast<ut::byte>(theme.foreground_color.G() * 0.7f),
	              static_cast<ut::byte>(theme.foreground_color.B() * 0.7f));

	// tab
	Fl::set_color(55, theme.primary_tab_color.R(), theme.primary_tab_color.G(), theme.primary_tab_color.B());

	// frame
	Fl::set_color(32, theme.background_color.R(), theme.background_color.G(), theme.background_color.B());
	Fl::set_color(42, theme.frame_color.R(), theme.frame_color.G(), theme.frame_color.B());
	Fl::set_color(44, theme.frame_color.R(), theme.frame_color.G(), theme.frame_color.B());
	Fl::set_color(51, theme.background_color.R(), theme.background_color.G(), theme.background_color.B());
	Fl::set_color(54, theme.frame_color.R(), theme.frame_color.G(), theme.frame_color.B());

	// independent modules
	entity_browser = ut::MakeUnique<EntityBrowser>(cfg->window.offset.X(),
	                                               cfg->window.offset.Y(),
	                                               EntityBrowser::skDefaultWidth,
	                                               EntityBrowser::skDefaultHeight,
	                                               theme);
	entity_browser->hide();
	entity_browser->end();

	// create main window
	window = ut::MakeUnique<MainWindow>(cfg->window.offset.X(),
	                                    cfg->window.offset.Y(),
	                                    cfg->window.extent.X(),
	                                    cfg->window.extent.Y(),
	                                    skTitle,
	                                    *this,
	                                    cfg->theme);
	window->size_range(skMinWidth, skMinHeight);
	window->callback(DesktopFrontend::OnCloseCallback, this);

	// menu
	menu = ut::MakeUnique<MenuBar>(cfg.Get(), 0, 0, window->w(), entity_browser.GetRef());

	// viewport container
	viewport_area = ut::MakeUnique<ViewportArea>(cfg.Get(),
	                                             0,
	                                             menu->h(),
	                                             window->w(),
	                                             window->h() - menu->h());
	viewports = viewport_area->GetViewports();

	// adjust main window
	window->resizable(viewport_area.Get());

	// finish main window
	window->end();

	// show main window
	window->show();
	window->take_focus();

	// select layout
	viewport_area->ChangeLayout(cfg->layout_id);

	// update viewports according to the cfg data
	viewport_area->ResizeViewports(cfg->viewports);
	viewport_area->SetViewportProjections(cfg->projections);
	viewport_area->SetViewportRenderModes(cfg->render_modes);

	// maximize the main window if needed
	if (cfg->maximized)
	{
		window->InitializeMaximized();
	}

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
		ut::log << init_error->GetDesc() + ut::cret;
		throw ut::Error(init_error.Move());
	}

	// lock fltk so that one could pass commands to the main thread
	// using Fl::awake() function
	Fl::lock();

	// run user interface routine
	Fl::run();

	// unlock fltk to gracefully exit
	Fl::unlock();

	// save config file
	SaveCfg();

	// exit signal
	exit.Store(true);
}

//----------------------------------------------------------------------------->
// Hides all child windows.
void DesktopFrontend::HideChildWindows()
{
	entity_browser->hide();
}

//----------------------------------------------------------------------------->
// Loads ui configuration from the file.
ut::Result<Config<Settings>, ut::Error> DesktopFrontend::LoadCfg()
{
	Config<Settings> cfg;
	ut::Optional<ut::Error> load_error = cfg.Load();
	if (load_error)
	{
		const ut::error::Code error_code = load_error->GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "UI config file is absent. Using default configuration..." << ut::cret;
		}
		else
		{
			ut::log << "Fatal error while loading UI config file." << ut::cret;
			return ut::MakeError(load_error.Move());
		}
	}

	// load theme
	load_error = cfg.theme.Load();
	if (load_error)
	{
		const ut::error::Code error_code = load_error->GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "Theme file is absent. Using default colors..." << ut::cret;
			cfg.theme.Save();
		}
	}


	// success
	return cfg;
}

//----------------------------------------------------------------------------->
// Saves current ui configuration to the file.
void DesktopFrontend::SaveCfg()
{
	Config<Settings> cfg;
	cfg.Load();

	// main window parameters
	cfg.window.offset.X() = window->x();
	cfg.window.offset.Y() = window->y();
	cfg.window.extent.X() = window->w();
	cfg.window.extent.Y() = window->h();
	cfg.maximized = window->IsMaximized();

	// viewport parameters
	cfg.layout_id = viewport_area->GetCurrentLayoutId();

	// viewports
	cfg.projections = viewport_area->GetViewportProjections();
	cfg.viewports = viewport_area->GetViewportRects();
	cfg.render_modes = viewport_area->GetViewportRenderModes();

	// save to file
	cfg.Save();
}

//----------------------------------------------------------------------------->
// Processes UI events.
System::Result DesktopFrontend::Update(ComponentAccessGroup& access)
{
	UT_ASSERT(entity_browser);
	
	CmdArray commands = entity_browser->UpdateEntities(access);

	if (exit.Read())
	{
		commands.Add(ut::MakeUnique<CmdExit>());
	}

	return commands;
}

//----------------------------------------------------------------------------->
// One can start iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::Ref<Viewport> >::Iterator DesktopFrontend::BeginViewports()
{
	return viewports.Begin();
}

// One can end iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::Ref<Viewport> >::Iterator DesktopFrontend::EndViewports()
{
	return viewports.End();
}

// One can start iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::Ref<Viewport> >::ConstIterator DesktopFrontend::BeginViewports() const
{
	return viewports.Begin();
}

// One can end iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::Ref<Viewport> >::ConstIterator DesktopFrontend::EndViewports() const
{
	return viewports.End();
}

//----------------------------------------------------------------------------->
// Exit callback that is called before closing the main window.
void DesktopFrontend::OnCloseCallback(Fl_Widget* widget, void* data)
{
	UT_ASSERT(widget != nullptr);
	UT_ASSERT(data != nullptr);
	DesktopFrontend* frontend = static_cast<DesktopFrontend*>(data);

	frontend->Close();
	widget->hide();
}

//----------------------------------------------------------------------------->
// Destroys internal resources before closing.
void DesktopFrontend::Close()
{
	// make viewports to call signal slots before their windows got destroyed
	const size_t viewport_count = viewports.Count();
	for (size_t i = 0; i < viewport_count; i++)
	{
		DesktopViewport& viewport = static_cast<DesktopViewport&>(viewports[i].Get());
		viewport.CloseSignal();
		viewport.ResetSignals();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
