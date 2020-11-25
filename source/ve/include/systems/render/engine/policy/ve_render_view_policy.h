//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
// Policies.
#include "ve_render_mesh_policy.h"

//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit_mgr.h"

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
	// Creates a render pass for rendering geometry to a g-buffer.
	static ut::Result<RenderPass, ut::Error> CreateGeometryPass(Device& device);

	// Creates shaders for rendering geometry to a g-buffer.
	static BoundShader CreateGeometryPassShader(ShaderLoader& shader_loader);

	// Creates pipeline for rendering geometry to a g-buffer.
	ut::Result<PipelineState, ut::Error> CreateGeometryPassPipeline(ut::uint32 width,
	                                                                ut::uint32 height);

	// G-Buffer target format.
	static constexpr pixel::Format skDepthFormat = pixel::d24_unorm_s8_uint;
	static constexpr pixel::Format skGBufferFormat = pixel::r8g8b8a8_unorm;

	// render resources
	RenderPass geometry_pass;
	BoundShader geometry_pass_shader;

	// policy tools
	Toolset& tools;
	Policies& policies;
	UnitSelector& selector;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
