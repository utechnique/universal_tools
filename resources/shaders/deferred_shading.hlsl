//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// This shader performs deferred shading pass for directional, point and
// spot lights. Use DIRECTIONAL_LIGHT, POINT_LIGHT and SPOT_LIGHT macros to
// compile this shader for specific light source type.
//----------------------------------------------------------------------------//
// Binding indices for shader resources included from separate files.
#define UB_ID_VIEW 0
#define UB_ID_LIGHT 1


//----------------------------------------------------------------------------//
// Define 'LIGHT_PASS' macro to compile light pass shader.
#ifndef LIGHT_PASS
#define LIGHT_PASS 0
#endif

// Define 'IBL_PASS' macro to compile ibl reflection pass shader.
#ifndef IBL_PASS
#define IBL_PASS 0
#endif

// Define 'IBL_MIP_COUNT' macro to set the number of
// mip levels in the ibl cubemap.
#ifndef IBL_MIP_COUNT
#define IBL_MIP_COUNT 8
#endif

// Define 'IBL' macro to enable or disable image based lighting.
#ifndef IBL
#define IBL 0
#endif

//----------------------------------------------------------------------------//
#if LIGHT_PASS
#include "light_buffer.hlsl"
#endif
#include "view_buffer.hlsl"
#include "lighting.hlsl"
#include "deferred_common.hlsl"

//----------------------------------------------------------------------------//
// VS -> PS data.
struct PS_INPUT
{
	float4 position  : SV_POSITION;
	float2 texcoord  : TEXCOORD0;
};

//----------------------------------------------------------------------------//
// Shader resources
SamplerState g_sampler : register(s2);
Texture2D g_tex2d_depth : register(t3);
Texture2D g_tex2d_diffuse : register(t4);
Texture2D g_tex2d_normal : register(t5);
#if IBL_PASS
TextureCube g_ibl_cubemap : register(t6);
#endif

//----------------------------------------------------------------------------//
// Pixel shader entry point.
float4 PS(PS_INPUT input) : SV_Target
{
	// decode G-Buffer
	float depth = g_tex2d_depth.Sample(g_sampler, input.texcoord).r;
	float4 diffuse = g_tex2d_diffuse.Sample(g_sampler, input.texcoord);
	float4 normal = g_tex2d_normal.Sample(g_sampler, input.texcoord);

	// initialize surface data
	SurfaceData surface = DecodeDeferredSurface(depth,
	                                            diffuse,
	                                            normal,
	                                            input.texcoord,
	                                            g_inv_view_proj,
	                                            g_camera_position.xyz);
#if IBL
	CalculateMetallicDiffuseSpecular(surface);
#elif AMBIENT_LIGHT
	// prevent normal map from being optimized out
	if (surface.diffuse.r < -1.0f)
	{
		surface.diffuse += surface.metallic * 0.001f;
	}
#endif

#if LIGHT_PASS
	// initialize light source data
	#if DIRECTIONAL_LIGHT
		float3 light_direction = -g_light_direction.xyz;
	#else
		float3 light_direction = g_light_position.xyz - surface.world_position;
	#endif
	LightingData light;
	light.direction = normalize(light_direction);
	light.color = g_light_color.rgb;
	light.tube_direction = g_light_orientation.xyz;
	light.ray_length = length(light_direction);
	light.source_radius = g_light_direction.w;
	light.source_length = g_light_color.w;
	light.is_radial = light.source_radius > 0.001f;

	// calculate lighting
	#if AMBIENT_LIGHT
		float3 light_amount = ComputeAmbientLighting(surface, light);
	#else
		float3 light_amount = ComputeDirectLighting(surface, light);
	#endif
	light_amount *= ComputeAttenuation(light);
#elif IBL_PASS
	float3 view_direction = -surface.look;
	float3 reflection_vector = normalize(reflect(view_direction, surface.normal));
	float NoV = saturate(dot(view_direction, surface.normal));
	float3 light_amount = GetImageBasedReflectionLighting(g_ibl_cubemap,
	                                                      g_sampler,
	                                                      surface.roughness,
	                                                      reflection_vector,
	                                                      NoV,
	                                                      surface.specular,
	                                                      IBL_MIP_COUNT);
#endif
	return float4(light_amount.rgb, 0.0f);
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//