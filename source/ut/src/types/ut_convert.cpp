//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
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
void Convert(const void* src_addr, TypeId src_type, void* dst_addr, TypeId dst_type)
{
	union IntrinsicType
	{
		bool m_bool;
		int8 m_int8;
		uint8 m_uint8;
		int16 m_int16;
		uint16 m_uint16;
		int32 m_int32;
		uint32 m_uint32;
		int64 m_int64;
		uint64 m_uint64;
		float m_float;
		double m_double;
		long double m_ldouble;
	};

	const IntrinsicType* src = static_cast<const IntrinsicType*>(src_addr);
	IntrinsicType* dst = static_cast<IntrinsicType*>(dst_addr);

	if (dst_type == Type<bool>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_bool = src->m_bool;
		else if (src_type == Type<int8>::Id())        dst->m_bool = src->m_int8 == 0 ? false : true;
		else if (src_type == Type<uint8>::Id())       dst->m_bool = src->m_uint8 == 0 ? false : true;
		else if (src_type == Type<int16>::Id())       dst->m_bool = src->m_int16 == 0 ? false : true;
		else if (src_type == Type<uint16>::Id())      dst->m_bool = src->m_uint16 == 0 ? false : true;
		else if (src_type == Type<int32>::Id())       dst->m_bool = src->m_int32 == 0 ? false : true;
		else if (src_type == Type<uint32>::Id())      dst->m_bool = src->m_uint32 == 0 ? false : true;
		else if (src_type == Type<int64>::Id())       dst->m_bool = src->m_int64 == 0 ? false : true;
		else if (src_type == Type<uint64>::Id())      dst->m_bool = src->m_uint64 == 0 ? false : true;
		else if (src_type == Type<float>::Id())       dst->m_bool = src->m_float == 0.0f ? false : true;
		else if (src_type == Type<double>::Id())      dst->m_bool = src->m_double == 0.0 ? false : true;
		else if (src_type == Type<long double>::Id()) dst->m_bool = src->m_ldouble == 0.0 ? false : true;
	}
	else if (dst_type == Type<int8>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_int8 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_int8 = static_cast<int8>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_int8 = static_cast<int8>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_int8 = static_cast<int8>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_int8 = static_cast<int8>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_int8 = static_cast<int8>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_int8 = static_cast<int8>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_int8 = static_cast<int8>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_int8 = static_cast<int8>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_int8 = static_cast<int8>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_int8 = static_cast<int8>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_int8 = static_cast<int8>(src->m_ldouble);
	}
	else if (dst_type == Type<uint8>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_uint8 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_uint8 = static_cast<uint8>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_uint8 = static_cast<uint8>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_uint8 = static_cast<uint8>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_uint8 = static_cast<uint8>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_uint8 = static_cast<uint8>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_uint8 = static_cast<uint8>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_uint8 = static_cast<uint8>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_uint8 = static_cast<uint8>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_uint8 = static_cast<uint8>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_uint8 = static_cast<uint8>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_uint8 = static_cast<uint8>(src->m_ldouble);
	}
	else if (dst_type == Type<int16>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_int16 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_int16 = static_cast<int16>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_int16 = static_cast<int16>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_int16 = static_cast<int16>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_int16 = static_cast<int16>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_int16 = static_cast<int16>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_int16 = static_cast<int16>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_int16 = static_cast<int16>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_int16 = static_cast<int16>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_int16 = static_cast<int16>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_int16 = static_cast<int16>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_int16 = static_cast<int16>(src->m_ldouble);
	}
	else if (dst_type == Type<uint16>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_uint16 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_uint16 = static_cast<uint16>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_uint16 = static_cast<uint16>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_uint16 = static_cast<uint16>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_uint16 = static_cast<uint16>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_uint16 = static_cast<uint16>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_uint16 = static_cast<uint16>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_uint16 = static_cast<uint16>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_uint16 = static_cast<uint16>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_uint16 = static_cast<uint16>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_uint16 = static_cast<uint16>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_uint16 = static_cast<uint16>(src->m_ldouble);
	}
	else if (dst_type == Type<int32>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_int32 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_int32 = static_cast<int32>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_int32 = static_cast<int32>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_int32 = static_cast<int32>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_int32 = static_cast<int32>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_int32 = static_cast<int32>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_int32 = static_cast<int32>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_int32 = static_cast<int32>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_int32 = static_cast<int32>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_int32 = static_cast<int32>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_int32 = static_cast<int32>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_int32 = static_cast<int32>(src->m_ldouble);
	}
	else if (dst_type == Type<uint32>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_uint32 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_uint32 = static_cast<uint32>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_uint32 = static_cast<uint32>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_uint32 = static_cast<uint32>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_uint32 = static_cast<uint32>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_uint32 = static_cast<uint32>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_uint32 = static_cast<uint32>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_uint32 = static_cast<uint32>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_uint32 = static_cast<uint32>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_uint32 = static_cast<uint32>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_uint32 = static_cast<uint32>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_uint32 = static_cast<uint32>(src->m_ldouble);
	}
	else if (dst_type == Type<int64>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_int64 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_int64 = static_cast<int64>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_int64 = static_cast<int64>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_int64 = static_cast<int64>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_int64 = static_cast<int64>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_int64 = static_cast<int64>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_int64 = static_cast<int64>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_int64 = static_cast<int64>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_int64 = static_cast<int64>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_int64 = static_cast<int64>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_int64 = static_cast<int64>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_int64 = static_cast<int64>(src->m_ldouble);
	}
	else if (dst_type == Type<uint64>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_uint64 = src->m_bool ? 1 : 0;
		else if (src_type == Type<int8>::Id())        dst->m_uint64 = static_cast<uint64>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_uint64 = static_cast<uint64>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_uint64 = static_cast<uint64>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_uint64 = static_cast<uint64>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_uint64 = static_cast<uint64>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_uint64 = static_cast<uint64>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_uint64 = static_cast<uint64>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_uint64 = static_cast<uint64>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_uint64 = static_cast<uint64>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_uint64 = static_cast<uint64>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_uint64 = static_cast<uint64>(src->m_ldouble);
	}
	else if (dst_type == Type<float>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_float = src->m_bool ? 1.0f : 0.0f;
		else if (src_type == Type<int8>::Id())        dst->m_float = static_cast<float>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_float = static_cast<float>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_float = static_cast<float>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_float = static_cast<float>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_float = static_cast<float>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_float = static_cast<float>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_float = static_cast<float>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_float = static_cast<float>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_float = static_cast<float>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_float = static_cast<float>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_float = static_cast<float>(src->m_ldouble);
	}
	else if (dst_type == Type<double>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_double = src->m_bool ? 1.0 : 0.0;
		else if (src_type == Type<int8>::Id())        dst->m_double = static_cast<double>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_double = static_cast<double>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_double = static_cast<double>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_double = static_cast<double>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_double = static_cast<double>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_double = static_cast<double>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_double = static_cast<double>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_double = static_cast<double>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_double = static_cast<double>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_double = static_cast<double>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_double = static_cast<double>(src->m_ldouble);
	}
	else if (dst_type == Type<long double>::Id())
	{
		if (src_type == Type<bool>::Id())             dst->m_ldouble = src->m_bool ? 1.0 : 0.0;
		else if (src_type == Type<int8>::Id())        dst->m_ldouble = static_cast<long double>(src->m_int8);
		else if (src_type == Type<uint8>::Id())       dst->m_ldouble = static_cast<long double>(src->m_uint8);
		else if (src_type == Type<int16>::Id())       dst->m_ldouble = static_cast<long double>(src->m_int16);
		else if (src_type == Type<uint16>::Id())      dst->m_ldouble = static_cast<long double>(src->m_uint16);
		else if (src_type == Type<int32>::Id())       dst->m_ldouble = static_cast<long double>(src->m_int32);
		else if (src_type == Type<uint32>::Id())      dst->m_ldouble = static_cast<long double>(src->m_uint32);
		else if (src_type == Type<int64>::Id())       dst->m_ldouble = static_cast<long double>(src->m_int64);
		else if (src_type == Type<uint64>::Id())      dst->m_ldouble = static_cast<long double>(src->m_uint64);
		else if (src_type == Type<float>::Id())       dst->m_ldouble = static_cast<long double>(src->m_float);
		else if (src_type == Type<double>::Id())      dst->m_ldouble = static_cast<long double>(src->m_double);
		else if (src_type == Type<long double>::Id()) dst->m_ldouble = static_cast<long double>(src->m_ldouble);
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//