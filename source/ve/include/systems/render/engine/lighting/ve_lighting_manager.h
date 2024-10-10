//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/units/ve_render_view.h"
#include "ve_deferred_shading.h"
#include "ve_forward_shading.h"
#include "ve_image_based_lighting.h"
#include "ve_render_unlit.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Lighting manager encapsulates different lighting techniques.
class Manager
{
public:
	// Constructor.
	Manager(Toolset& toolset);

	// Creates lighting (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @param is_cube - 'true' to create as a cubemap.
	//    @return - a new lighting::ViewData object or error if failed.
	ut::Result<lighting::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                         ut::uint32 width,
	                                                         ut::uint32 height,
	                                                         bool is_cube,
	                                                         ut::uint32 light_buffer_mip_count = 1);

	// Updates uniform buffers for the provided light units.
	void UpdateLightUniforms(Context& context, Light::Sources& lights);

	// light techniques
	IBL ibl;
	DeferredShading deferred_shading;
	ForwardShading forward_shading;
	UnlitRenderer unlit;

private:
	// Updates light source buffer with provided data.
	void UpdateLightUniforms(Context& context,
	                         Buffer& buffer,
	                         const ut::Vector<3>& position,
	                         const ut::Color<3>& color,
	                         const ut::Vector<3>& direction,
	                         const ut::Vector<3>& orientation,
	                         float intensity,
	                         float attenuation_distance,
	                         float inner_cone,
	                         float outer_cone,
	                         float shape_radius,
	                         float shape_length);

	Toolset& tools;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
