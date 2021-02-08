//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#if UT_LINUX
//----------------------------------------------------------------------------//
#include "systems/input/ve_linux_input.h"
#include "systems/input/ve_input_keyboard.h"
#include "systems/input/ve_input_mouse.h"
//----------------------------------------------------------------------------//
#include <linux/input.h>
#include <linux/input-event-codes.h>
//----------------------------------------------------------------------------//
#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// This map is used to find corresponding Linux keyboard key.
static const size_t skLinuxKeyboardMap[keyboard::button_count] =
{
	KEY_ESC,          KEY_1,            KEY_2,          KEY_3,
	KEY_4,            KEY_5,            KEY_6,          KEY_7,
	KEY_8,            KEY_9,            KEY_0,          KEY_MINUS,
	KEY_EQUAL,        KEY_BACK,         KEY_TAB,        KEY_Q,
	KEY_W,            KEY_E,            KEY_R,          KEY_T,
	KEY_Y,            KEY_U,            KEY_I,          KEY_O,
	KEY_P,            KEY_LEFTBRACE,    KEY_RIGHTBRACE, KEY_ENTER,
	KEY_LEFTCTRL,     KEY_A,            KEY_S,          KEY_D,
	KEY_F,            KEY_G,            KEY_H,          KEY_J,
	KEY_K,            KEY_L,            KEY_SEMICOLON,  KEY_APOSTROPHE,
	KEY_GRAVE,        KEY_LEFTSHIFT,    KEY_BACKSLASH,  KEY_Z,
	KEY_X,            KEY_C,            KEY_V,          KEY_B,
	KEY_N,            KEY_M,            KEY_COMMA,      KEY_DOT,
	KEY_SLASH,        KEY_RIGHTSHIFT,   KEY_KPASTERISK, KEY_LEFTALT,
	KEY_SPACE,        KEY_CAPSLOCK,     KEY_F1,         KEY_F2,
	KEY_F3,           KEY_F4,           KEY_F5,         KEY_F6,
	KEY_F7,           KEY_F8,           KEY_F9,         KEY_F10,
	KEY_NUMLOCK,      KEY_SCROLLLOCK,   KEY_KP7,        KEY_KP8,
	KEY_KP9,          KEY_KPMINUS,      KEY_KP4,        KEY_KP5,
	KEY_KP6,          KEY_KPPLUS,       KEY_KP1,        KEY_KP2,
	KEY_KP3,          KEY_KP0,          KEY_KPDOT,      KEY_F11,
	KEY_F12,          KEY_F13,          KEY_F14,        KEY_F15,
	KEY_KATAKANA,     KEY_YEN,          KEY_KPEQUAL,    KEY_STOP,
	KEY_KPENTER,      KEY_RIGHTCTRL,    KEY_KPCOMMA,    KEY_KPSLASH,
	KEY_SYSRQ,        KEY_RIGHTALT,     KEY_HOME,       KEY_UP,
	KEY_PREVIOUS,     KEY_LEFT,         KEY_RIGHT,      KEY_END,
    KEY_DOWN,         KEY_NEXT,         KEY_INSERT,     KEY_DELETE,
    KEY_CYCLEWINDOWS, KEY_CYCLEWINDOWS, KEY_PAUSE
};

