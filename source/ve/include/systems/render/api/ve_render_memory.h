//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(memory)
//----------------------------------------------------------------------------//
// Enumeration of possible variants how gpu object can be used.
enum class Usage
{
	// accessible only by the GPU (full access), fast
	gpu_read_write,

	// accessible by both the GPU (read only) and the CPU (write only),
	// slow, but better than staging
	gpu_read_cpu_write,

	// can't be modified after creation, cpu has no access, fast
	gpu_immutable
};

// Describes how a gpu object can be accessed from cpu.
enum class CpuAccess
{
	// managed data can only be read, modification is prohibited
	read,

	// managed data has undefined value on access and can only be overwritten
	write,

	// managed data can be both read and written
	read_write,
};

//----------------------------------------------------------------------------//
END_NAMESPACE(memory)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//