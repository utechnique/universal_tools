//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_backend.h"
#include "commands/ve_cmd_exit.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor.
Backend::Backend(ut::SharedPtr<Frontend::Thread> in_frontend_thread) : System("ui")
                                                                     , frontend_thread(ut::Move(in_frontend_thread))
{}

// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
System::Result Backend::Update(ComponentAccess& access)
{
	// return value
	ut::Array< ut::UniquePtr<Cmd> > commands;

	// process frontend events
	System::Result frontend_cmd;
	frontend_thread->Enqueue([&](Frontend& frontend) { frontend_cmd = frontend.Update(access); });
	if (frontend_cmd)
	{
		commands += frontend_cmd.Move();
	}

	// return array of commands that are to be
	// executed by owning environment
	return commands;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//