//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_def.h"
#include "ut_platform.h"
#include "ut_numeric_types.h"
#include "ut_char_traits.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Name of the unknown or custom (user-defined) type
template<typename T> inline const char* TypeName() { return "data"; }

// Specialization for the intrinsic types
template<> inline const char* TypeName<bool>()        { return "bool";    }
template<> inline const char* TypeName<int8>()        { return "int8";    }
template<> inline const char* TypeName<uint8>()       { return "byte";    }
template<> inline const char* TypeName<int16>()       { return "int16";   }
template<> inline const char* TypeName<uint16>()      { return "uint16";  }
template<> inline const char* TypeName<int32>()       { return "int32";   }
template<> inline const char* TypeName<uint32>()      { return "uint32";  }
template<> inline const char* TypeName<int64>()       { return "int64";   }
template<> inline const char* TypeName<uint64>()      { return "uint64";  }
template<> inline const char* TypeName<float>()       { return "float";   }
template<> inline const char* TypeName<double>()      { return "double";  }
template<> inline const char* TypeName<long double>() { return "ldouble"; }

// Returns true if provided type name is a name of the numeric type
template<typename T> inline bool IsNumericType(const T* type_name)
{
	if (StrCmp<T>(type_name, TypeName<int32>())       == 0 ||
		StrCmp<T>(type_name, TypeName<uint32>())      == 0 ||
		StrCmp<T>(type_name, TypeName<float>())       == 0 ||
		StrCmp<T>(type_name, TypeName<int16>())       == 0 ||
		StrCmp<T>(type_name, TypeName<uint16>())      == 0 ||
		StrCmp<T>(type_name, TypeName<int8>())        == 0 ||
		StrCmp<T>(type_name, TypeName<uint8>())       == 0 ||
		StrCmp<T>(type_name, TypeName<int64>())       == 0 ||
		StrCmp<T>(type_name, TypeName<uint64>())      == 0 ||
		StrCmp<T>(type_name, TypeName<double>())      == 0 ||
		StrCmp<T>(type_name, TypeName<long double>()) == 0)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//