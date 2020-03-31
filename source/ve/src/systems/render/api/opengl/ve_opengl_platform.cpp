//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Returns true if opengl context was created and initialized.
// This function is thread-safe.
bool OpenGLDummyWindow::IsInitialized()
{
	return initialized.Read();
}

// The only purpose for draw() method is to make fltk create an opengl context.
// Then this context can be used as a shared parent for our context so that
// we could draw opengl stuff in other windows (that have own context).
void OpenGLDummyWindow::draw()
{
	// skip initialization if it's already done
	if (initialized.Read())
	{
		return;
	}

	// gl_start causes fltk to create a new context for this window
	gl_start();

	// here we can steal opengl context from fltk
	ut::Result<OpenGLContextHandle, ut::Error> shared_context = GetCurrentContext();
	if (!shared_context)
	{
		throw ut::Error(shared_context.MoveAlt());
	}

	// gl_finish must be called after gl_start for proper fltk work
	gl_finish();

	// create a new context using fltk's one as a parent
	ut::Optional<ut::Error> init_error = Initialize(shared_context.GetResult());
	if (init_error)
	{
		throw ut::Error(init_error.Move());
	}

	// detach any context from this thread
	ut::Optional<ut::Error> reset_error = ResetContext();
	if (reset_error)
	{
		throw ut::Error(reset_error.Move());
	}

	// everything is good, initialization successfull
	initialized.Store(true);
}

// OpenGL window can be created and/or deleted only in fltk thread.
void OpenGLDummyWindow::Deleter::operator()(OpenGLDummyWindow* viewport) const
{
	Fl::lock();
	Fl::awake([](void* p) { delete static_cast<OpenGLDummyWindow*>(p); }, viewport);
	Fl::unlock();
}

//----------------------------------------------------------------------------//
// Helper structure to synchronize data between CreateOpenGLDummyWindow()
// function and fltk thread.
typedef ut::Container<OpenGLDummyWindow::UniquePtr&, ut::Mutex&, ut::ConditionVariable&> OpenGLDummySync;

// Helper function creating a new window in fltk thread.
void CreateOpenGLDummyProc(void* p)
{
	UT_ASSERT(p);

	// argument is a pointer to a special wrapper for synchronization data
	OpenGLDummySync* sync = static_cast<OpenGLDummySync*>(p);

	// extract synchronization primitives from temporal container
	OpenGLDummyWindow::UniquePtr& window = sync->Get<OpenGLDummyWindow::UniquePtr&>();
	ut::Mutex& mutex = sync->Get<ut::Mutex&>();
	ut::ConditionVariable& cvar = sync->Get<ut::ConditionVariable&>();

	{ // note that there must be at least one visible fltk window for this code to work
		ut::ScopeLock lock(mutex);

		// top-most window is used as a parent for dummy window
		Fl_Window* top_window = Fl::first_window();
		UT_ASSERT(top_window);

		// create a window and call show() method, otherwise this
		// window won't be drawn and fltk will not create a context for it
		top_window->begin();
		window = OpenGLDummyWindow::UniquePtr(new OpenGLDummyWindow);
		window->show();
		top_window->end();
	}

	// inform CreateOpenGLDummyWindow() function that this function was
	// successfully processed inside fltk thread
	cvar.WakeOne();
}

// Creates new opengl context for the current thread. Synchronizes with fltk
// to make different windows(viewports) accessible for this context.
OpenGLDummyWindow::UniquePtr CreateOpenGLDummyWindow()
{
	// a new window will be put in this pointer
	OpenGLDummyWindow::UniquePtr dummy_window;

	// synchronization data
	ut::Mutex mutex;
	ut::ConditionVariable cvar;
	OpenGLDummySync sync(dummy_window, mutex, cvar);

	// send a request to create a new window to the fltk thread
	Fl::lock();
	Fl::awake(CreateOpenGLDummyProc, &sync);
	Fl::unlock();

	{ // wait for window to be created
		ut::ScopeLock lock(mutex);
		while (!dummy_window)
		{
			cvar.Wait(lock);
		}
	}

	// wait till context is initialized (method Fl_Window::draw() must be called)
	while (!dummy_window->IsInitialized())
	{
		ut::this_thread::Sleep(1);
	}

	// success
	return ut::Move(dummy_window);
}

//----------------------------------------------------------------------------//
// Helper class to lock fltk thread in current scope.
FltkScopeLock::FltkScopeLock() { Fl::lock(); }
FltkScopeLock::~FltkScopeLock() { Fl::unlock(); }

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL && UT_WINDOWS
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//