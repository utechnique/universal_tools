//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "desktop_ui.h"
#include "../rc.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// minimum width and height of the main window
const int DesktopUI::skRangeX = 320;
const int DesktopUI::skRangeY = 320;

// height of the toolbar
const int DesktopUI::skToolbarHeight = 48;

// size of the toolbar element
const int DesktopUI::skToolbarElementSize = 40;

//----------------------------------------------------------------------------//
// This callback opens configuration editor window.
//    @param widget - pointer to the button widget.
//    @param data - expected to be a pointer to the DesktopUI object.
void DesktopUI::Toolbar::CfgButtonCallback(Fl_Widget* widget, void* data)
{
	UT_ASSERT(data != nullptr);
	DesktopUI* ui = static_cast<DesktopUI*>(data);
	ut::Optional<ut::Error> open_error = ui->OpenCfgEditor();
	if (open_error)
	{
		throw ut::Error(open_error.Move());
	}
}

// This callback enables encryption for messages.
//    @param widget - pointer to the button widget.
//    @param data - expected to be a pointer to the DesktopUI object.
void DesktopUI::Toolbar::ToggleEncryptionCallback(Fl_Widget* widget, void* data)
{
	UT_ASSERT(data != nullptr);
	DesktopUI* ui = static_cast<DesktopUI*>(data);
	ut::Optional<ut::Error> toggle_error = ui->ToggleEncryption(true);
	if (toggle_error)
	{
		throw ut::Error(toggle_error.Move());
	}
}

// This callback disables encryption for messages.
//    @param widget - pointer to the button widget.
//    @param data - expected to be a pointer to the DesktopUI object.
void DesktopUI::Toolbar::TurnOffEncryptionCallback(Fl_Widget* widget, void* data)
{
	UT_ASSERT(data != nullptr);
	DesktopUI* ui = static_cast<DesktopUI*>(data);
	ut::Optional<ut::Error> turn_off_error = ui->ToggleEncryption(false);
	if (turn_off_error)
	{
		throw ut::Error(turn_off_error.Move());
	}
}

//----------------------------------------------------------------------------//
// Forward declaration for the dispatcher callback;
void DispatcherCallback(void*);

//----------------------------------------------------------------------------//
// Job for dispatching UI commands.
class UiDispatcherJob : public ut::Job
{
	// Only dispatcher callback has access to private methods.
	friend void DispatcherCallback(void*);
public:
	// Constructor, UI object reference must be provided
	UiDispatcherJob(DesktopUI& in_ui) : ui(in_ui)
	{ }

	// Runs a dispatching loop.
	void Execute()
	{
		while (!exit_request.Read())
		{
			// dispatch commands
			commands = ui.DispatchCommands();

			// do something only if there is at least one command
			if (commands.GetNum())
			{
				// marking commands as uncomplete to make possible
				// waiting for completion
				MarkUncomplete();

				// send commands to the main thread and execute them
				Fl::awake(DispatcherCallback, this);

				// wait before all commands will be executed
				WaitCompletion();
			}

			// give other threads a chance to run
			ut::this_thread::Sleep(1);
		}
	}

private:
	// Dispatcher callback grabs commands from the main thread
	// using this method.
	ut::Array< ut::UniquePtr<UiCmd> > GrabCommands()
	{
		ut::Array< ut::UniquePtr<UiCmd> > out(commands);
		commands.Empty();
		return out;
	}

	// Commands need to be marked as uncomplete using interlocked
	// variable to make possible knowing when these commands will
	// be fully executed.
	void MarkUncomplete()
	{
		ut::atomics::interlocked::Store(&state, 0);
	}

	// Dispatcher callback informs dispather job that all commands
	// have been executed using this method.
	void Complete()
	{
		ut::atomics::interlocked::Store(&state, 1);
	}

	// Waits untill all commands that have been passed to the main
	// thread will be executed.
	void WaitCompletion()
	{
		while (ut::atomics::interlocked::Read(&state) == 0)
		{
			ut::this_thread::Sleep(1);
		}
	}

	// interlocked variable to hold completion state of the commands
	ut::int32 state;

	// command buffer
	ut::Array< ut::UniquePtr<UiCmd> > commands;

	// reference to the UI object
	DesktopUI& ui;
};

// Dispatcher callback grabs and executes commands from the main thread.
void DispatcherCallback(void* ptr)
{
	// convert single parameter to the pointer to dispatcher job.
	UiDispatcherJob* job = static_cast<UiDispatcherJob*>(ptr);

	// execute commands
	ut::Array< ut::UniquePtr<UiCmd> > commands = job->GrabCommands();
	for (size_t i = 0; i < commands.GetNum(); i++)
	{
		commands[i]->Execute();
	}

	// inform job that all commands were executed
	job->Complete();
}

