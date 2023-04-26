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
// Enumeration of possible variants how gpu object can be used.
namespace memory
{
	enum Usage
	{
		// accessible only by the GPU (full access), fast
		gpu_read_write,

		// accessible by both the GPU (read only) and the CPU (write only),
		// slow, but better than staging
		gpu_read_cpu_write,

		// can't be modified after creation, cpu has no access, fast
		gpu_immutable
	};
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//