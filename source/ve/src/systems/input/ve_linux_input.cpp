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
static const size_t skLinuxKeyboardMap[static_cast<size_t>(keyboard::Button::count)] =
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
    case KEY_ESC: return static_cast<ut::uint32>(keyboard::Button::escape);
    case KEY_1: return static_cast<ut::uint32>(keyboard::Button::one);
    case KEY_2: return static_cast<ut::uint32>(keyboard::Button::two);
    case KEY_3: return static_cast<ut::uint32>(keyboard::Button::three);
	case KEY_4: return static_cast<ut::uint32>(keyboard::Button::four);
	case KEY_5: return static_cast<ut::uint32>(keyboard::Button::five);
	case KEY_6: return static_cast<ut::uint32>(keyboard::Button::six);
	case KEY_7: return static_cast<ut::uint32>(keyboard::Button::seven);
	case KEY_8: return static_cast<ut::uint32>(keyboard::Button::eight);
	case KEY_9: return static_cast<ut::uint32>(keyboard::Button::nine);
	case KEY_0: return static_cast<ut::uint32>(keyboard::Button::zero);
	case KEY_MINUS: return static_cast<ut::uint32>(keyboard::Button::minus);
	case KEY_EQUAL: return static_cast<ut::uint32>(keyboard::Button::equals);
	case KEY_BACK: return static_cast<ut::uint32>(keyboard::Button::back);
	case KEY_TAB: return static_cast<ut::uint32>(keyboard::Button::tab);
	case KEY_Q: return static_cast<ut::uint32>(keyboard::Button::q);
	case KEY_W: return static_cast<ut::uint32>(keyboard::Button::w);
	case KEY_E: return static_cast<ut::uint32>(keyboard::Button::e);
	case KEY_R: return static_cast<ut::uint32>(keyboard::Button::r);
	case KEY_T: return static_cast<ut::uint32>(keyboard::Button::t);
	case KEY_Y: return static_cast<ut::uint32>(keyboard::Button::y);
	case KEY_U: return static_cast<ut::uint32>(keyboard::Button::u);
	case KEY_I: return static_cast<ut::uint32>(keyboard::Button::i);
	case KEY_O: return static_cast<ut::uint32>(keyboard::Button::o);
	case KEY_P: return static_cast<ut::uint32>(keyboard::Button::p);
	case KEY_LEFTBRACE: return static_cast<ut::uint32>(keyboard::Button::lbracket);
	case KEY_RIGHTBRACE: return static_cast<ut::uint32>(keyboard::Button::rbracket);
	case KEY_ENTER: return static_cast<ut::uint32>(keyboard::Button::enter);
	case KEY_LEFTCTRL: return static_cast<ut::uint32>(keyboard::Button::lcontrol);
	case KEY_A: return static_cast<ut::uint32>(keyboard::Button::a);
	case KEY_S: return static_cast<ut::uint32>(keyboard::Button::s);
	case KEY_D: return static_cast<ut::uint32>(keyboard::Button::d);
	case KEY_F: return static_cast<ut::uint32>(keyboard::Button::f);
	case KEY_G: return static_cast<ut::uint32>(keyboard::Button::g);
	case KEY_H: return static_cast<ut::uint32>(keyboard::Button::h);
	case KEY_J: return static_cast<ut::uint32>(keyboard::Button::j);
	case KEY_K: return static_cast<ut::uint32>(keyboard::Button::k);
	case KEY_L: return static_cast<ut::uint32>(keyboard::Button::l);
	case KEY_SEMICOLON: return static_cast<ut::uint32>(keyboard::Button::semicolon);
	case KEY_APOSTROPHE: return static_cast<ut::uint32>(keyboard::Button::apostrophe);
	case KEY_GRAVE: return static_cast<ut::uint32>(keyboard::Button::grave);
	case KEY_LEFTSHIFT: return static_cast<ut::uint32>(keyboard::Button::lshift);
	case KEY_BACKSLASH: return static_cast<ut::uint32>(keyboard::Button::backslash);
	case KEY_Z: return static_cast<ut::uint32>(keyboard::Button::z);
	case KEY_X: return static_cast<ut::uint32>(keyboard::Button::x);
	case KEY_C: return static_cast<ut::uint32>(keyboard::Button::c);
	case KEY_V: return static_cast<ut::uint32>(keyboard::Button::v);
	case KEY_B: return static_cast<ut::uint32>(keyboard::Button::b);
	case KEY_N: return static_cast<ut::uint32>(keyboard::Button::n);
	case KEY_M: return static_cast<ut::uint32>(keyboard::Button::m);
	case KEY_COMMA: return static_cast<ut::uint32>(keyboard::Button::comma);
	case KEY_DOT: return static_cast<ut::uint32>(keyboard::Button::dot);
	case KEY_SLASH: return static_cast<ut::uint32>(keyboard::Button::slash);
	case KEY_RIGHTSHIFT: return static_cast<ut::uint32>(keyboard::Button::rshift);
	case KEY_KPASTERISK: return static_cast<ut::uint32>(keyboard::Button::multiply);
	case KEY_LEFTALT: return static_cast<ut::uint32>(keyboard::Button::lalt);
	case KEY_SPACE: return static_cast<ut::uint32>(keyboard::Button::space);
	case KEY_CAPSLOCK: return static_cast<ut::uint32>(keyboard::Button::capslock);
	case KEY_F1: return static_cast<ut::uint32>(keyboard::Button::f1);
	case KEY_F2: return static_cast<ut::uint32>(keyboard::Button::f2);
	case KEY_F3: return static_cast<ut::uint32>(keyboard::Button::f3);
	case KEY_F4: return static_cast<ut::uint32>(keyboard::Button::f4);
	case KEY_F5: return static_cast<ut::uint32>(keyboard::Button::f5);
	case KEY_F6: return static_cast<ut::uint32>(keyboard::Button::f6);
	case KEY_F7: return static_cast<ut::uint32>(keyboard::Button::f7);
	case KEY_F8: return static_cast<ut::uint32>(keyboard::Button::f8);
	case KEY_F9: return static_cast<ut::uint32>(keyboard::Button::f9);
	case KEY_F10: return static_cast<ut::uint32>(keyboard::Button::f10);
	case KEY_NUMLOCK: return static_cast<ut::uint32>(keyboard::Button::numlock);
	case KEY_SCROLLLOCK: return static_cast<ut::uint32>(keyboard::Button::scroll);
	case KEY_KP7: return static_cast<ut::uint32>(keyboard::Button::numpad7);
	case KEY_KP8: return static_cast<ut::uint32>(keyboard::Button::numpad8);
	case KEY_KP9: return static_cast<ut::uint32>(keyboard::Button::numpad9);
	case KEY_KPMINUS: return static_cast<ut::uint32>(keyboard::Button::numpad_minus);
	case KEY_KP4: return static_cast<ut::uint32>(keyboard::Button::numpad4);
	case KEY_KP5: return static_cast<ut::uint32>(keyboard::Button::numpad5);
	case KEY_KP6: return static_cast<ut::uint32>(keyboard::Button::numpad6);
	case KEY_KPPLUS: return static_cast<ut::uint32>(keyboard::Button::numpad_plus);
	case KEY_KP1: return static_cast<ut::uint32>(keyboard::Button::numpad1);
	case KEY_KP2: return static_cast<ut::uint32>(keyboard::Button::numpad2);
	case KEY_KP3: return static_cast<ut::uint32>(keyboard::Button::numpad3);
	case KEY_KP0: return static_cast<ut::uint32>(keyboard::Button::numpad0);
	case KEY_KPDOT: return static_cast<ut::uint32>(keyboard::Button::numpad_dot);
	case KEY_F11: return static_cast<ut::uint32>(keyboard::Button::F11);
	case KEY_F12: return static_cast<ut::uint32>(keyboard::Button::F12);
	case KEY_F13: return static_cast<ut::uint32>(keyboard::Button::F13);
	case KEY_F14: return static_cast<ut::uint32>(keyboard::Button::F14);
	case KEY_F15: return static_cast<ut::uint32>(keyboard::Button::F15);
	case KEY_KATAKANA: return static_cast<ut::uint32>(keyboard::Button::kana);
	case KEY_YEN: return static_cast<ut::uint32>(keyboard::Button::yen);
	case KEY_KPEQUAL: return static_cast<ut::uint32>(keyboard::Button::numpad_equals);
	case KEY_STOP: return static_cast<ut::uint32>(keyboard::Button::stop);
	case KEY_KPENTER: return static_cast<ut::uint32>(keyboard::Button::numpad_enter);
	case KEY_RIGHTCTRL: return static_cast<ut::uint32>(keyboard::Button::rcontrol);
	case KEY_KPCOMMA: return static_cast<ut::uint32>(keyboard::Button::numpad_comma);
	case KEY_KPSLASH: return static_cast<ut::uint32>(keyboard::Button::numpad_slash);
	case KEY_SYSRQ: return static_cast<ut::uint32>(keyboard::Button::sysrq);
	case KEY_RIGHTALT: return static_cast<ut::uint32>(keyboard::Button::ralt);
	case KEY_HOME: return static_cast<ut::uint32>(keyboard::Button::home);
	case KEY_UP: return static_cast<ut::uint32>(keyboard::Button::up);
	case KEY_PREVIOUS: return static_cast<ut::uint32>(keyboard::Button::previous);
	case KEY_LEFT: return static_cast<ut::uint32>(keyboard::Button::left);
	case KEY_RIGHT: return static_cast<ut::uint32>(keyboard::Button::right);
	case KEY_END: return static_cast<ut::uint32>(keyboard::Button::end);
    case KEY_DOWN: return static_cast<ut::uint32>(keyboard::Button::down);
    case KEY_NEXT: return static_cast<ut::uint32>(keyboard::Button::next);
    case KEY_INSERT: return static_cast<ut::uint32>(keyboard::Button::insert);
    case KEY_DELETE: return static_cast<ut::uint32>(keyboard::Button::del);
    case KEY_CYCLEWINDOWS: return static_cast<ut::uint32>(keyboard::Button::lwin);
    case KEY_PAUSE: return static_cast<ut::uint32>(keyboard::Button::pause);
    }
    return ut::Optional<ut::uint32>();
}