//----------------------------------------------------------------------------//
// Returns corresponding ve::input::keyboard::Button of the provided Linux key.
ut::Optional<ut::uint32> MapLinuxKeyboardKey(ut::uint32 key)
{
    switch(key)
    {
    case KEY_ESC: return keyboard::button_escape;
    case KEY_1: return keyboard::button_1;
    case KEY_2: return keyboard::button_2;
    case KEY_3: return keyboard::button_3;
	case KEY_4: return keyboard::button_4;
	case KEY_5: return keyboard::button_5;
	case KEY_6: return keyboard::button_6;
	case KEY_7: return keyboard::button_7;
	case KEY_8: return keyboard::button_8;
	case KEY_9: return keyboard::button_9;
	case KEY_0: return keyboard::button_0;
	case KEY_MINUS: return keyboard::button_minus;
	case KEY_EQUAL: return keyboard::button_equals;
	case KEY_BACK: return keyboard::button_back;
	case KEY_TAB: return keyboard::button_tab;
	case KEY_Q: return keyboard::button_q;
	case KEY_W: return keyboard::button_w;
	case KEY_E: return keyboard::button_e;
	case KEY_R: return keyboard::button_r;
	case KEY_T: return keyboard::button_t;
	case KEY_Y: return keyboard::button_y;
	case KEY_U: return keyboard::button_u;
	case KEY_I: return keyboard::button_i;
	case KEY_O: return keyboard::button_o;
	case KEY_P: return keyboard::button_p;
	case KEY_LEFTBRACE: return keyboard::button_lbracket;
	case KEY_RIGHTBRACE: return keyboard::button_rbracket;
	case KEY_ENTER: return keyboard::button_enter;
	case KEY_LEFTCTRL: return keyboard::button_lcontrol;
	case KEY_A: return keyboard::button_a;
	case KEY_S: return keyboard::button_s;
	case KEY_D: return keyboard::button_d;
	case KEY_F: return keyboard::button_f;
	case KEY_G: return keyboard::button_g;
	case KEY_H: return keyboard::button_h;
	case KEY_J: return keyboard::button_j;
	case KEY_K: return keyboard::button_k;
	case KEY_L: return keyboard::button_l;
	case KEY_SEMICOLON: return keyboard::button_semicolon;
	case KEY_APOSTROPHE: return keyboard::button_apostrophe;
	case KEY_GRAVE: return keyboard::button_grave;
	case KEY_LEFTSHIFT: return keyboard::button_lshift;
	case KEY_BACKSLASH: return keyboard::button_backslash;
	case KEY_Z: return keyboard::button_z;
	case KEY_X: return keyboard::button_x;
	case KEY_C: return keyboard::button_c;
	case KEY_V: return keyboard::button_v;
	case KEY_B: return keyboard::button_b;
	case KEY_N: return keyboard::button_n;
	case KEY_M: return keyboard::button_m;
	case KEY_COMMA: return keyboard::button_comma;
	case KEY_DOT: return keyboard::button_dot;
	case KEY_SLASH: return keyboard::button_slash;
	case KEY_RIGHTSHIFT: return keyboard::button_rshift;
	case KEY_KPASTERISK: return keyboard::button_multiply;
	case KEY_LEFTALT: return keyboard::button_lalt;
	case KEY_SPACE: return keyboard::button_space;
	case KEY_CAPSLOCK: return keyboard::button_capslock;
	case KEY_F1: return keyboard::button_f1;
	case KEY_F2: return keyboard::button_f2;
	case KEY_F3: return keyboard::button_f3;
	case KEY_F4: return keyboard::button_f4;
	case KEY_F5: return keyboard::button_f5;
	case KEY_F6: return keyboard::button_f6;
	case KEY_F7: return keyboard::button_f7;
	case KEY_F8: return keyboard::button_f8;
	case KEY_F9: return keyboard::button_f9;
	case KEY_F10: return keyboard::button_f10;
	case KEY_NUMLOCK: return keyboard::button_numlock;
	case KEY_SCROLLLOCK: return keyboard::button_scroll;
	case KEY_KP7: return keyboard::button_numpad7;
	case KEY_KP8: return keyboard::button_numpad8;
	case KEY_KP9: return keyboard::button_numpad9;
	case KEY_KPMINUS: return keyboard::button_numpad_minus;
	case KEY_KP4: return keyboard::button_numpad4;
	case KEY_KP5: return keyboard::button_numpad5;
	case KEY_KP6: return keyboard::button_numpad6;
	case KEY_KPPLUS: return keyboard::button_numpad_plus;
	case KEY_KP1: return keyboard::button_numpad1;
	case KEY_KP2: return keyboard::button_numpad2;
	case KEY_KP3: return keyboard::button_numpad3;
	case KEY_KP0: return keyboard::button_numpad0;
	case KEY_KPDOT: return keyboard::button_numpad_dot;
	case KEY_F11: return keyboard::button_F11;
	case KEY_F12: return keyboard::button_F12;
	case KEY_F13: return keyboard::button_F13;
	case KEY_F14: return keyboard::button_F14;
	case KEY_F15: return keyboard::button_F15;
	case KEY_KATAKANA: return keyboard::button_kana;
	case KEY_YEN: return keyboard::button_yen;
	case KEY_KPEQUAL: return keyboard::button_numpad_equals;
	case KEY_STOP: return keyboard::button_stop;
	case KEY_KPENTER: return keyboard::button_numpad_enter;
	case KEY_RIGHTCTRL: return keyboard::button_rcontrol;
	case KEY_KPCOMMA: return keyboard::button_numpad_comma;
	case KEY_KPSLASH: return keyboard::button_numpad_slash;
	case KEY_SYSRQ: return keyboard::button_sysrq;
	case KEY_RIGHTALT: return keyboard::button_ralt;
	case KEY_HOME: return keyboard::button_home;
	case KEY_UP: return keyboard::button_up;
	case KEY_PREVIOUS: return keyboard::button_previous;
	case KEY_LEFT: return keyboard::button_left;
	case KEY_RIGHT: return keyboard::button_right;
	case KEY_END: return keyboard::button_end;
    case KEY_DOWN: return keyboard::button_down;
    case KEY_NEXT: return keyboard::button_next;
    case KEY_INSERT: return keyboard::button_insert;
    case KEY_DELETE: return keyboard::button_delete;
    case KEY_CYCLEWINDOWS: return keyboard::button_lwin;
    case KEY_PAUSE: return keyboard::button_pause;
    }
    return ut::Optional<ut::uint32>();
}

