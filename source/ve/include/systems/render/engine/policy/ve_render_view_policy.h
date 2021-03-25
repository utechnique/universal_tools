//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
// Policies.
#include "ve_render_model_policy.h"
#include "ve_render_directional_light_policy.h"
#include "ve_render_point_light_policy.h"
#include "ve_render_spot_light_policy.h"

//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit_mgr.h"
#include "systems/render/engine/lighting/ve_lighting_manager.h"
#include "systems/render/engine/post_process/ve_post_process_mgr.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// View policy renders the environment to the view units.
template<> class Policy<View>
{
public:
	// Constructor.
	Policy(Toolset &toolset, UnitSelector& unit_selector, Policies& engine_policies);

	// Initializes provided view unit.
	void Initialize(View& view);

	// Renders all render units to all render views.
	void RenderEnvironment(Context& context);

private:
	// G-Buffer target format.
	static constexpr pixel::Format skDepthFormat = pixel::d24_unorm_s8_uint;
	static constexpr pixel::Format skGBufferFormat = pixel::r8g8b8a8_unorm;

	// policy tools
	Toolset& tools;
	Policies& policies;
	UnitSelector& selector;

	// managers
	lighting::Manager lighting_mgr;
	postprocess::Manager post_process_mgr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
