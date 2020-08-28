//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "systems/render/api/dx11/ve_dx11_primitive_topology.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts primitive topology to the one compatible with DirectX11.
D3D11_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopologyToDX11(primitive::Topology topology)
{
	switch (topology)
	{
	case primitive::point_list:                    return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	case primitive::line_list:                     return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case primitive::line_list_with_adjacency:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
	case primitive::line_strip:                    return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case primitive::line_strip_with_adjacency:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
	case primitive::triangle_list:                 return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case primitive::triangle_list_with_adjacency:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
	case primitive::triangle_strip:                return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case primitive::triangle_strip_with_adjacency: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
	case primitive::patch_list:                    return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	}
	return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//