//----------------------------------------------------------------------------//
// Returns corresponding ve::input::mouse::Button of the provided Linux key.
ut::Optional<ut::uint32> MapLinuxMouseKey(ut::uint32 key)
{
    switch(key)
    {
    case BTN_LEFT: return mouse::button_lbutton;
    case BTN_RIGHT: return mouse::button_rbutton;
    case BTN_MIDDLE: return mouse::button_mbutton;
    }
    return ut::Optional<ut::uint32>();
}

//----------------------------------------------------------------------------//
// Directory with the input events files (event0, event1 ...)
const char* LinuxInputHandler::skEventDir = "/dev/input/";

//----------------------------------------------------------------------------//
// Constructor.
LinuxInputHandler::DeviceFile::DeviceFile(ut::uint32 in_device_index,
                                          int in_file,
                                          ut::String in_loc) : device_index(in_device_index)
                                                             , file(in_file)
                                                             , location(ut::Move(in_loc))
{}

// Move constructor.
LinuxInputHandler::DeviceFile::DeviceFile(DeviceFile&& other) noexcept : device_index(other.device_index)
                                                                       , file(other.file)
                                                                       , location(ut::Move(other.location))
{
    other.file = 0;
}

// Move operator.
LinuxInputHandler::DeviceFile& LinuxInputHandler::DeviceFile::operator =(DeviceFile&& other) noexcept
{
    device_index = other.device_index;
    file = other.file;
    location = ut::Move(other.location);
    other.file = 0;
}

// Destructor.
LinuxInputHandler::DeviceFile::~DeviceFile()
{
    if(file > 0)
    {
        close(file);
    }
}

//----------------------------------------------------------------------------//
// Constructor.
LinuxInputHandler::LinuxInputHandler()
{
    InitInputDevices();
}

//----------------------------------------------------------------------------->
// Updates input devices.
void LinuxInputHandler::Update()
{
    UpdateKeyboard();
    UpdateMouse();
}

//----------------------------------------------------------------------------->
// Reads keyboard events from file that have occurred since the last frame.
void LinuxInputHandler::UpdateKeyboard()
{
    if(!keyboard || keyboard->file < 0)
    {
        return;
    }

    // recover keyboard device if it was lost
    if(keyboard->file == 0)
    {
        keyboard->file = open(keyboard->location.ToCStr(), O_RDONLY | O_NONBLOCK);
        if (keyboard->file < 0)
        {
            keyboard->file = 0;
            return;
        }
        else
        {
            ut::log.Lock() << "Keyboard device has been restored. " << ut::cret;
        }
    }

    Device& device = cur_state[keyboard->device_index];

    // maximum number of keyboard events per frame
    constexpr size_t max_events = 64;

    // read events that have occurred since the last call
    struct input_event events[max_events];
    const int read_result = read(keyboard->file, events, sizeof(struct input_event) * max_events);
    if (read_result < static_cast<int>(sizeof(struct input_event)))
    {
        if(read_result < 0 && errno == ENODEV)
        {
            ut::log.Lock() << "Warning! Keyboard device lost. " << ut::cret;
            close(keyboard->file);
            keyboard->file = 0;
        }
        return;
    }

    // iterate events
    for (ut::uint32 i = 0; i < read_result / sizeof(struct input_event); i++)
    {
        const struct input_event& event = events[i];
        const ut::Optional<ut::uint32> signal_id = MapLinuxKeyboardKey(event.code);

        // skip non-key events
        if (event.type != EV_KEY || !signal_id)
        {
            continue;
        }

        // set signal value
        switch(event.value)
        {
            // key up
            case 0: device.UpdateDiscreteSignal(signal_id.Get(), 0); break;

            // key down
            case 1: device.UpdateDiscreteSignal(signal_id.Get(), 1); break;

            // auto repeat
            case 2: device.UpdateDiscreteSignal(signal_id.Get(), 1); break;
        }
    }
}

