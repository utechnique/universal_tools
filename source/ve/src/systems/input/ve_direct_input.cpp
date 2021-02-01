//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#if UT_WINDOWS
//----------------------------------------------------------------------------//
#include "systems/input/ve_direct_input.h"
#include "systems/input/ve_input_keyboard.h"
#include "systems/input/ve_input_mouse.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Map to find corresponding DirectInput key of the ve::input::keyboard::Button
// enumeration.
static const size_t skDIKeyboardMap[keyboard::button_count] =
{
	DIK_ESCAPE,      DIK_1,        DIK_2,            DIK_3,
	DIK_4,           DIK_5,        DIK_6,            DIK_7,
	DIK_8,           DIK_9,        DIK_0,            DIK_MINUS,
	DIK_EQUALS,      DIK_BACK,     DIK_TAB,          DIK_Q,
	DIK_W,           DIK_E,        DIK_R,            DIK_T,
	DIK_Y,           DIK_U,        DIK_I,            DIK_O,
	DIK_P,           DIK_LBRACKET, DIK_RBRACKET,     DIK_RETURN,
	DIK_LCONTROL,    DIK_A,        DIK_S,            DIK_D,
	DIK_F,           DIK_G,        DIK_H,            DIK_J,
	DIK_K,           DIK_L,        DIK_SEMICOLON,    DIK_APOSTROPHE,
	DIK_GRAVE,       DIK_LSHIFT,   DIK_BACKSLASH,    DIK_Z,
	DIK_X,           DIK_C,        DIK_V,            DIK_B,
	DIK_N,           DIK_M,        DIK_COMMA,        DIK_PERIOD,
	DIK_SLASH,       DIK_RSHIFT,   DIK_MULTIPLY,     DIK_LMENU,
	DIK_SPACE,       DIK_CAPITAL,  DIK_F1,           DIK_F2,
	DIK_F3,          DIK_F4,       DIK_F5,           DIK_F6,
	DIK_F7,          DIK_F8,       DIK_F9,           DIK_F10,
	DIK_NUMLOCK,     DIK_SCROLL,   DIK_NUMPAD7,      DIK_NUMPAD8,
	DIK_NUMPAD9,     DIK_SUBTRACT, DIK_NUMPAD4,      DIK_NUMPAD5,
	DIK_NUMPAD6,     DIK_ADD,      DIK_NUMPAD1,      DIK_NUMPAD2,
	DIK_NUMPAD3,     DIK_NUMPAD0,  DIK_DECIMAL,      DIK_F11,
	DIK_F12,         DIK_F13,      DIK_F14,          DIK_F15,
	DIK_KANA,        DIK_YEN,      DIK_NUMPADEQUALS, DIK_STOP,
	DIK_NUMPADENTER, DIK_RCONTROL, DIK_NUMPADCOMMA,  DIK_DIVIDE,
	DIK_SYSRQ,       DIK_RMENU,    DIK_HOME,         DIK_UP,
	DIK_PRIOR,       DIK_LEFT,     DIK_RIGHT,        DIK_END,
	DIK_DOWN,        DIK_NEXT,     DIK_INSERT,       DIK_DELETE,
	DIK_LWIN,        DIK_RWIN,     DIK_PAUSE
};

//----------------------------------------------------------------------------//
// Constructor.
DirectInputHandler::DirectInputHandler()
{
	LPDIRECTINPUT8 direct_input;
	HRESULT result = DirectInput8Create(GetModuleHandle(NULL),
	                                    DIRECTINPUT_VERSION,
	                                    IID_IDirectInput8,
	                                    (VOID**)&direct_input,
	                                    NULL);
	if (FAILED(result))
	{
		throw ut::Error(ut::error::fail, ut::Print(result) + " DirectInput8Create");
	}
	dinput = ut::ComPtr<IDirectInput8>(direct_input);

	InitDirectInputDevices();
}

//----------------------------------------------------------------------------->
// Updates input devices.
void DirectInputHandler::Update()
{
	// make sure that all possible directinput devices are ready
	InitDirectInputDevices();

	// references to existing input devices
	ut::Optional<Device&> keyboard;
	ut::Optional<Device&> mouse;

	// check what devices already exist
	const size_t device_count = cur_state.GetNum();
	for (size_t i = 0; i < device_count; i++)
	{
		Device& device = cur_state[i];
		const Device::Id id = device.GetTd();
		if (device.CompareId(&GUID_SysKeyboard, sizeof(GUID)))
		{
			keyboard = device;
		}
		else if (device.CompareId(&GUID_SysMouse, sizeof(GUID)))
		{
			mouse = device;
		}
	}

	// process devices
	ProcessKeyboard(keyboard);
	ProcessMouse(mouse);
}

