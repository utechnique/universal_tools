//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Enumeration of possible variants how custom data can be encoded as a color
// of one pixel.
namespace pixel
{
	enum Format
	{
		unknown,
		r8g8b8a8_unorm,
		b8g8r8a8_unorm,
		r8g8b8a8_srgb,
		b8g8r8a8_srgb,
		r16_float,
		r16_sint,
		r16_uint,
		r16_snorm,
		r16_unorm,
		r16g16_float,
		r16g16_sint,
		r16g16_uint,
		r16g16_snorm,
		r16g16_unorm,
		r16g16b16a16_float,
		r16g16b16a16_sint,
		r16g16b16a16_uint,
		r16g16b16a16_snorm,
		r16g16b16a16_unorm,
		r32_float,
		r32_sint,
		r32_uint,
		r32g32_float,
		r32g32_sint,
		r32g32_uint,
		r32g32b32_float,
		r32g32b32_sint,
		r32g32b32_uint,
		r32g32b32a32_float,
		r32g32b32a32_sint,
		r32g32b32a32_uint,
		d16_unorm,
		d24_unorm_s8_uint,
		d32_float,
		d32_float_s8_uint,
	};

	// Returns size of one pixel of the specified format in bytes.
	constexpr ut::uint32 GetSize(Format format)
	{
		return format == r8g8b8a8_unorm     ? 4 :
		       format == b8g8r8a8_unorm     ? 4 :
		       format == r8g8b8a8_srgb      ? 4 :
		       format == b8g8r8a8_srgb      ? 4 :
		       format == r16_float          ? 2 :
		       format == r16_sint           ? 2 :
		       format == r16_uint           ? 2 :
		       format == r16_snorm          ? 2 :
		       format == r16_unorm          ? 2 :
		       format == r16g16_float       ? 4 :
		       format == r16g16_sint        ? 4 :
		       format == r16g16_uint        ? 4 :
		       format == r16g16_snorm       ? 4 :
		       format == r16g16_unorm       ? 4 :
		       format == r16g16b16a16_float ? 8 :
		       format == r16g16b16a16_sint  ? 8 :
		       format == r16g16b16a16_uint  ? 8 :
		       format == r16g16b16a16_snorm ? 8 :
		       format == r16g16b16a16_unorm ? 8 :
		       format == r32_float          ? 4 :
		       format == r32_sint           ? 4 :
		       format == r32_uint           ? 4 :
		       format == r32g32_float       ? 8 :
		       format == r32g32_sint        ? 8 :
		       format == r32g32_uint        ? 8 :
		       format == r32g32b32_float    ? 12 :
		       format == r32g32b32_sint     ? 12 :
		       format == r32g32b32_uint     ? 12 :
		       format == r32g32b32a32_float ? 16 :
		       format == r32g32b32a32_sint  ? 16 :
		       format == r32g32b32a32_uint  ? 16 :
		       format == d16_unorm          ? 2 :
		       format == d24_unorm_s8_uint  ? 4 :
		       format == d32_float          ? 4 :
		       format == d32_float_s8_uint  ? 8 :
		       0;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//