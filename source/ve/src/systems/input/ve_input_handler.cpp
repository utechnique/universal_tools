//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/input/ve_input_handler.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Returns 'true' if a key with provided name is pressed down.
//    @param name - name of the desired signal.
//    @return - true if desired signal was found and is being pressed,
//              and false otherwise.
bool Handler::IsKeyDown(const ut::String& name)
{
	const size_t device_count = cur_state.GetNum();
	for (size_t i = 0; i < device_count; i++)
	{
		ut::Optional<const Signal&> signal = cur_state[i].FindSignal(name);
		if (signal)
		{
			return signal->GetDiscreteValue() != 0 ? true : false;
		}
	}
	return false;
}

// Returns a value of the discrete signal.
//    @param name - name of the desired signal.
//    @return - discrete value of the signal,
//              returns zero if signal wasn't found.
Signal::Discrete Handler::GetDiscreteSignal(const ut::String& name)
{
	const size_t device_count = cur_state.GetNum();
	for (size_t i = 0; i < device_count; i++)
	{
		ut::Optional<const Signal&> signal = cur_state[i].FindSignal(name);
		if (signal)
		{
			return signal->GetDiscreteValue();
		}
	}
	return static_cast<Signal::Discrete>(0);
}

// Returns a value of the analog signal.
//    @param name - name of the desired signal.
//    @return - analog value of the signal,
//              returns zero if signal wasn't found.
Signal::Analog Handler::GetAnalogSignal(const ut::String& name)
{
	const size_t device_count = cur_state.GetNum();
	for (size_t i = 0; i < device_count; i++)
	{
		ut::Optional<const Signal&> signal = cur_state[i].FindSignal(name);
		if (signal)
		{
			return signal->GetAnalogValue();
		}
	}
	return static_cast<Signal::Analog>(0);
}

// Copies @cur_state to @prev_state.
void Handler::SwapStates()
{
	const size_t cur_device_count = cur_state.GetNum();
	const size_t prev_device_count = prev_state.GetNum();

	if (prev_device_count == cur_device_count)
	{
		for (size_t i = 0; i < cur_device_count; i++)
		{
			prev_state[i] = cur_state[i];
		}
	}
	else
	{
		prev_state = cur_state;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//