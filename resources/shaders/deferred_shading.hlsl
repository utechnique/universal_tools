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
#include "view_buffer.hlsl"
#include "light_buffer.hlsl"
#include "lighting.hlsl"

//----------------------------------------------------------------------------//
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

//----------------------------------------------------------------------------//
// Calculates world 3d position of the pixel using the depth value from the
// G-Buffer.
float3 RecoverPositionFromDepth(float4x4 view_projection,
                                float2 texcoord,
                                float depth)
{
	float4 out_position;
	out_position.x = texcoord.x * 2.0f - 1.0f;
	out_position.y = -(texcoord.y * 2.0f - 1.0f);
	out_position.z = depth;
	out_position.w = 1.0f;
	out_position = mul(out_position, view_projection);
	out_position /= out_position.w;
	return out_position.xyz;
}

//----------------------------------------------------------------------------//
// Pixel shader entry point.
float4 PS(PS_INPUT input) : SV_Target
{
	// decode G-Buffer
	float depth = g_tex2d_depth.Sample(g_sampler, input.texcoord).r;
	float4 diffuse = g_tex2d_diffuse.Sample(g_sampler, input.texcoord);
	float4 normal = g_tex2d_normal.Sample(g_sampler, input.texcoord);

	float3 world_position = RecoverPositionFromDepth(g_inv_view_proj, input.texcoord, depth);
	float3 view_direction = normalize(g_camera_position.xyz - world_position);
#if DIRECTIONAL_LIGHT
	float3 light_direction = -g_light_direction.xyz;
#else
	float3 light_direction = g_light_position.xyz - world_position;
#endif

	// initialize surface data
	SurfaceData surface;
	surface.look = view_direction;
	surface.normal = normal;
	surface.diffuse = diffuse.rgb;
	surface.specular = 0.5f;
	surface.roughness = diffuse.a;
	surface.min_roughness = 0.04f;
	surface.cavity = 1.0f;

	// initialize light source data
	LightingData light;
	light.direction = normalize(light_direction);
	light.color = g_light_color.rgb;
	light.tube_direction = g_light_orientation.xyz;
	light.ray_length = length(light_direction);
	light.source_radius = g_light_direction.w;
	light.source_length = g_light_color.w;
	light.is_radial = light.source_radius > 0.001f;

	// calculate lighting
	float4 light_amount = ComputeDirectLighting(surface, light);
	light_amount.rgb *= ComputeAttenuation(light);

	return light_amount;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//