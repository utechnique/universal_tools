//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if UT_WINDOWS
//----------------------------------------------------------------------------//
#include "ve_input_handler.h"
//----------------------------------------------------------------------------//
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// DirectInput handler for Windows.
class DirectInputHandler : public Handler
{
public:
	// Constructor.
	DirectInputHandler();

	// Updates input devices.
	void Update() override;

private:
	// Helper struct to extract keyboard state.
	struct DIKeyboardState
	{
		BYTE keys[256];
	};

	// Helper struct to extract mouse state.
	struct DIMouseState
	{
		LONG x;
		LONG y;
		BYTE lmb;
		BYTE rmb;
		BYTE mmb;
		BYTE padding;
	};

	// Initializes @di_keyboard.
	HRESULT InitKeyboardDevice();

	// Initializes @di_mouse.
	HRESULT InitMouseDevice();

	// Creates a IDirectInputDevice8 object for all input devices.
	void InitDirectInputDevices();

	// Creates a keyboard device.
	ut::Result<Device, ut::Error> CreateKeyboard();

	// Creates a mouse device.
	ut::Result<Device, ut::Error> CreateMouse();

	// Processes keyboard.
	void ProcessKeyboard(ut::Optional<Device&>& keyboard);

	// Processes mouse.
	void ProcessMouse(ut::Optional<Device&>& mouse);

	// Extracts a keyboard state from @di_keyboard and stores in the
	// provided device object.
	void UpdateKeyboard(Device& keyboard);

	// Extracts a mouse state from @di_keyboard and stores in the
	// provided device object.
	void UpdateMouse(Device& keyboard);

	// DirectInput resources.
	ut::ComPtr<IDirectInput8> dinput;
	ut::ComPtr<IDirectInputDevice8> di_keyboard;
	ut::ComPtr<IDirectInputDevice8> di_mouse;

	// Mouse state from the previous state (to calculate mouse offset).
	ut::Optional<DIMouseState> prev_mouse_state;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // UT_WINDOWS
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//