//----------------------------------------------------------------------------//
// Constructor, parent application must be provided.
// All widgets are initialized here.
//    @param application - reference to parent (owning) application object
DesktopUI::DesktopUI(Application& application) : app(application)
                                               , cfg_editor(*this)
                                               , encrypt_messages(false)
{
	// get configuration copy
	Configuration cfg = app.GetCfg();

	// calculate all metrics
	int toolbar_element_offset = (skToolbarHeight - skToolbarElementSize) / 2;
	int interior_width = cfg.ui.width;
	int interior_height = cfg.ui.height - skToolbarHeight;
	int text_width = static_cast<int>(static_cast<float>(interior_width) /
		(cfg.ui.lr_ratio + 1.0f));
	int list_width = interior_width - text_width;
	int input_height = static_cast<int>(static_cast<float>(interior_height) /
		(cfg.ui.tb_ratio + 1.0f));
	int output_height = interior_height - input_height;

	// set scheme
	Fl::scheme("plastic");

	// create main window
	window = new Fl_Double_Window(cfg.ui.position_x,
	                              cfg.ui.position_y,
	                              cfg.ui.width,
	                              cfg.ui.height,
	                              "Commie");
	window->size_range(skRangeX, skRangeY);

	// create toolbar
	toolbar.container = new Fl_Double_Window(0, 0,
	                                         interior_width,
	                                         skToolbarHeight,
	                                         "Toolbar");

	toolbar.configuration = new Fl_Button(toolbar_element_offset,
	                                      toolbar_element_offset,
	                                      skToolbarElementSize,
	                                      skToolbarElementSize);
	toolbar.configuration_image = new Fl_RGB_Image(resources::gkOptionsIcon.data,
	                                               resources::gkOptionsIcon.width,
	                                               resources::gkOptionsIcon.height,
	                                               4);
	toolbar.configuration->image(toolbar.configuration_image.Get());
	toolbar.configuration->callback(Toolbar::CfgButtonCallback, this);
	toolbar.configuration->visible_focus(0);


	toolbar.encryption_on = new Fl_Button(toolbar_element_offset * 2 + skToolbarElementSize,
	                                   toolbar_element_offset,
	                                   skToolbarElementSize,
	                                   skToolbarElementSize);
	toolbar.encryption_on_image = new Fl_RGB_Image(resources::gkClosedLockIcon.data,
	                                               resources::gkClosedLockIcon.width,
	                                               resources::gkClosedLockIcon.height,
	                                               4);
	toolbar.encryption_on->image(toolbar.encryption_on_image.Get());
	toolbar.encryption_on->callback(Toolbar::TurnOffEncryptionCallback, this);
	toolbar.encryption_on->visible_focus(0);
	toolbar.encryption_on->hide();

	toolbar.encryption_off = new Fl_Button(toolbar_element_offset * 2 + skToolbarElementSize,
	                                       toolbar_element_offset,
	                                       skToolbarElementSize,
	                                       skToolbarElementSize);
	toolbar.encryption_off_image = new Fl_RGB_Image(resources::gkOpenedLockIcon.data,
	                                                resources::gkOpenedLockIcon.width,
	                                                resources::gkOpenedLockIcon.height,
	                                                4);
	toolbar.encryption_off->image(toolbar.encryption_off_image.Get());
	toolbar.encryption_off->callback(Toolbar::ToggleEncryptionCallback, this);
	toolbar.encryption_off->visible_focus(0);

	toolbar.container->end();

	// set encryption status
	ut::Optional<ut::Error> toggle_encryption_error = ToggleEncryption(encrypt_messages, false);
	if (toggle_encryption_error)
	{
		throw ut::Error(toggle_encryption_error.Move());
	}

	// create body
	body_container = new Fl_Double_Window(0, skToolbarHeight, interior_width, interior_height);
	body_container->resizable(body_container.Get());
	if (cfg.server_mode)
	{
		body = new ServerUI(application, body_container.Get());
	}
	else
	{
		// create client interface
		ClientUI* client_ui = new ClientUI(application, body_container.Get(), cfg.ui.lr_ratio, cfg.ui.tb_ratio);

		// bind signals
		ut::MemberInvoker<void (DesktopUI::*)(const ut::String&,
		                                      const ut::net::HostAddress&)> msg_sent_callback(&DesktopUI::MessageSent, this);
		client_ui->message_sent.Connect(msg_sent_callback);

		// set body pointer
		body = client_ui;
	}
	body_container->end();

	// mutable area limits
	window->resizable(body_container.Get());

	// finish main window
	window->end();

	// show main window
	window->show(0, nullptr);
}

//----------------------------------------------------------------------------->
// Launches user interface
void DesktopUI::Run()
{
	// Start dispatcher thread:
	// FLTK is very buggy in multithreading mode, so it was decided to run
	// all widget-based actions only in the main thread. Thus we need a
	// dispatching thread that will be grabbing commands from the UI layer
	// and pass them into the main thread to be processed by fltk.
	ut::UniquePtr<ut::Job> job(new UiDispatcherJob(*this));
	cmd_dispatcher_thread = new ut::Thread(ut::Move(job));

	// lock fltk so that we could pass commands to the main thread
	// using Fl::awake(cmd) function
	Fl::lock();

	// run user interface routine
	Fl::run();

	// stop dispatcher thread before unlocking fltk
	cmd_dispatcher_thread.Delete();

	// unlock fltk to gracefully exit
	Fl::unlock();
}

