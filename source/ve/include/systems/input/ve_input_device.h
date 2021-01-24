//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_input_signal.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// ve::input::Device is a virtual input device. It manages current state of the
// corresponding input signals.
class Device
{
public:
	// Id can have different size for different devices.
	typedef ut::Array<byte> Id;

	// Constructor.
	//    @param device_id - unique identifier of the device.
	Device(Id device_id);

	// Move constructor.
	Device(Device&&) noexcept;

	// Move assignment operator.
	Device& operator=(Device&&) noexcept;

	// Copy constructor.
	Device(const Device& copy);

	// Copy assignment operator.
	Device& operator=(const Device& copy);

	// Returns a reference to the device id.
	const Id& GetTd() const;

	// Returns a reference to the array of signals.
	const ut::Array<Signal>& GetSignals() const;

	// Searches for a signal by name.
	//    @param name - signal name.
	//    @return - const reference to the desired signal
	//              or nothing if not found.
	ut::Optional<const Signal&> FindSignal(const ut::String& name) const;

	// Compares device id.
	//    @param other_id - pointer to the device id to compare with.
	//    @param other_id_size - size of the id in bytes.
	//    @return - true is id matches, false otherwise.
	bool CompareId(const void* other_id, size_t other_id_size) const;

	// Compares device id.
	//    @param other_id - reference to the device id to compare with.
	//    @return - true is id matches, false otherwise.
	bool CompareId(const Id& other_id) const;

	// Adds a new signal to the device. This signal can't be deleted.
	//    @param signal - a signal to add.
	//    @param name - signal name.
	//    @return - optional error if failed.
	ut::Optional<ut::Error> AddSignal(Signal signal, ut::String name);

	// Updates a value of the discrete signal.
	//    @param signal_id - id of the signal in @signals array.
	//    @param value - value to update signal with.
	void UpdateDiscreteSignal(size_t signal_id, Signal::Discrete value);

	// Updates a value of the analog signal.
	//    @param signal_id - id of the signal in @signals array.
	//    @param value - value to update signal with.
	void UpdateAnalogSignal(size_t signal_id, Signal::Analog value);

private:
	// Initializes this device as a copy of the provided device object.
	// This function makes an attempt to avoid excessive memory allocations.
	// If this device and @copy shares the same id - only signal values are
	// copied, map is not affected. Otherwise this device must be initialized
	// from scratch.
	//    @param copy - const reference to the device.
	void Copy(const Device& copy);

	// Unique identifier of this device.
	Id id;

	// Array of signals owned by this device.
	ut::Array<Signal> signals;

	// Map to find signals by name.
	ut::AVLTree<ut::String, size_t> map;

	// The hash is needed to prevent excessive memory allocation on copying,
	// memory can be just copied if the hash is the same.
	ut::uint32 hash;
	ut::Array<ut::byte> hash_source;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//