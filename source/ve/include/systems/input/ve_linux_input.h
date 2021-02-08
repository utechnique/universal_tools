//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if UT_LINUX
//----------------------------------------------------------------------------//
#include "ve_input_handler.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Input handler for Linux OS. Uses event files to update state of the devices.
class LinuxInputHandler : public Handler
{
public:
	// Constructor.
	LinuxInputHandler();

	// Updates input devices.
	void Update() override;

private:
    // This is a proxy interface between ve::input::Device and
    // corresponding event file.
    struct DeviceFile
    {
        // Constructor.
        DeviceFile(ut::uint32 in_device_index,
                   int in_file,
                   ut::String in_loc);

        // Copying is prohibited.
        DeviceFile(const DeviceFile& ) = delete;
        DeviceFile& operator =(const DeviceFile&) = delete;

        // Move constructor.
        DeviceFile(DeviceFile&& other) noexcept;

        // Move operator.
        DeviceFile& operator =(DeviceFile&& other) noexcept;

        // Destructor.
        ~DeviceFile();

        ut::uint32 device_index;
        int file = 0;
        ut::String location;
    };

    // Encapsulates the information about input devices that are
    // present on this system. Every device contains a string with the
    // path to corresponding event file.
    struct DeviceFileMap
    {
        ut::Optional<ut::String> keyboard;
        ut::Optional<ut::String> mouse;
    };

    // Reads keyboard events from file that have occurred since the last frame.
    void UpdateKeyboard();

    // Reads mouse events from file that have occurred since the last frame.
    void UpdateMouse();

    // Initializes input devices such as keyboard, mouse, etc.
    void InitInputDevices();

    // Creates a keyboard interface using a file from the provided map.
    ut::Optional<ut::Error> CreateKeyboard(const DeviceFileMap& device_map);

    // Creates a mouse interface using a file from the provided map.
    ut::Optional<ut::Error> CreateMouse(const DeviceFileMap& device_map);

    // Collects info about the input devices present on the system and finds
    // appropriate input files for devices such as mouse, keyboard, etc.
    ut::Result<DeviceFileMap, ut::Error> GetFileMap();

    // Checks if a bit with desired index is set in the provided data.
    static bool TestBit(const ut::byte* data, ut::uint32 bit_index);

    // Devices.
    ut::Optional<DeviceFile> keyboard;
    ut::Optional<DeviceFile> mouse;

    // Directory with the input events files (event0, event1 ...)
    static const char* skEventDir;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // UT_LINUX
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
