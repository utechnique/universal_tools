//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_input_device.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// ve::input::Handler is an interface maintaining input devices.
class Handler
{
public:
	// Updates input devices.
	virtual void Update() = 0;

	// Virtual destructor.
	virtual ~Handler() = default;

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

	// Copies @cur_state to @prev_state.
	void SwapStates();

protected:
	ut::Array<Device> cur_state;

private:
	ut::Array<Device> prev_state;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//