//----------------------------------------------------------------------------->
// Reads mouse events from file that have occurred since the last frame.
void LinuxInputHandler::UpdateMouse()
{
    if(!mouse || mouse->file < 0)
    {
        return;
    }

    // recover mouse device if it was lost
    if(mouse->file == 0)
    {
        mouse->file = open(mouse->location.ToCStr(), O_RDONLY | O_NONBLOCK);
        if (mouse->file < 0)
        {
            mouse->file = 0;
            return;
        }
        else
        {
            ut::log.Lock() << "Mouse device has been restored. " << ut::cret;
        }
    }

    Device& device = cur_state[mouse->device_index];

    // maximum number of mouse events per frame
    constexpr size_t max_events = 64;

    // read events that have occurred since the last call
    struct input_event events[max_events];
    int read_result = read(mouse->file, events, sizeof(struct input_event) * max_events);
    if (read_result < static_cast<int>(sizeof(struct input_event)))
    {
        if(read_result < 0 && errno == ENODEV)
        {
            ut::log.Lock() << "Warning! Mouse device lost. " << ut::cret;
            close(mouse->file);
            mouse->file = 0;
            return;
        }
        read_result = 0;
    }

    // relative mouse and wheel offset since the previous frame
    float rel_x = 0.0f;
    float rel_y = 0.0f;
	float rel_wheel = 0.0f;

    // iterate events
    for (ut::uint32 i = 0; i < read_result / sizeof(struct input_event); i++)
    {
        struct input_event& event = events[i];
        const ut::Optional<ut::uint32> signal_id = MapLinuxMouseKey(event.code);

        if (event.type == EV_KEY && signal_id)
        {
            // set button signal value
            switch(event.value)
            {
                // key up
                case 0: device.UpdateDiscreteSignal(signal_id.Get(), 0); break;

                // key down
                case 1: device.UpdateDiscreteSignal(signal_id.Get(), 1); break;

                // auto repeat
                case 2: device.UpdateDiscreteSignal(signal_id.Get(), 1); break;
            }
        }
        else if (event.type == EV_REL)
        {
            // accumulate relative offset
            switch(event.code)
            {
                case REL_X: rel_x += static_cast<float>(event.value); break;
                case REL_Y: rel_y += static_cast<float>(event.value); break;
                case REL_WHEEL: rel_wheel += static_cast<float>(event.value); break;
            }
        }
    }

    // update relative offset
    device.UpdateAnalogSignal(mouse::button_count + mouse::movement_x, rel_x);
    device.UpdateAnalogSignal(mouse::button_count + mouse::movement_y, rel_y);
	device.UpdateAnalogSignal(mouse::button_count + mouse::movement_wheel, rel_wheel);
}

//----------------------------------------------------------------------------->
// Initializes input devices such as keyboard, mouse, etc.
void LinuxInputHandler::InitInputDevices()
{
    ut::Result<DeviceFileMap, ut::Error> file_map = GetFileMap();
    if(!file_map)
    {
        ut::log.Lock() << file_map.GetAlt().GetDesc() << ut::cret;
        return;
    }

    ut::Optional<ut::Error> create_error = CreateKeyboard(file_map.Get());
    if(create_error)
    {
        ut::log.Lock() << "Failed to initialize keyboard." << ut::cret;
    }

    create_error = CreateMouse(file_map.Get());
    if(create_error)
    {
        ut::log.Lock() << "Failed to initialize mouse." << ut::cret;
    }
}

