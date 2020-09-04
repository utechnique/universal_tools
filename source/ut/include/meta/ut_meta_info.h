//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "system/ut_endianness.h"
#include "meta/ut_reflective.h"
#include "events/ut_signal.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Info is a class that is a guide for serialization and
// deserialization. It describes a format of the serialized entity.
class Info
{
public:
	typedef uint32 Version;
	typedef uint32 Flag;
	
	// Create ut::meta::Info with ut::meta::serialization_flags::kComplete flag.
	//    @param version - version number, default is 1.
	//    @return - new ut::meta::Info object.
	static Info CreateComplete(Version version = 1);

	// Create ut::meta::Info with ut::meta::serialization_flags::kMinimal flag.
	//    @param version - version number, default is 1.
	//    @return - new ut::meta::Info object.
	static Info CreateMinimal(Version version = 1);

	// Create ut::meta::Info with ut::meta::serialization_flags::kPure flag.
	//    @param version - version number, default is 1.
	//    @return - new ut::meta::Info object.
	static Info CreatePure(Version version = 1);

	// Constructor
	//    @param in_version - version number.
	//    @param in_flags - set of binary flags, see ut::meta::serialization_flags
	//                      namespace for details.
	Info(Version in_version, Flag in_flags);

	// Getter for endianness order flag, see 
	// ut::meta::serialization_flags::kLittleEndian for details.
	//    @return - current endianness order.
	endian::order GetEndianness() const;

	// Setter for endianness order flag, see 
	// ut::meta::serialization_flags::kLittleEndian for details.
	//    @param endianness - endianness to be set.
	void SetEndianness(endian::order endianness);

	// Returns 'true' if type information flag is on.
	// See ut::meta::serialization_flags::kTypeInfo for details.
	bool HasTypeInformation() const;

	// Sets type information flag.
	// See ut::meta::serialization_flags::kTypeInfo for details.
	//    @param status - boolean that turns on/off type information.
	void EnableTypeInformation(bool status);

	// Returns 'true' if linkage information flag is on.
	// See ut::meta::serialization_flags::kLinkageInfo for details.
	bool HasLinkageInformation() const;

	// Sets linkage information flag.
	// See ut::meta::serialization_flags::kLinkageInfo for details.
	//    @param status - boolean that turns on/off linkage information.
	void EnableLinkageInformation(bool status);

	// Returns 'true' if parameters preserve name in binary mode.
	// This option exists because you may want to reduce binary data size.
	// See ut::meta::serialization_flags::kBinaryNames for details.
	bool HasBinaryNames() const;

	// Turns on/off parameter names for binary serialization.
	// See ut::meta::serialization_flags::kBinaryNames for details.
	// This option exists because you may want to reduce binary data size.
	//    @param status - boolean that turns on/off binary names.
	void EnableBinaryNames(bool status);

	// Returns 'true' if size information flag is on.
	// See ut::meta::serialization_flags::kSizeInfo for details.
	bool HasSizeinformation() const;

	// Sets size information flag.
	// See ut::meta::serialization_flags::kSizeInfo for details.
	//    @param status - boolean that turns on/off size information.
	void EnableSizeInformation(bool status);

	// Returns 'true' if parameters must have a special separate node 
	// encapsulating value and children in a text mode.
	// This option exists because you may want to have good-looking
	// human-friendly text files by disabling this option.
	// See ut::meta::serialization_flags::kTextValueEncapsulation for details.
	bool HasValueEncapsulation() const;

	// Turns on/off value node encapsulation in a text mode.
	// See ut::meta::serialization_flags::kBinaryNames for details.
	// This option exists because you may want to have good-looking
	// human-friendly text files by disabling this option.
	// See ut::meta::serialization_flags::kTextValueEncapsulation for details.
	//    @param status - boolean that turns on/off value encapsulation.
	void EnableValueEncapsulation(bool status);

	// Returns current set of binary flags.
	Flag GetFlags() const;

	// Applies provided flags
	//    @param in_flags - set of binary flags, see ut::meta::serialization_flags
	//                      namespace for details. Previous flags are overwritten.
	void SetFlags(Flag in_flags);

	// Returns a number of the current version.
	Version GetVersion() const;

	// Sets a number of the current version.
	void SetVersion(Version in_version);

	// Connects slot to the log signal. This signal is triggered when
	// some extraordinary serialization event occurs.
	void ConnectLogSignalSlot(Function<void(const ut::String& event_description)> slot);
	
	// Calls log slots with the provided message.
	void LogMessage(const String& msg);

private:

	// Signal when some extraordinary serialization event occurs.
	ut::Signal<void(const ut::String&)> log_signal;

	// Checks if all flags are compatible, and if any conflict occurs - fixes it.
	void VerifyFlags();

	// Integer value containing a number of the current version.
	Version version;

	// Integer value containing a set of binary flags.
	// See ut::meta::serialization_flags namespace for details.
	Flag flags;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//