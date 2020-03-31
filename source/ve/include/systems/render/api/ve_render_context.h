//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_target.h"
#include "systems/render/api/ve_render_display.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Context class represents a device context interface which
// generates rendering commands.
class Context : public PlatformContext
{
public:
	// Constructor.
	Context(PlatformContext platform_context);

	// Move constructor.
	Context(Context&&) = default;

	// Move operator.
	Context& operator =(Context&&) = default;

	// Copying is prohibited.
	Context(const Context&) = delete;
	Context& operator =(const Context&) = delete;

	// Set all the elements in a render target to one value.
	void ClearTarget(Target& target, float* color);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//