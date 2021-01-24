//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// ve::input::Signal is an interface encapsulating input control. A signal can
// be analog or discrete. Call ve::input::GetType() to figure out a type.
class Signal
{
	friend class Device;
public:
	// Value type of the discrete signal.
	typedef ut::uint32 Discrete;

	// Value type of the analog signal.
	typedef float Analog;

	// Possible signal types.
	enum Type
	{
		type_discrete,
		type_analog
	};

	// Creates new discrete signal object.
	static Signal CreateDiscrete(Discrete discrete_signal);

	// Creates new analog signal object.
	static Signal CreateAnalog(Analog analog_signal);

	// Returns a discrete value.
	//    @param sensitivity - makes sense if the signal is an analog signal:
	//                         this function return 1 if analog value is
	//                         higher than sensitivity or 0 otherwise.
	//    @return - discrete value of this signal.
	Discrete GetDiscreteValue(Analog sensitivity = 0.75f) const;

	// Returns an analog value. In case if the signal is a discrete signal:
	// this function returns 0.0f if discrete value is zero or 1.0f otherwise.
	Analog GetAnalogValue() const;

	// Returns a type of this signal.
	Type GetType() const;

private:
	// Constructor - constructs a discrete signal.
	Signal(Discrete button_key);

	// Constructor - constructs an analog signal.
	Signal(Analog analog_key);

	// Signal value.
	union
	{
		Discrete discrete;
		Analog analog;
	} signal;

	// Signal type.
	Type type;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//