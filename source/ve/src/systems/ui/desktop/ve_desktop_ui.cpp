//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui.h"
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
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
MainWindow::MainWindow(int x, int y,
                       int w, int h,
                       const char* title,
                       ut::Atomic<bool>& ini_ref) : Fl_Window(x, y, w, h, title)
                                                  , initialized(ini_ref)
{}

// Overriden handle method.
int MainWindow::handle(int event)
{
    if(event == FL_FOCUS)
    {
        initialized.Store(true);
    }
    return Fl_Window::handle(event);
}

//----------------------------------------------------------------------------//
// Constructor.
DesktopFrontend::DesktopFrontend() : window_ready(false)
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
			return load_error.Move();
		}
	}

	// set scheme
	Fl::scheme("base");

	// set colors
	Fl::background(cfg.background_color.R(),
	               cfg.background_color.G(),
	               cfg.background_color.B());

	Fl::background2(cfg.background_color.R(),
	                cfg.background_color.G(),
	                cfg.background_color.B());

	Fl::foreground(cfg.foreground_color.R(),
	               cfg.foreground_color.G(),
	               cfg.foreground_color.B());

	Fl::set_color(FL_LIGHT3,
	              static_cast<ut::byte>(cfg.foreground_color.R() * 0.7f),
	              static_cast<ut::byte>(cfg.foreground_color.G() * 0.7f),
	              static_cast<ut::byte>(cfg.foreground_color.B() * 0.7f));

	Fl::set_color(55, cfg.tab_color.R(), cfg.tab_color.G(), cfg.tab_color.B());

	// create main window
	window = ut::MakeUnique<MainWindow>(cfg.window.offset.X(),
	                                    cfg.window.offset.Y(),
	                                    cfg.window.extent.X(),
	                                    cfg.window.extent.Y(),
	                                    skTitle,
	                                    window_ready);
	window->size_range(skMinWidth, skMinHeight);
	window->callback(DesktopFrontend::OnCloseCallback, this);

	// viewport container
	viewport_area = ut::MakeUnique<ViewportArea>(cfg,
	                                             0,
	                                             0,
	                                             window->w(),
	                                             window->h());
	viewports = viewport_area->GetViewports();

	// adjust main window
	window->resizable(viewport_area.Get());

	// finish main window
	window->end();

	// show main window
	window->show(0, nullptr);

	// select layout
	viewport_area->ChangeLayout(cfg.layout_id);

	// update viewports according to the cfg data
	viewport_area->ResizeViewports(cfg.viewports);
	viewport_area->SetViewportProjections(cfg.projections);
	viewport_area->SetViewportRenderModes(cfg.render_modes);

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

	// run user interface routine
	Fl::run();

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
	Config<Settings> cfg;
	cfg.Load();

	// main window parameters
	cfg.window.offset.X() = window->x();
	cfg.window.offset.Y() = window->y();
	cfg.window.extent.X() = window->w();
	cfg.window.extent.Y() = window->h();

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
	const size_t viewport_count = viewports.GetNum();
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
