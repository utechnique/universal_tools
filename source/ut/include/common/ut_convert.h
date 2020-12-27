//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_type_names.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Converts intrinsic types.
// List of supported types:
//   bool
//   int8
//   uint8
//   int16
//   uint16
//   int32
//   uint32
//   int64
//   uint64
//   float
//   double
//   long double
// Does nothing if destination or source type is not supported.
// Take into account that this function is slow and prefer explicit C++ style
// cast whenever it's possible.
//    @param src_addr - pointer to the object to be converted.
//    @param src_type - type Id of the source object.
//    @param dst_addr - destination address for the converted value.
//    @param dst_type - type Id of the destination object.
void Convert(const void* src_addr,
             TypeId src_type,
             void* dst_addr,
             TypeId dst_type);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//