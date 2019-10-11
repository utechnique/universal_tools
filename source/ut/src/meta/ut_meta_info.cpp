//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_info.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
namespace serialization_flags
{
	// If this bit is on, little-endian order is used for serialization.
	// Turn off this bit to use big-endian order.
	const Info::Flag kLittleEndian = 0x1;

	// If this bit is on, a special string containing a name of the type must
	// accompany all serialized entities.
	const Info::Flag kTypeInfo = 0x2;

	// If this bit is on, a special identifier must accompany all serialized
	// entities. Also top-most entity must have a special list of links.
	const Info::Flag kLinkageInfo = 0x4;

	// If this bit is on, all serialized entities must have a name.
	// You may turn this bit off to reduce final size of serialized binary
	// data. But then it would be impossible to check parameters by name,
	// also an order of parameters must be the same in serialized and current
	// version.
	const Info::Flag kBinaryNames = 0x8;

	// If this bit is on, a parameter value is encapsulated inside a separate
	// node. This is how naming issues (when child node can have the same name
	// as one of the attributes) can be avoided. However you can turn this bit
	// off to make text files look better and human-friendly. But keep in mind
	// that you can't name child node with any name from ut::meta::node_names
	// enumeration in this case.
	const Info::Flag kTextValueEncapsulation = 0x20;

	// Set of flags with maximum information about the serialized entity.
	const Info::Flag kComplete = kLittleEndian | kTypeInfo | kLinkageInfo |
	                             kBinaryNames | kTextValueEncapsulation;

	// Set of flags with minimum information about the serialized entity.
	// This is suitable only for very primitive structures (plain values,
	// arrays, etc.) without pointers.
	const Info::Flag kPure = kLittleEndian;
}

//----------------------------------------------------------------------------//
// Create ut::meta::Info with ut::meta::serialization_flags::kComplete flag.
//    @param version - version number, default is 1.
//    @return - new ut::meta::Info object.
Info Info::CreateComplete(Version version)
{
	return Info(version, serialization_flags::kComplete);
}

// Create ut::meta::Info with ut::meta::serialization_flags::kPure flag.
//    @param version - version number, default is 1.
//    @return - new ut::meta::Info object.
Info Info::CreatePure(Version version)
{
	return Info(version, serialization_flags::kPure);
}

//----------------------------------------------------------------------------//
// Constructor
//    @param in_version - version number.
//    @param in_flags - set of binary flags, see ut::meta::serialization_flags
//                      namespace for details.
Info::Info(Version in_version,
           Flag in_flags) : version(in_version)
                          , flags(in_flags)
{

}

//----------------------------------------------------------------------------->
// Getter for endianness order flag, see 
// ut::meta::serialization_flags::kLittleEndian for details.
//    @return - current endianness order.
endian::order Info::GetEndianness() const
{
	if (flags & serialization_flags::kLittleEndian)
	{
		return endian::little;
	}
	else
	{
		return endian::big;
	}
}

// Setter for endianness order flag, see 
// ut::meta::serialization_flags::kLittleEndian for details.
//    @param endianness - endianness to be set.
void Info::SetEndianness(endian::order endianness)
{
	if (endianness == endian::little)
	{
		flags |= serialization_flags::kLittleEndian;
	}
	else
	{
		flags &= ~serialization_flags::kLittleEndian;
	}
	VerifyFlags();
}

//----------------------------------------------------------------------------->
// Returns 'true' if type information flag is on.
// See ut::meta::serialization_flags::kTypeInfo for details.
bool Info::HasTypeInformation() const
{
	return (flags & serialization_flags::kTypeInfo) ? true : false;
}

// Sets type information flag.
// See ut::meta::serialization_flags::kTypeInfo for details.
//    @param status - boolean that turns on/off type information.
void Info::EnableTypeInformation(bool status)
{
	if (status)
	{
		flags |= serialization_flags::kTypeInfo;
	}
	else
	{
		flags &= ~serialization_flags::kTypeInfo;
	}
	VerifyFlags();
}

