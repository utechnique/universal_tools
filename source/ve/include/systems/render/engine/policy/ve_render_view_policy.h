//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
// Policies.
#include "ve_render_mesh_instance_policy.h"
#include "ve_render_directional_light_policy.h"
#include "ve_render_point_light_policy.h"
#include "ve_render_spot_light_policy.h"
#include "ve_render_ambient_light_policy.h"

//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit_mgr.h"
#include "systems/render/engine/lighting/ve_lighting_manager.h"
#include "systems/render/engine/post_process/ve_post_process_mgr.h"
#include "systems/render/engine/ve_render_hitmask.h"

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
	//    @param context - reference to the render context.
	//    @param time_step_ms - time step in milliseconds.
	void RenderEnvironment(Context& context, System::Time time_step_ms);

private:
	// Renders the provided view.
	void RenderView(Context& context, View& view, Light::Sources& lights);

	// Creates hdr scene targets.
	ut::Result<View::SceneBuffer, ut::Error> CreateSceneBuffer(ut::uint32 width,
	                                                           ut::uint32 height,
	                                                           bool is_cube,
	                                                           ut::uint32 light_buffer_mip_count = 1);

	// Renders environment.
	Image& RenderLightPass(Context& context,
	                       View::SceneBuffer& scene,
	                       Light::Sources& lights,
	                       const ut::Matrix<4>& view_matrix,
	                       const ut::Matrix<4>& proj_matrix,
	                       const ut::Vector<3>& view_position,
	                       View::LightPassMode light_pass_mode,
	                       ut::Optional<Image&> ibl_cubemap = ut::Optional<Image&>(),
	                       Image::Cube::Face face = Image::Cube::positive_x);

	// Renders IBL cubemap.
	void RenderIblCubemap(Context& context,
	                      View& view,
	                      Light::Sources& lights);

	// Updates view uniform buffer.
	void UpdateViewUniforms(Context& context,
	                        View::SceneBuffer& scene,
	                        const ut::Matrix<4>& view_matrix,
	                        const ut::Matrix<4>& proj_matrix,
	                        const ut::Vector<3>& view_position,
	                        Image::Cube::Face face);

	// G-Buffer target format.
	static constexpr pixel::Format skPreferredDepthFormat = pixel::d24_unorm_s8_uint;
	static constexpr pixel::Format skAlternativeDepthFormat = pixel::d32_float_s8_uint;
	static constexpr pixel::Format skGBufferFormat = pixel::r8g8b8a8_unorm;

	// policy tools
	Toolset& tools;
	Policies& policies;
	UnitSelector& selector;

	// managers
	lighting::Manager lighting_mgr;
	postprocess::Manager post_process_mgr;
	HitMask hitmask;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
