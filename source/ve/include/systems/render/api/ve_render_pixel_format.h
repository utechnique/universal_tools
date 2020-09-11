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
		r8g8b8,
		b8g8r8,
		r8g8b8a8,
		b8g8r8a8,
		r8g8b8a8_srgb,
		b8g8r8a8_srgb,
		r32,
		r32g32,
		r32g32b32,
		r32g32b32a32,
	};

	// Returns size of one pixel of the specified format in bytes.
	constexpr ut::uint32 GetSize(Format format)
	{
		return format == r8g8b8        ? 3 : 
		       format == b8g8r8        ? 3 :
		       format == r8g8b8a8      ? 4 :
		       format == b8g8r8a8      ? 4 :
		       format == r8g8b8a8_srgb ? 4 :
		       format == b8g8r8a8_srgb ? 4 :
		       format == r32           ? 4 :
		       format == r32g32        ? 8 :
		       format == r32g32b32     ? 12 :
		       format == r32g32b32a32  ? 16 :
		       0;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//