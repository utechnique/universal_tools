//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/lighting/ve_lighting_manager.h"
#include "systems/render/engine/policy/ve_render_ambient_light_policy.h"
#include "systems/render/engine/policy/ve_render_directional_light_policy.h"
#include "systems/render/engine/policy/ve_render_point_light_policy.h"
#include "systems/render/engine/policy/ve_render_spot_light_policy.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
Manager::Manager(Toolset& toolset) : tools(toolset)
                                   , ibl(toolset)
                                   , deferred_shading(toolset, ibl.GetMipCount())
                                   , forward_shading(toolset, ibl.GetMipCount())                           
{}

// Creates lighting (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @param is_cube - 'true' to create as a cubemap.
//    @return - a new lighting::ViewData object or error if failed.
ut::Result<lighting::ViewData, ut::Error> Manager::CreateViewData(Target& depth_stencil,
                                                                  ut::uint32 width,
                                                                  ut::uint32 height,
                                                                  bool is_cube,
                                                                  ut::uint32 light_buffer_mip_count)
{
	// light buffer
	Target::Info info;
	info.type = is_cube ? Image::type_cube : Image::type_2D;
	info.format = tools.formats.light_buffer;
	info.usage = Target::Info::usage_color;
	info.mip_count = light_buffer_mip_count;
	info.width = width;
	info.height = height;
	info.depth = 1;
	ut::Result<Target, ut::Error> light_buffer = tools.device.CreateTarget(info);
	if (!light_buffer)
	{
		return ut::MakeError(light_buffer.MoveAlt());
	}

	// deferred shading
	ut::Result<DeferredShading::ViewData, ut::Error> def_sh_data = deferred_shading.CreateViewData(depth_stencil,
	                                                                                               light_buffer.Get(),
	                                                                                               width, height,
	                                                                                               is_cube);
	if (!def_sh_data)
	{
		return ut::MakeError(def_sh_data.MoveAlt());
	}

	// forward shading
	ut::Result<ForwardShading::ViewData, ut::Error> forward_sh_data = forward_shading.CreateViewData(depth_stencil,
	                                                                                                 light_buffer.Get(),
	                                                                                                 is_cube);
	if (!forward_sh_data)
	{
		return ut::MakeError(forward_sh_data.MoveAlt());
	}

	lighting::ViewData data = 
	{
	    light_buffer.Move(),
		def_sh_data.Move(),
		forward_sh_data.Move()
	};

	return data;
}

// Updates uniform buffers for the provided light units.
void Manager::UpdateLightUniforms(Context& context, Light::Sources& lights)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();

	// directional lights
	const size_t directional_light_count = lights.directional.Count();
	for (size_t i = 0; i < directional_light_count; i++)
	{
		DirectionalLight& light = lights.directional[i];
		UpdateLightUniforms(context,
		                    light.data->frames[current_frame_id].uniform_buffer,
		                    light.world_transform.translation,
		                    light.color,
		                    Light::GetDirection(light.world_transform.rotation),
		                    ut::Vector<3>(0),
		                    light.intensity,
		                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	}

	// point lights
	const size_t point_light_count = lights.point.Count();
	for (size_t i = 0; i < point_light_count; i++)
	{
		PointLight& light = lights.point[i];
		UpdateLightUniforms(context,
		                    light.data->frames[current_frame_id].uniform_buffer,
		                    light.world_transform.translation,
		                    light.color,
		                    ut::Vector<3>(0),
		                    Light::GetTubeDirection(light.world_transform.rotation),
		                    light.intensity,
		                    light.attenuation_distance,
		                    0.0f, 0.0f,
		                    light.shape_radius,
		                    light.shape_length);
	}

	// spot lights
	const size_t spot_light_count = lights.spot.Count();
	for (size_t i = 0; i < spot_light_count; i++)
	{
		SpotLight& light = lights.spot[i];
		UpdateLightUniforms(context,
		                    light.data->frames[current_frame_id].uniform_buffer,
		                    light.world_transform.translation,
		                    light.color,
		                    Light::GetDirection(light.world_transform.rotation),
		                    Light::GetTubeDirection(light.world_transform.rotation),
		                    light.intensity,
		                    light.attenuation_distance,
		                    light.inner_cone,
		                    light.outer_cone,
		                    light.shape_radius,
		                    light.shape_length);
	}

	// ambient lights
	const size_t ambient_light_count = lights.ambient.Count();
	for (size_t i = 0; i < ambient_light_count; i++)
	{
		AmbientLight& light = lights.ambient[i];
		UpdateLightUniforms(context,
		                    light.data->frames[current_frame_id].uniform_buffer,
		                    light.world_transform.translation,
		                    light.color,
		                    Light::GetDirection(light.world_transform.rotation),
		                    Light::GetTubeDirection(light.world_transform.rotation),
		                    light.intensity,
		                    light.attenuation_distance,
		                    0.0f, 0.0f, 0.0f, 0.0f);
	}
}

// Updates light source buffer with provided data.
void Manager::UpdateLightUniforms(Context& context,
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
                                  float shape_length)
{
	DirectionalLight::Uniforms uniforms;

	uniforms.position.X() = position.X();
	uniforms.position.Y() = position.Y();
	uniforms.position.Z() = position.Z();

	uniforms.direction.X() = direction.X();
	uniforms.direction.Y() = direction.Y();
	uniforms.direction.Z() = direction.Z();
	uniforms.direction.W() = shape_radius;

	uniforms.color.R() = color.R() * intensity;
	uniforms.color.G() = color.G() * intensity;
	uniforms.color.B() = color.B() * intensity;
	uniforms.color.A() = shape_length;

	uniforms.attenuation.X() = attenuation_distance;
	uniforms.attenuation.Y() = ut::Cos(ut::ToRadiands(inner_cone));
	uniforms.attenuation.Z() = ut::Cos(ut::ToRadiands(outer_cone));

	uniforms.orientation.X() = orientation.X();
	uniforms.orientation.Y() = orientation.Y();
	uniforms.orientation.Z() = orientation.Z();

	tools.rc_mgr.UpdateBuffer(context, buffer, &uniforms);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//