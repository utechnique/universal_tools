//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_primitive_topology.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts primitive topology to vulkan acceptable format.
VkPrimitiveTopology ConvertPrimitiveTopologyToVulkan(primitive::Topology topology)
{
	switch (topology)
	{
	case primitive::Topology::point_list:                    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case primitive::Topology::line_list:                     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case primitive::Topology::line_list_with_adjacency:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case primitive::Topology::line_strip:                    return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case primitive::Topology::line_strip_with_adjacency:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
	case primitive::Topology::triangle_list:                 return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case primitive::Topology::triangle_list_with_adjacency:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
	case primitive::Topology::triangle_strip:                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case primitive::Topology::triangle_strip_with_adjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
	case primitive::Topology::patch_list:                    return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}
	return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//