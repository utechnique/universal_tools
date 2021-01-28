//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/input/ve_input_device.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Constructor.
//    @param device_id - unique identifier of the device.
Device::Device(Device::Id device_id) : id(ut::Move(device_id))
{}

// Move constructor.
Device::Device(Device&&) noexcept = default;

// Move assignment operator.
Device& Device::operator=(Device&&) noexcept = default;

// Copy constructor.
Device::Device(const Device& copy)
{
	Copy(copy);
}

// Copy assignment operator.
Device& Device::operator=(const Device& copy)
{
	Copy(copy);
	return *this;
}

// Returns a reference to the device id.
const Device::Id& Device::GetTd() const
{
	return id;
}

// Returns a reference to the array of signals.
const ut::Array<Signal>&  Device::GetSignals() const
{
	return signals;
}

// Searches for a signal by name.
//    @param name - signal name.
//    @return - const reference to the desired signal
//              or nothing if not found.
ut::Optional<const Signal&> Device::FindSignal(const ut::String& name) const
{
	ut::Optional<const size_t&> id = map.Find(name);
	if (id)
	{
		return signals[id.Get()];
	}
	return ut::Optional<const Signal&>();
}

// Compares device id.
//    @param other_id - pointer to the device id to compare with.
//    @param other_id_size - size of the id in bytes.
//    @return - true is id matches, false otherwise.
bool Device::CompareId(const void* other_id, size_t other_id_size) const
{
	const size_t id_size = id.GetSize();
	if (id_size != other_id_size)
	{
		return false;
	}

	for (size_t i = 0; i < id_size; i++)
	{
		const ut::byte* b = static_cast<const ut::byte*>(other_id) + i;
		if (id[i] != *b)
		{
			return false;
		}
	}

	return true;
}

// Compares device id.
//    @param other_id - reference to the device id to compare with.
//    @return - true is id matches, false otherwise.
bool Device::CompareId(const Device::Id& other_id) const
{
	return CompareId(other_id.GetAddress(), other_id.GetSize());
}

// Adds a new signal to the device. This signal can't be deleted.
//    @param signal - a signal to add.
//    @param name - signal name.
//    @return - optional error if failed.
ut::Optional<ut::Error> Device::AddSignal(Signal signal, ut::String name)
{
	if (!signals.Add(ut::Move(signal)))
	{
		return ut::Error(ut::error::out_of_memory);
	}

	if (!map.Insert(name, signals.GetNum() - 1))
	{
		return ut::Error(ut::error::already_exists);
	}

	const size_t name_length = name.Length();
	for (size_t i = 0; i < name_length; i++)
	{
		hash_source.Add(name[i]);
	}
	hash = ut::Adler32Sum(hash_source.GetAddress(), hash_source.GetSize());

	return ut::Optional<ut::Error>();
}

// Updates a value of the discrete signal.
//    @param signal_id - id of the signal in @signals array.
//    @param value - value to update signal with.
void Device::UpdateDiscreteSignal(size_t signal_id, Signal::Discrete value)
{
	if (signal_id < signals.GetNum())
	{
		signals[signal_id].signal.discrete = value;
	}
}

// Updates a value of the analog signal.
//    @param signal_id - id of the signal in @signals array.
//    @param value - value to update signal with.
void Device::UpdateAnalogSignal(size_t signal_id, Signal::Analog value)
{
	if (signal_id < signals.GetNum())
	{
		signals[signal_id].signal.analog = value;
	}
}

// Initializes this device as a copy of the provided device object.
// This function makes an attempt to avoid excessive memory allocations.
// If this device and @copy shares the same id - only signal values are
// copied, map is not affected. Otherwise this device must be initialized
// from scratch.
//    @param copy - const reference to the device.
void Device::Copy(const Device& copy)
{
	// full copying if hashes don't match
	const size_t signal_count = signals.GetNum();
	if (!CompareId(copy.id) ||
		hash != copy.hash ||
		signal_count != copy.signals.GetNum())
	{
		id = copy.id;
		signals = copy.signals;
		map = copy.map;
		hash = copy.hash;
		hash_source = copy.hash_source;
		return;
	}

	// partial copying otherwise
	for (size_t i = 0; i < signal_count; i++)
	{
		signals[i].signal = copy.signals[i].signal;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