//----------------------------------------------------------------------------->
// Initializes @di_keyboard.
HRESULT DirectInputHandler::InitKeyboardDevice()
{
	LPDIRECTINPUTDEVICE8 keyboard_device;
	HRESULT result = dinput->CreateDevice(GUID_SysKeyboard, &keyboard_device, NULL);
	if (FAILED(result))
	{
		return result;
	}

	const HWND hwnd = HWND_DESKTOP;
	result = keyboard_device->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		keyboard_device->Release();
		return result;
	}

	result = keyboard_device->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		keyboard_device->Release();
		return result;
	}

	di_keyboard = ut::ComPtr<IDirectInputDevice8>(keyboard_device);

	return S_OK;
}

//----------------------------------------------------------------------------->
// Initializes @di_mouse.
HRESULT DirectInputHandler::InitMouseDevice()
{
	LPDIRECTINPUTDEVICE8 mouse_device;
	HRESULT result = dinput->CreateDevice(GUID_SysMouse, &mouse_device, NULL);
	if (FAILED(result))
	{
		return result;
	}

	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIMouseState structure to IDirectInputDevice::GetDeviceState().
	DIOBJECTDATAFORMAT object_formats[] =
	{
		{ &GUID_XAxis, FIELD_OFFSET(DIMouseState, x), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 }, // X axis
		{ &GUID_YAxis, FIELD_OFFSET(DIMouseState, y), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 }, // Y axis
		{ 0, FIELD_OFFSET(DIMouseState, lmb), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 }, // Button 0
		{ 0, FIELD_OFFSET(DIMouseState, rmb), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 }, // Button 1 (optional)
		{ 0, FIELD_OFFSET(DIMouseState, mmb), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 }  // Button 2 (optional)
	};
	DWORD num_mouse_objects = sizeof(object_formats) / sizeof(DIOBJECTDATAFORMAT);

	DIDATAFORMAT mouse_data_format =
	{
		sizeof(DIDATAFORMAT),
		sizeof(DIOBJECTDATAFORMAT),
		DIDF_ABSAXIS,
		sizeof(DIMouseState),
		num_mouse_objects,
		object_formats
	};

	result = mouse_device->SetDataFormat(&mouse_data_format);
	if (FAILED(result))
	{
		return result;
	}

	// Set the cooperative level to let DInput know how this mouse device should
	// interact with the system and with other DInput applications.
	const HWND hwnd = HWND_DESKTOP;
	result = mouse_device->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	if (FAILED(result))
	{
		return result;
	}

	di_mouse = ut::ComPtr<IDirectInputDevice8>(mouse_device);

	return S_OK;
}

//----------------------------------------------------------------------------->
// Creates a IDirectInputDevice8 object for all input devices.
void DirectInputHandler::InitDirectInputDevices()
{
	if (!di_keyboard)
	{
		InitKeyboardDevice();
	}

	if (!di_mouse)
	{
		InitMouseDevice();
	}
}

//----------------------------------------------------------------------------->
// Creates a keyboard device.
ut::Result<Device, ut::Error> DirectInputHandler::CreateKeyboard()
{
	// create device object with unique identifier
	Device::Id keyboard_id(sizeof(GUID));
	ut::memory::Copy(keyboard_id.GetAddress(), &GUID_SysKeyboard, sizeof(GUID));
	Device keyboard_device(keyboard_id);

	// create keys
	for (size_t i = 0; i < keyboard::button_count; i++)
	{
		ut::Optional<ut::Error> error = keyboard_device.AddSignal(Signal::CreateDiscrete(0),
		                                                          keyboard::skKeyNames[i]);
		if (error)
		{
			return ut::MakeError(error.Move());
		}
	}

	return keyboard_device;
}

//----------------------------------------------------------------------------->
// Creates a mouse device.
ut::Result<Device, ut::Error> DirectInputHandler::CreateMouse()
{
	// create device object with unique identifier
	Device::Id mouse_id(sizeof(GUID));
	ut::memory::Copy(mouse_id.GetAddress(), &GUID_SysMouse, sizeof(GUID));
	Device mouse_device(mouse_id);

	// create buttons
	for (size_t i = 0; i < mouse::button_count; i++)
	{
		ut::Optional<ut::Error> error = mouse_device.AddSignal(Signal::CreateDiscrete(0),
		                                                       mouse::skButtonNames[i]);
		if (error)
		{
			throw ut::MakeError(error.Move());
		}
	}

	// XY movement
	for (size_t i = 0; i < 2; i++)
	{
		ut::Optional<ut::Error> error = mouse_device.AddSignal(Signal::CreateAnalog(0.0f),
		                                                       mouse::skMovementNames[i]);
		if (error)
		{
			throw ut::MakeError(error.Move());
		}
	}

	return mouse_device;
}

