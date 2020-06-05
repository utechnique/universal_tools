//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Graphics engine works with fixed types of queue.
namespace vulkan_queue
{
	enum FamilyType
	{
		main, // general-purpose queue (graphics+compute+transfer)
		compute, // compute-only queue
		transfer, // transfer-only queue (streaming)
	};

	struct Family
	{
		// id of this queue family
		uint32_t id;

		// number of queues available on gpu for this family
		uint32_t count; 
	};
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//