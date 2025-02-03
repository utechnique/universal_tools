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
// ve::render::VertexElement describes single element of a vertex such as
// position, texcoord, etc.
class VertexElement
{
public:
	// Constructor.
	VertexElement(ut::String in_semantic_name,
	              pixel::Format in_format,
	              ut::uint32 in_offset) : semantic_name(ut::Move(in_semantic_name))
                                        , format(in_format)
                                        , offset(in_offset)
	{}

	// name of the element (POSITION, TEXCOORD, TANGENT, etc.)
	ut::String semantic_name;

	// format of the element
	pixel::Format format;

	// offset from the beginning of the vertex (in bytes)
	ut::uint32 offset;
};

// Type of index buffer indices.
enum class IndexType
{
	uint16,
	uint32
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//