//----------------------------------------------------------------------------->
// Opens configuration editor.
//    @return - error if failed to open the editor
ut::Optional<ut::Error> DesktopUI::OpenCfgEditor()
{
	Configuration cfg = app.GetCfg();
	return cfg_editor.Open(cfg);
}

//----------------------------------------------------------------------------->
// Synchronizes internal UI data, options and preferences with
// main application. Just save your data into provided cfg reference.
//    @param cfg - reference to the configuration object
//                 of the main application
void DesktopUI::SyncCfg(Configuration& cfg)
{
	// main window parameters
	cfg.ui.position_x = window->x();
	cfg.ui.position_y = window->y();
	cfg.ui.width = window->w();
	cfg.ui.height = window->h();

	// save client widgets' parameters
	if (body != nullptr)
	{
		const ut::DynamicType& dynamic_type = body->Identify();
		if (dynamic_type.GetName() == ut::GetPolymorphicName<ClientUI>())
		{
			ClientUI* client_ui = static_cast<ClientUI*>(body.Get());
			cfg.ui.lr_ratio = client_ui->GetHorizontalRatio();
			cfg.ui.tb_ratio = client_ui->GetVerticalRatio();
		}
	}
}

//----------------------------------------------------------------------------->
// Forces main application to update it's configuration object
// by deserializing the provided text document.
//    @param doc - reference to the source text document.
//    @return - error if failed to save configuration.
ut::Optional<ut::Error> DesktopUI::SaveCfg(ut::text::Document& doc) const
{
	// update configuration
	ut::Optional<ut::Error> update_cfg_error = app.UpdateCfg(doc);
	if (update_cfg_error)
	{
		return ut::Error(update_cfg_error.Move());
	}

	// save configuration to file
	return app.SaveCfgFile();
}

//----------------------------------------------------------------------------->
// Prints provided text to the output box widget.
//    @param text - reference to the string to be displayed.
//    @return - error if failed.
ut::Optional<ut::Error> DesktopUI::DisplayText(const ut::String& text)
{
	// create command
	ut::UniquePtr<UiCmd> cmd(new DisplayTextCmd(body.GetRef(), text));

	// lock command buffer
	ut::ScopeSyncLock< ut::Array< ut::UniquePtr<UiCmd> > > cmd_lock(commands);
	ut::Array< ut::UniquePtr<UiCmd> >& locked_commands = cmd_lock.Get();

	// add command
	locked_commands.Add(ut::Move(cmd));

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Updates client list widget.
//    @param list - list of clients.
//    @return - error if failed.
ut::Optional<ut::Error> DesktopUI::UpdateClientList(const ClientList& list)
{
	// save client widgets' parameters
	if (body != nullptr)
	{
		const ut::DynamicType& dynamic_type = body->Identify();
		if (dynamic_type.GetName() == ut::GetPolymorphicName<ClientUI>())
		{
			// get client ui
			ClientUI& client_ui = static_cast<ClientUI&>(body.GetRef());

			// create command
			ut::UniquePtr<UiCmd> cmd(new UpdateClientListCmd(client_ui, list));

			// lock command buffer
			ut::ScopeSyncLock< ut::Array< ut::UniquePtr<UiCmd> > > cmd_lock(commands);
			ut::Array< ut::UniquePtr<UiCmd> >& locked_commands = cmd_lock.Get();

			// add command
			locked_commands.Add(ut::Move(cmd));
		}
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Enables or disables encryption for messages.
//    @param status - set 'true' to encrypt messages.
//    @return - error if failed.
ut::Optional<ut::Error> DesktopUI::ToggleEncryption(bool status, bool display_info)
{
	// change status variable
	encrypt_messages = status;

	// string containing information about current encryption status
	ut::String info_string;

	// hide one button and show instead another one
	if (encrypt_messages)
	{
		toolbar.encryption_off->hide();
		toolbar.encryption_on->show();
		info_string = "[encryption: on]";
	}
	else
	{
		toolbar.encryption_on->hide();
		toolbar.encryption_off->show();
		info_string = "[encryption: off]";
	}

	// display information in output widget
	if (display_info)
	{
		DisplayText(info_string + ut::cret);
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Moves ui commands from pull to the dispather. Thread Safe.
ut::Array< ut::UniquePtr<UiCmd> > DesktopUI::DispatchCommands()
{
	// lock command buffer
	ut::ScopeSyncLock< ut::Array< ut::UniquePtr<UiCmd> > > cmd_lock(commands);
	ut::Array< ut::UniquePtr<UiCmd> >& locked_commands = cmd_lock.Get();

	// move commands
	ut::Array< ut::UniquePtr<UiCmd> > out;
	out = Move(locked_commands);
	locked_commands.Empty();
	return out;
}

//----------------------------------------------------------------------------->
// Callback when client message is to be sent.
void DesktopUI::MessageSent(const ut::String& message,
                            const ut::net::HostAddress& address)
{
	message_sent(message, address, encrypt_messages);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//