//----------------------------------------------------------------------------//
// Returns corresponding ve::input::mouse::Button of the provided Linux key.
ut::Optional<ut::uint32> MapLinuxMouseKey(ut::uint32 key)
{
    switch(key)
    {
    case BTN_LEFT: return static_cast<ut::uint32>(mouse::Button::lbutton);
    case BTN_RIGHT: return static_cast<ut::uint32>(mouse::Button::rbutton);
    case BTN_MIDDLE: return static_cast<ut::uint32>(mouse::Button::mbutton);
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
    const size_t keyboard_count = keyboards.Count();
    for (size_t i = 0; i < keyboard_count; i++)
    {
        UpdateKeyboard(keyboards[i]);
    }

    const size_t mice_count = mice.Count();
    for (size_t i = 0; i < mice_count; i++)
    {
        UpdateMouse(mice[i]);
    }
}

//----------------------------------------------------------------------------->
// Reads keyboard events from file that have occurred since the last frame.
void LinuxInputHandler::UpdateKeyboard(DeviceFile& keyboard)
{
    if(keyboard.file < 0)
    {
        return;
    }

    // recover keyboard device if it was lost
    if(keyboard.file == 0)
    {
        keyboard.file = open(keyboard.location.GetAddress(), O_RDONLY | O_NONBLOCK);
        if (keyboard.file < 0)
        {
            keyboard.file = 0;
            return;
        }
        else
        {
            ut::log.Lock() << "Keyboard device has been restored. " << ut::cret;
        }
    }

    Device& device = cur_state[keyboard.device_index];

    // maximum number of keyboard events per frame
    constexpr size_t max_events = 64;

    // read events that have occurred since the last call
    struct input_event events[max_events];
    const int read_result = read(keyboard.file, events, sizeof(struct input_event) * max_events);
    if (read_result < static_cast<int>(sizeof(struct input_event)))
    {
        if (read_result < 0 && errno == ENODEV)
        {
            ut::log.Lock() << "Warning! Keyboard device lost. " << ut::cret;
            close(keyboard.file);
            keyboard.file = 0;
        }
        else if (read_result < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ut::log.Lock() << "Warning! Keyboard read result failed. Error code " << errno << ut::cret;
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
void LinuxInputHandler::UpdateMouse(DeviceFile& mouse)
{
    if(mouse.file < 0)
    {
        return;
    }

    // recover mouse device if it was lost
    if(mouse.file == 0)
    {
        mouse.file = open(mouse.location.GetAddress(), O_RDONLY | O_NONBLOCK);
        if (mouse.file < 0)
        {
            mouse.file = 0;
            return;
        }
        else
        {
            ut::log.Lock() << "Mouse device has been restored. " << ut::cret;
        }
    }

    Device& device = cur_state[mouse.device_index];

    // maximum number of mouse events per frame
    constexpr size_t max_events = 64;

    // read events that have occurred since the last call
    struct input_event events[max_events];
    int read_result = read(mouse.file, events, sizeof(struct input_event) * max_events);
    if (read_result < static_cast<int>(sizeof(struct input_event)))
    {
        if(read_result < 0 && errno == ENODEV)
        {
            ut::log.Lock() << "Warning! Mouse device lost. " << ut::cret;
            close(mouse.file);
            mouse.file = 0;
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
    device.UpdateAnalogSignal(static_cast<ut::uint32>(mouse::Button::count) + static_cast<ut::uint32>(mouse::Movement::x), rel_x);
    device.UpdateAnalogSignal(static_cast<ut::uint32>(mouse::Button::count) + static_cast<ut::uint32>(mouse::Movement::y), rel_y);
	device.UpdateAnalogSignal(static_cast<ut::uint32>(mouse::Button::count) + static_cast<ut::uint32>(mouse::Movement::wheel), rel_wheel);
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

    ut::Optional<ut::Error> create_error = CreateKeyboards(file_map.Get());
    if(create_error)
    {
        ut::log.Lock() << "Failed to initialize keyboard." << ut::cret;
    }

    create_error = CreateMice(file_map.Get());
    if(create_error)
    {
        ut::log.Lock() << "Failed to initialize mouse." << ut::cret;
    }
}

//----------------------------------------------------------------------------->
// Creates a keyboard interface using a file from the provided map.
ut::Optional<ut::Error> LinuxInputHandler::CreateKeyboards(const DeviceFileMap& device_map)
{
    // check if keyboard is present in the map
    if(device_map.keyboards.IsEmpty())
    {
        return ut::Error(ut::error::not_found);
    }

    const size_t keyboard_count = device_map.keyboards.Count();
    for (size_t i = 0; i < keyboard_count; i++)
    {
        // open input file
        const ut::String& file_path = device_map.keyboards[i];
        const int file = open(file_path.GetAddress(), O_RDONLY | O_NONBLOCK);
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
        ut::memory::Copy(keyboard_id.GetAddress(), phys_path.GetAddress(), id_length);
        Device keyboard_device(ut::Move(keyboard_id));

        // create keys
        for (size_t i = 0; i < static_cast<size_t>(keyboard::Button::count); i++)
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
        if (!cur_state.Add(ut::Move(keyboard_device)))
        {
            return ut::Error(ut::error::out_of_memory);
        }
        if (!keyboards.Add(DeviceFile(cur_state.Count() - 1, file, file_path)))
        {
            return ut::Error(ut::error::out_of_memory);
        }
	}

	// success
    return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Creates a mouse interface using a file from the provided map.
ut::Optional<ut::Error> LinuxInputHandler::CreateMice(const DeviceFileMap& device_map)
{
    // check if keyboard is present in the map
    if(!device_map.mice.Count())
    {
        return ut::Error(ut::error::not_found);
    }

    const size_t mice_count = device_map.mice.Count();
    for (size_t i = 0; i < mice_count; i++)
    {
        // open input file
        const ut::String& file_path = device_map.mice[i];
        const int file = open(file_path.GetAddress(), O_RDONLY | O_NONBLOCK);
        if (file < 0)
        {
            ut::log.Lock() << "Failed to open mouse input file: " << file_path << ut::cret;
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
        ut::memory::Copy(mouse_id.GetAddress(), phys_path.GetAddress(), id_length);
        Device mouse_device(ut::Move(mouse_id));

        // create buttons
        for (size_t i = 0; i < static_cast<size_t>(mouse::Button::count); i++)
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
        for (size_t i = 0; i < static_cast<size_t>(mouse::Movement::count); i++)
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
        if (!cur_state.Add(ut::Move(mouse_device)))
        {
            return ut::Error(ut::error::out_of_memory);
        }
        if (!mice.Add(DeviceFile(cur_state.Count() - 1, file, file_path)))
        {
            return ut::Error(ut::error::out_of_memory);
        }
	}

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
        int event_file = open(file_path.GetAddress(), O_RDONLY | O_NONBLOCK);
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
            ut::log.Lock() << "Keyboard source #" << out.keyboards.Count()
                           << " was set to: " << file_path << ut::cret;
            if (!out.keyboards.Add(file_path))
            {
                return ut::MakeError(ut::error::out_of_memory);
            }
        }
        else if (TestBit(evbits, EV_KEY) && TestBit(keybits, BTN_MOUSE))
        {
            ut::log.Lock() << "Mouse source #" << out.mice.Count()
                           << " was set to: " << file_path << ut::cret;
            if (!out.mice.Add(file_path))
            {
                return ut::MakeError(ut::error::out_of_memory);
            }
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