//----------------------------------------------------------------------------->
// Returns 'true' if linkage information flag is on.
// See ut::meta::serialization_flags::kLinkageInfo for details.
bool Info::HasLinkageInformation() const
{
	return (flags & serialization_flags::kLinkageInfo) ? true : false;
}

// Sets linkage information flag.
// See ut::meta::serialization_flags::kLinkageInfo for details.
//    @param status - boolean that turns on/off linkage information.
void Info::EnableLinkageInformation(bool status)
{
	if (status)
	{
		flags |= serialization_flags::kLinkageInfo;
	}
	else
	{
		flags &= ~serialization_flags::kLinkageInfo;
	}
	VerifyFlags();
}

//----------------------------------------------------------------------------->
// Returns 'true' if parameters preserve name in binary mode.
// This option exists because you may want to reduce binary data size.
// See ut::meta::serialization_flags::kBinaryNames for details.
bool Info::HasBinaryNames() const
{
	return (flags & serialization_flags::kBinaryNames) ? true : false;
}

// Turns on/off parameter names for binary serialization.
// See ut::meta::serialization_flags::kBinaryNames for details.
// This option exists because you may want to reduce binary data size.
//    @param status - boolean that turns on/off binary names.
void Info::EnableBinaryNames(bool status)
{
	if (status)
	{
		flags |= serialization_flags::kBinaryNames;
	}
	else
	{
		flags &= ~serialization_flags::kBinaryNames;
	}
	VerifyFlags();
}

//----------------------------------------------------------------------------->
// Returns 'true' if parameters must have a special separate node 
// encapsulating value and children in a text mode.
// This option exists because you may want to have good-looking
// human-friendly text files by disabling this option.
// See ut::meta::serialization_flags::kTextValueEncapsulation for details.
bool Info::HasValueEncapsulation() const
{
	return (flags & serialization_flags::kTextValueEncapsulation) ? true : false;
}

// Turns on/off value node encapsulation in a text mode.
// See ut::meta::serialization_flags::kBinaryNames for details.
// This option exists because you may want to have good-looking
// human-friendly text files by disabling this option.
// See ut::meta::serialization_flags::kTextValueEncapsulation for details.
//    @param status - boolean that turns on/off value encapsulation.
void Info::EnableValueEncapsulation(bool status)
{
	if (status)
	{
		flags |= serialization_flags::kTextValueEncapsulation;
	}
	else
	{
		flags &= ~serialization_flags::kTextValueEncapsulation;
	}
	VerifyFlags();
}

//----------------------------------------------------------------------------->
// Returns current set of binary flags.
Info::Flag Info::GetFlags() const
{
	return flags;
}

// Applies provided flags
//    @param in_flags - set of binary flags, see ut::meta::serialization_flags
//                      namespace for details. Previous flags are overwritten.
void Info::SetFlags(Flag in_flags)
{
	flags = in_flags;
}

//----------------------------------------------------------------------------->
// Returns a number of the current version.
Info::Version Info::GetVersion() const
{
	return version;
}

// Sets a number of the current version.
void Info::SetVersion(Version in_version)
{
	version = in_version;
}

//----------------------------------------------------------------------------->
// Connects slot to the log signal. This signal is triggered when
// some extraordinary serialization event occurs.
void Info::ConnectLogSignalSlot(const Function<void(const ut::String&)>& slot)
{
	log_signal.Connect(slot);
}

// Calls log slots with the provided message.
void Info::LogMessage(const String& msg)
{
	log_signal(msg);
}

//----------------------------------------------------------------------------->
// Checks if all flags are compatible, and if any conflict occurs - fixes it.
void Info::VerifyFlags()
{
	if (HasBinaryNames() || HasTypeInformation() || HasLinkageInformation())
	{
		flags |= serialization_flags::kTextValueEncapsulation;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//