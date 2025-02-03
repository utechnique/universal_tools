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
// Supported primitive topologies.
namespace primitive
{
	enum class Topology
	{
		point_list,
		line_list,
		line_list_with_adjacency,
		line_strip,
		line_strip_with_adjacency,
		triangle_list,
		triangle_list_with_adjacency,
		triangle_strip,
		triangle_strip_with_adjacency,
		patch_list,
	};
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//