//----------------------------------------------------------------------------->
// Creates a keyboard interface using a file from the provided map.
ut::Optional<ut::Error> LinuxInputHandler::CreateKeyboard(const DeviceFileMap& device_map)
{
    // check if keyboard is present in the map
    if(!device_map.keyboard)
    {
        return ut::Error(ut::error::not_found);
    }

    // open input file
    const ut::String& file_path = device_map.keyboard.Get();
    const int file = open(file_path, O_RDONLY | O_NONBLOCK);
    if (file < 0)
    {
        ut::log.Lock() << "Failed to open keyboard input file: " << file_path << ut::cret;
        return ut::Error(ut::ConvertErrno(errno));
    }

    // read physical path (it will be used as an id)
    ut::byte physbits[1024];
    if(ioctl(file, EVIOCGPHYS(sizeof(physbits)), physbits) < 0)
    {
        close(file);
        return ut::Error(ut::ConvertErrno(errno));
    }

    // create device object with unique identifier
    const ut::String phys_path(reinterpret_cast<const char*>(physbits));
    const size_t id_length = phys_path.Length();
	Device::Id keyboard_id(id_length);
	ut::memory::Copy(keyboard_id.GetAddress(), phys_path.ToCStr(), id_length);
	Device keyboard_device(ut::Move(keyboard_id));

	// create keys
	for (size_t i = 0; i < keyboard::button_count; i++)
	{
		ut::Optional<ut::Error> error = keyboard_device.AddSignal(Signal::CreateDiscrete(0),
		                                                          keyboard::skKeyNames[i]);
		if (error)
		{
		    close(file);
			return error.Move();
		}
	}

    // add device
	cur_state.Add(ut::Move(keyboard_device));
	keyboard = DeviceFile(cur_state.GetNum() - 1, file, file_path);

	// success
    return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Creates a mouse interface using a file from the provided map.
ut::Optional<ut::Error> LinuxInputHandler::CreateMouse(const DeviceFileMap& device_map)
{
    // check if keyboard is present in the map
    if(!device_map.mouse)
    {
        return ut::Error(ut::error::not_found);
    }

    // open input file
    const ut::String& file_path = device_map.mouse.Get();
    const int file = open(file_path, O_RDONLY | O_NONBLOCK);
    if (file < 0)
    {
        ut::log.Lock() << "Failed to open keyboard input file: " << file_path << ut::cret;
        return ut::Error(ut::ConvertErrno(errno));
    }

    // read physical path (it will be used as an id)
    ut::byte physbits[1024];
    if(ioctl(file, EVIOCGPHYS(sizeof(physbits)), physbits) < 0)
    {
        close(file);
        return ut::Error(ut::ConvertErrno(errno));
    }

    // create device object with unique identifier
    const ut::String phys_path(reinterpret_cast<const char*>(physbits));
    const size_t id_length = phys_path.Length();
	Device::Id mouse_id(id_length);
	ut::memory::Copy(mouse_id.GetAddress(), phys_path.ToCStr(), id_length);
	Device mouse_device(ut::Move(mouse_id));

	// create buttons
	for (size_t i = 0; i < mouse::button_count; i++)
	{
		ut::Optional<ut::Error> error = mouse_device.AddSignal(Signal::CreateDiscrete(0),
		                                                       mouse::skButtonNames[i]);
		if (error)
		{
		    close(file);
			return error.Move();
		}
	}

	// XY movement and wheel
	for (size_t i = 0; i < mouse::movement_count; i++)
	{
		ut::Optional<ut::Error> error = mouse_device.AddSignal(Signal::CreateAnalog(0.0f),
		                                                       mouse::skMovementNames[i]);
		if (error)
		{
		    close(file);
			return error.Move();
		}
	}

	// add device
	cur_state.Add(ut::Move(mouse_device));
	mouse = DeviceFile(cur_state.GetNum() - 1, file, file_path);

	// success
    return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Collects info about the input devices present on the system and finds
// appropriate input files for devices such as mouse, keyboard, etc.
ut::Result<LinuxInputHandler::DeviceFileMap, ut::Error> LinuxInputHandler::GetFileMap()
{
    DeviceFileMap out;

    DIR* dp = opendir(skEventDir);
    struct dirent* dirp;
    if(dp == nullptr)
    {
        ut::log.Lock() << "Failed to open " << skEventDir << ut::cret;
        return ut::MakeError(ut::ConvertErrno(errno));
    }

    while ((dirp = readdir(dp)) != nullptr)
    {
        if(ut::StrStr(dirp->d_name, "event") == nullptr)
        {
            continue;
        }

        ut::String file_path(skEventDir);
        file_path += dirp->d_name;
        int event_file = open(file_path.ToCStr(), O_RDONLY | O_NONBLOCK);
        if (event_file < 0)
        {
            continue;
        }

        ut::byte evbits[EV_MAX/8+1];
        ut::byte keybits[KEY_MAX/8+1];

        memset(evbits, 0, sizeof(evbits));
        memset(keybits, 0, sizeof(keybits));

        ioctl(event_file, EVIOCGBIT(0, sizeof(evbits)), &evbits);
        ioctl(event_file, EVIOCGBIT(EV_KEY, sizeof(keybits)), &keybits);

        close(event_file);

        if (TestBit(evbits, EV_KEY) && TestBit(keybits, KEY_0))
        {
            out.keyboard = file_path;
        }
        else if (TestBit(evbits, EV_KEY) && TestBit(keybits, BTN_MOUSE))
        {
            out.mouse = file_path;
        }
    }

    closedir(dp);

    return out;
}

//----------------------------------------------------------------------------->
// Checks if a bit with desired index is set in the provided data.
bool LinuxInputHandler::TestBit(const ut::byte* data, ut::uint32 bit_index)
{
    return data[bit_index / 8] & (1 << (bit_index % 8)) != 0;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // UT_LINUX
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
