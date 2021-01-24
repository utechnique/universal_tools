//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/input/ve_input_signal.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Creates new discrete signal object.
Signal Signal::CreateDiscrete(Signal::Discrete discrete_signal)
{
	return Signal(discrete_signal);
}

// Creates new analog signal object.
Signal Signal::CreateAnalog(Signal::Analog analog_signal)
{
	return Signal(analog_signal);
}

// Returns a discrete value.
//    @param sensitivity - makes sense if the signal is an analog signal:
//                         this function return 1 if analog value is
//                         higher than sensitivity or 0 otherwise.
//    @return - discrete value of this signal.
Signal::Discrete Signal::GetDiscreteValue(Signal::Analog sensitivity) const
{
	switch (type)
	{
	case type_discrete: return signal.discrete;
	case type_analog: return ut::Abs(signal.analog) >= sensitivity ? 1 : 0;
	default: return false;
	}
}

// Returns an analog value. In case if the signal is a discrete signal:
// this function returns 0.0f if discrete value is zero or 1.0f otherwise.
Signal::Analog Signal::GetAnalogValue() const
{
	switch (type)
	{
	case type_discrete: return signal.discrete ? 1.0f : 0.0f;
	case type_analog: return signal.analog;
	default: return false;
	}
}

// Returns a type of this signal.
Signal::Type Signal::GetType() const
{
	return type;
}

// Constructor - constructs a discrete signal.
Signal::Signal(Signal::Discrete button_key) : type(type_discrete)
{
	signal.discrete = button_key;
}

// Constructor - constructs an analog signal.
Signal::Signal(Signal::Analog analog_key) : type(type_analog)
{
	signal.analog = analog_key;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//