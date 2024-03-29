//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "preprocessor/ut_def.h"
#include "platforms/ut_platform.h"
#include "types/ut_numeric_types.h"
#include "text/ut_char_traits.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Unique type identifier.
typedef uptr TypeId;

// Name of the unknown or custom (user-defined) type
template <typename T> struct Type
{
	// Name() function must be declared for all ut::Type specializations.
	static inline const char* Name() { return "data"; }

	// Id() function is guaranteed to be available only for intrinsic types.
	static inline TypeId Id() { return reinterpret_cast<TypeId>(&intrinsic_id); }

private:
	// this id is available only for intrinsic types.
	static byte intrinsic_id;
};
template<typename T> byte Type<T>::intrinsic_id = 0;

// Static array
template <typename T, size_t size> struct Type<T[size]>
{
	static inline const char* Name() { return "static_array"; }
};

// Pointer types
template <typename T> struct Type<T*>
{
	static inline const char* Name() { return "raw_ptr"; }
};

// Const types have the same name and id
template <typename T> struct Type<const T>
{
	static inline const char* Name() { return Type<T>::Name(); }
	static inline TypeId Id() { return Type<T>::Id(); }
};

// Specialization for the intrinsic types
template<> inline const char* Type<bool>::Name()        { return "bool";    }
template<> inline const char* Type<int8>::Name()        { return "int8";    }
template<> inline const char* Type<uint8>::Name()       { return "byte";    }
template<> inline const char* Type<int16>::Name()       { return "int16";   }
template<> inline const char* Type<uint16>::Name()      { return "uint16";  }
template<> inline const char* Type<int32>::Name()       { return "int32";   }
template<> inline const char* Type<uint32>::Name()      { return "uint32";  }
template<> inline const char* Type<int64>::Name()       { return "int64";   }
template<> inline const char* Type<uint64>::Name()      { return "uint64";  }
template<> inline const char* Type<float>::Name()       { return "float";   }
template<> inline const char* Type<double>::Name()      { return "double";  }
template<> inline const char* Type<long double>::Name() { return "ldouble"; }
template<> inline const char* Type<void>::Name()        { return "void"; }

// Returns true if the provided type name is a name of the numeric type
template<typename T> inline bool IsNumericType(const T* type_name)
{
	if (StrCmp<T>(type_name, Type<int32>::Name())  ||
		StrCmp<T>(type_name, Type<uint32>::Name()) ||
		StrCmp<T>(type_name, Type<float>::Name())  ||
		StrCmp<T>(type_name, Type<int16>::Name())  ||
		StrCmp<T>(type_name, Type<uint16>::Name()) ||
		StrCmp<T>(type_name, Type<int8>::Name())   ||
		StrCmp<T>(type_name, Type<uint8>::Name())  ||
		StrCmp<T>(type_name, Type<int64>::Name())  ||
		StrCmp<T>(type_name, Type<uint64>::Name()) ||
		StrCmp<T>(type_name, Type<double>::Name()) ||
		StrCmp<T>(type_name, Type<long double>::Name()))
	{
		return true;
	}
	return false;
}

// Returns true if provided type is a numeric type
template<typename T> inline bool IsNumericType() { return false; }
template<> inline bool IsNumericType<int32>() { return true; }
template<> inline bool IsNumericType<uint32>() { return true; }
template<> inline bool IsNumericType<float>() { return true; }
template<> inline bool IsNumericType<int16>() { return true; }
template<> inline bool IsNumericType<uint16>() { return true; }
template<> inline bool IsNumericType<int8>() { return true; }
template<> inline bool IsNumericType<uint8>() { return true; }
template<> inline bool IsNumericType<int64>() { return true; }
template<> inline bool IsNumericType<uint64>() { return true; }
template<> inline bool IsNumericType<double>() { return true; }
template<> inline bool IsNumericType<long double>() { return true; }

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//