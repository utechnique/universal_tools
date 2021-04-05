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
// Define 'IBL_MIP_COUNT' macro to set the number of
// mip levels in the ibl cubemap.
#ifndef IBL_MIP_COUNT
#define IBL_MIP_COUNT 8
#endif

//----------------------------------------------------------------------------//
#include "view_buffer.hlsl"
#include "lighting.hlsl"
#include "deferred_common.hlsl"

//----------------------------------------------------------------------------//
struct PS_INPUT
{
	float4 position  : SV_POSITION;
	float2 texcoord  : TEXCOORD0;
};

//----------------------------------------------------------------------------//
// Shader resources
SamplerState g_sampler : register(s2);
SamplerState g_ibl_sampler : register(s3);
Texture2D g_tex2d_depth : register(t4);
Texture2D g_tex2d_diffuse : register(t5);
Texture2D g_tex2d_normal : register(t6);
TextureCube g_ibl_cubemap : register(t7);

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
	CalculateMetallicDiffuseSpecular(surface);

	// calculate IBL
	float3 view_direction = -surface.look;
	float3 reflection_vector = normalize(reflect(view_direction, surface.normal));
	float NoV = saturate(dot(view_direction, surface.normal));
	float3 light_amount = GetImageBasedReflectionLighting(g_ibl_cubemap,
	                                                      g_ibl_sampler,
	                                                      surface.roughness,
	                                                      reflection_vector,
	                                                      NoV,
	                                                      surface.specular,
	                                                      IBL_MIP_COUNT);

	return float4(light_amount.rgb, 0.0f);
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//