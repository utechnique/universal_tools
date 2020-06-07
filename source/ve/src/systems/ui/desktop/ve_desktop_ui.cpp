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

//----------------------------------------------------------------------------->
// Destructor
DesktopFrontend::~DesktopFrontend()
{}

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
		const ut::error::Code error_code = load_error.Get().GetCode();
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
	Fl::scheme("plastic");

	// create main window
	window = ut::MakeUnique<MainWindow>(cfg->position_x,
	                                    cfg->position_y,
	                                    cfg->width,
	                                    cfg->height,
	                                    skTitle,
	                                    window_ready);
	window->size_range(skMinWidth, skMinHeight);
	window->callback(DesktopFrontend::OnCloseCallback, this);

	// create render area
	Viewport::Id viewport_id = viewport_id_generator.Generate();
	ut::UniquePtr<DesktopViewport> viewport;
	viewport = ut::MakeUnique<DesktopViewport>(viewport_id, "1", 0, 0, window->w(), window->h());

	// adjust main window
	window->resizable(viewport.Get());

	// add viewport to the map
	if (!viewports.Add(ut::Move(viewport)))
	{
		return ut::Error(ut::error::out_of_memory);
	}

	// finish main window
	window->end();

	// show main window
	window->show(0, nullptr);

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
		ut::log << init_error.Get().GetDesc() + ut::cret;
		throw ut::Error(init_error.Move());
	}

	// lock fltk so that we could pass commands to the main thread
	// using Fl::awake(cmd) function
	Fl::lock();

	// run user interface routine
	Fl::run();

	// unlock fltk to gracefully exit
	Fl::unlock();

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

	// main window parameters
	cfg->position_x = window->x();
	cfg->position_y = window->y();
	cfg->width = window->w();
	cfg->height = window->h();

	// save to file
	cfg.Save();
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
		UT_ASSERT(viewports[i]);
		DesktopViewport& viewport = static_cast<DesktopViewport&>(viewports[i].GetRef());
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
