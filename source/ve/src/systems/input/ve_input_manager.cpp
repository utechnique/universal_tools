//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/input/ve_input_manager.h"
//----------------------------------------------------------------------------//
#if UT_WINDOWS
#include "systems/input/ve_direct_input.h"
#elif UT_LINUX

#endif
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Constructor.
Manager::Manager() : config(LoadCfg())
{
#if UT_WINDOWS
	handlers.Add(ut::MakeUnique<DirectInputHandler>());
#elif UT_LINUX

#endif
}

// Returns 'true' if a key with provided name is pressed down.
//    @param name - name of the desired signal.
//    @return - true if desired signal was found and is being pressed,
//              and false otherwise.
bool Manager::IsKeyDown(const ut::String& name)
{
	const size_t handler_count = handlers.GetNum();
	for (size_t i = 0; i < handler_count; i++)
	{
		if (handlers[i]->IsKeyDown(name))
		{
			return true;
		}
	}
	return false;
}

// Returns a value of the discrete signal.
//    @param name - name of the desired signal.
//    @return - discrete value of the signal,
//              returns zero if signal wasn't found.
Signal::Discrete Manager::GetDiscreteSignal(const ut::String& name)
{
	const size_t handler_count = handlers.GetNum();
	for (size_t i = 0; i < handler_count; i++)
	{
		const Signal::Discrete value = handlers[i]->GetDiscreteSignal(name);
		if (static_cast<Signal::Discrete>(0) != value)
		{
			return value;
		}
	}
	return static_cast<Signal::Discrete>(0);
}

// Returns a value of the analog signal.
//    @param name - name of the desired signal.
//    @return - analog value of the signal,
//              returns zero if signal wasn't found.
Signal::Analog Manager::GetAnalogSignal(const ut::String& name)
{
	const size_t handler_count = handlers.GetNum();
	for (size_t i = 0; i < handler_count; i++)
	{
		const Signal::Analog value = handlers[i]->GetAnalogSignal(name);
		if (!ut::Equal<Signal::Analog>(static_cast<Signal::Analog>(0), value))
		{
			return value;
		}
	}
	return static_cast<Signal::Analog>(0);
}

// Updates handlers.
void Manager::Update()
{
	const size_t handler_count = handlers.GetNum();
	for (size_t i = 0; i < handler_count; i++)
	{
		handlers[i]->SwapStates();
		handlers[i]->Update();
	}
}

// Returns a configuration object. Tries to load it from file, and creates
// a default one if loading failed.
Config<Settings> Manager::LoadCfg()
{
	Config<Settings> config;
	ut::Optional<ut::Error> load_cfg_error = config.Load();
	if (load_cfg_error)
	{
		const ut::error::Code error_code = load_cfg_error->GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "Render config file is absent. Using default configuration..." << ut::cret;
			config.Save();
		}
		else
		{
			ut::log << "Fatal error while loading render config file." << ut::cret;
			throw load_cfg_error.Move();
		}
	}

	return config;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//