//----------------------------------------------------------------------------->
// Processes keyboard.
void DirectInputHandler::ProcessKeyboard(ut::Optional<Device&>& keyboard)
{
	if (di_keyboard)
	{
		if (keyboard)
		{
			UpdateKeyboard(keyboard.Get());
		}
		else
		{
			ut::Result<Device, ut::Error> keyboard_device = CreateKeyboard();
			if (keyboard_device)
			{
				UpdateKeyboard(keyboard_device.Get());
				cur_state.Add(keyboard_device.Move());
			}
			else
			{
				ut::log.Lock() << "[DirectInput] Failed to create keyboard device :"
				               << keyboard_device.GetAlt().GetDesc() << ut::cret;
			}
		}
	}
}

//----------------------------------------------------------------------------->
// Processes mouse.
void DirectInputHandler::ProcessMouse(ut::Optional<Device&>& mouse)
{
	if (di_mouse)
	{
		if (mouse)
		{
			UpdateMouse(mouse.Get());
		}
		else
		{
			ut::Result<Device, ut::Error> mouse_device = CreateMouse();
			if (mouse_device)
			{
				UpdateMouse(mouse_device.Get());
				cur_state.Add(mouse_device.Move());
			}
			else
			{
				ut::log.Lock() << "[DirectInput] Failed to create mouse device :"
				               << mouse_device.GetAlt().GetDesc() << ut::cret;
			}
		}
	}
}

//----------------------------------------------------------------------------->
// Extracts a keyboard state from @di_keyboard and stores in the
// provided device object.
void DirectInputHandler::UpdateKeyboard(Device& keyboard)
{
	// poll keyboard device to read the current state
	HRESULT result = di_keyboard->Poll();
	if (FAILED(result))
	{
		result = di_keyboard->Acquire();
		while (result == DIERR_INPUTLOST)
		{
			result = di_keyboard->Acquire();
		}
	}

	if (FAILED(result))
	{
		di_keyboard.Delete();
		return;
	}

	// get the input's keyboard state
	DIKeyboardState keyboard_state;
	result = di_keyboard->GetDeviceState(sizeof(DIKeyboardState), &keyboard_state);
	if (FAILED(result))
	{
		ut::log.Lock() << "[DirectInput] GetDeviceState: FAILED (" << ut::Print(result) << ")" << ut::cret;
		return;
	}

	// copy keyboard state data
	for (size_t i = 0; i < keyboard::button_count; i++)
	{
		const size_t signal_id = skDIKeyboardMap[i];
		keyboard.UpdateDiscreteSignal(i, (keyboard_state.keys[signal_id] & 0x80) ? 1 : 0);
	}
}

//----------------------------------------------------------------------------->
// Extracts a mouse state from @di_keyboard and stores in the
// provided device object.
void DirectInputHandler::UpdateMouse(Device& mouse)
{
	// poll mouse device to read the current state
	HRESULT result = di_mouse->Poll();
	if (FAILED(result))
	{
		result = di_mouse->Acquire();
		while (result == DIERR_INPUTLOST)
		{
			result = di_mouse->Acquire();
		}
	}

	if (FAILED(result))
	{
		di_mouse.Delete();
		prev_mouse_state = ut::Optional<DIMouseState>();
		return;
	}

	// Get the input's mouse state
	DIMouseState mouse_state;
	result = di_mouse->GetDeviceState(sizeof(DIMouseState), &mouse_state);
	if (FAILED(result))
	{
		ut::log.Lock() << "[DirectInput] GetDeviceState: FAILED (" << ut::Print(result) << ")" << ut::cret;
		return;
	}

	mouse.UpdateDiscreteSignal(mouse::button_lbutton, (mouse_state.lmb & 0x80) ? 1 : 0);
	mouse.UpdateDiscreteSignal(mouse::button_rbutton, (mouse_state.rmb & 0x80) ? 1 : 0);
	mouse.UpdateDiscreteSignal(mouse::button_mbutton, (mouse_state.mmb & 0x80) ? 1 : 0);

	if (prev_mouse_state)
	{
		const float offset_x = static_cast<float>(mouse_state.x - prev_mouse_state->x);
		const float offset_y = static_cast<float>(mouse_state.y - prev_mouse_state->y);
		mouse.UpdateAnalogSignal(mouse::button_count + mouse::movement_x, offset_x);
		mouse.UpdateAnalogSignal(mouse::button_count + mouse::movement_y, offset_y);
	}
	else
	{
		mouse.UpdateAnalogSignal(mouse::button_count + mouse::movement_x, 0.0f);
		mouse.UpdateAnalogSignal(mouse::button_count + mouse::movement_y, 0.0f);
	}

	prev_mouse_state = mouse_state;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // UT_WINDOWS
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//