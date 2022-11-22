//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "hash/ut_murmur3.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Hash is a template that is specialized for those types:
//    bool
//    int8 and uint8
//    int16 and uint16
//    int32 and uint32
//    int64 and uint64
//    float
//    double
//    long double
//    const char*
//    ut::String
// Every specialized version must have operator() defined accepting
// appropriate reference type and returning a size_t value.
template<typename KeyType>
struct Hash
{
	size_t operator()(const KeyType& value) const
	{
#if UT_PLATFORM_64BITS
		size_t out[2];
		MurmurHash3_x64_128(&value,
		                    static_cast<int>(sizeof(value)),
		                    1984,
		                    out);
		return out[0];
#else
		size_t out;
		MurmurHash3_x86_32(&value,
		                   static_cast<int>(sizeof(value)),
		                   1984,
		                   &out);
		return out;
#endif
	}
};

// bool
template<> struct Hash<bool>
{
	size_t operator()(const bool& value) const { return value ? 1 : 0; }
};

// int8
template<> struct Hash<int8>
{
	size_t operator()(const int8& value) const { return static_cast<size_t>(value); }
};

// uint8
template<> struct Hash<uint8>
{
	size_t operator()(const uint8& value) const { return static_cast<size_t>(value); }
};

// int16
template<> struct Hash<int16>
{
	size_t operator()(const int16& value) const { return static_cast<size_t>(value); }
};

// uint16
template<> struct Hash<uint16>
{
	size_t operator()(const uint16& value) const { return static_cast<size_t>(value); }
};

// int32
template<> struct Hash<int32>
{
	size_t operator()(const int32& value) const { return static_cast<size_t>(value); }
};

// uint32
template<> struct Hash<uint32>
{
	size_t operator()(const uint32& value) const { return static_cast<size_t>(value); }
};

// int64
template<> struct Hash<int64>
{
	size_t operator()(const int64& value) const { return static_cast<size_t>(value); }
};

// uint64
template<> struct Hash<uint64>
{
	size_t operator()(const uint64& value) const { return static_cast<size_t>(value); }
};

// float
template<> struct Hash<float>
{
	size_t operator()(const float& value) const
	{
		Hash<uint32> hash;
		return hash(*reinterpret_cast<const uint32*>(&value));
	}
};

// double
template<> struct Hash<double>
{
	size_t operator()(const double& value) const
	{
		Hash<uint64> hash;
		return hash(*reinterpret_cast<const uint64*>(&value));
	}
};

// long double
template<> struct Hash<long double>
{
	size_t operator()(const long double& value) const
	{
		Hash<uint64> hash;
		return hash(*reinterpret_cast<const uint64*>(&value));
	}
};

// null-terminated string
template<> struct Hash<char*>
{
	size_t operator()(const char* str) const
	{
		const size_t len = StrLen<char>(str);
#if UT_PLATFORM_64BITS
		size_t out[2];
		MurmurHash3_x64_128(str,
			static_cast<int>(len),
			1984,
			out);
		return out[0];
#else
		size_t out;
		MurmurHash3_x86_32(str,
		                   static_cast<int>(len),
		                   1984,
		                   &out);
		return out;
#endif
	}
};

// string
template<> struct Hash<String>
{
	size_t operator()(const String& str) const
	{
		const size_t len = str.Length();
#if UT_PLATFORM_64BITS
		size_t out[2];
		MurmurHash3_x64_128(str.GetAddress(),
		                    static_cast<int>(len),
		                    1984,
		                    out);
		return out[0];
#else
		size_t out;
		MurmurHash3_x86_32(str.GetAddress(),
		                   static_cast<int>(len),
		                   1984,
		                   &out);
		return out;
#endif
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
