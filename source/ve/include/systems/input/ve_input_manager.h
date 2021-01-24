//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_input_handler.h"
#include "ve_input_cfg.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// ve::input::Manager provides centralized access to input devices.
class Manager
{
	friend class PollSystem;
public:
	// Constructor.
	Manager();

	// Returns 'true' if a key with provided name is pressed down.
	//    @param name - name of the desired signal.
	//    @return - true if desired signal was found and is being pressed,
	//              and false otherwise.
	bool IsKeyDown(const ut::String& name);

	// Returns a value of the discrete signal.
	//    @param name - name of the desired signal.
	//    @return - discrete value of the signal,
	//              returns zero if signal wasn't found.
	Signal::Discrete GetDiscreteSignal(const ut::String& name);

	// Returns a value of the analog signal.
	//    @param name - name of the desired signal.
	//    @return - analog value of the signal,
	//              returns zero if signal wasn't found.
	Signal::Analog GetAnalogSignal(const ut::String& name);

	// Input settings.
	Config<Settings> config;

private:
	// Updates handlers.
	void Update();

	// Returns a configuration object. Tries to load it from file, and creates
	// a default one if loading failed.
	static Config<Settings> LoadCfg();

	ut::Array< ut::UniquePtr<Handler> > handlers;
};


//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//