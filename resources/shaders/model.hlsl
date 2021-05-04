//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Binding indices for shader resources included from separate files.
#define UB_ID_VIEW 0
#define UB_ID_LIGHT 1

//----------------------------------------------------------------------------//
// Define 'INSTANCING' macro to enable or disable instancing.
#ifndef INSTANCING
#define INSTANCING 0
#endif

// Define 'ALPHA_TEST' macro to enable or disable alpha test.
#ifndef ALPHA_TEST
#define ALPHA_TEST 0
#endif

// Define 'DEFERRED_PASS' macro to compile shader rendering polygons to the
// geometry buffer.
#ifndef DEFERRED_PASS
#define DEFERRED_PASS 0
#endif

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

//----------------------------------------------------------------------------//
#include "vertex.hlsl"
#include "view_buffer.hlsl"
#if LIGHT_PASS
#include "light_buffer.hlsl"
#endif
#if LIGHT_PASS || IBL_PASS
#include "lighting.hlsl"
#endif

//----------------------------------------------------------------------------//
// Define return type of the pixel shader.
#if DEFERRED_PASS
#include "deferred_common.hlsl"
#define PS_OUTPUT GBuffer
#else
#define PS_OUTPUT float4
#endif

//----------------------------------------------------------------------------//
// VS -> PS data.
struct PS_INPUT 
{
	float4 position : SV_POSITION;

#if !DEFERRED_PASS
	float3 world_position : TEXCOORD5;
#endif

#if VERTEX_HAS_TEXCOORD
	float2 texcoord : TEXCOORD0;
#endif

#if VERTEX_HAS_NORMAL
	float3 normal : TEXCOORD1;
#endif

#if VERTEX_HAS_TANGENT
	float3 tangent : TEXCOORD2;
	float3 binormal : TEXCOORD3;
#endif

#if INSTANCING
	uint instance_id : TEXCOORD4;
#endif
};

// Per-instance material buffer.
struct Material
{
	float4 diffuse_add;
	float4 diffuse_mul;
	float4 material_add;
	float4 material_mul;
};

//----------------------------------------------------------------------------//
// Shader resources.
cbuffer g_ub_transform : register(b2)
{
#if INSTANCING
	row_major float4x4 g_transform[BATCH_SIZE];
#else
	row_major float4x4 g_transform;
#endif
};

cbuffer g_ub_material : register(b3)
{
#if INSTANCING
	Material g_material[BATCH_SIZE];
#else
	Material g_material;
#endif
};

SamplerState g_sampler : register(s4);
Texture2D g_tex2d_diffuse : register(t5);
Texture2D g_tex2d_normal : register(t6);
Texture2D g_tex2d_material : register(t7);
#if IBL_PASS
TextureCube g_ibl_cubemap : register(t8);
#endif

//----------------------------------------------------------------------------//
// Vertex shader entry.
PS_INPUT VS(Vertex input)
{
	PS_INPUT output;

#if INSTANCING
	row_major float4x4 transform = g_transform[input.instance_id];
#else
	row_major float4x4 transform = g_transform;
#endif

	// position
#if VERTEX_2D_POSITION
	float3 position_3d = float3(input.position.xy, 0.0f);
#else
	float3 position_3d = input.position.xyz;
#endif
	float3 world_position = mul(float4(position_3d, 1.0f), transform);
	output.position = mul(float4(world_position, 1.0f), g_view_proj);

	// world position
#if !DEFERRED_PASS
	output.world_position = world_position;
#endif

	// texcoord
#if VERTEX_HAS_TEXCOORD
	output.texcoord = input.texcoord;
#endif

	// normal
#if VERTEX_HAS_NORMAL
	output.normal = mul(input.normal, transform);
#endif

	// tangent, binormal
#if VERTEX_HAS_TANGENT
	output.tangent = mul(input.tangent, transform);
	output.binormal = cross(output.normal, output.tangent);
#endif

	// instance id
#if INSTANCING
	output.instance_id = input.instance_id;
#endif

	return output;
}

//----------------------------------------------------------------------------//
// Pixel shader entry.
PS_OUTPUT PS(PS_INPUT input) : SV_Target
{
	PS_OUTPUT output;

	// extract texcoord
#if VERTEX_HAS_TEXCOORD
	float2 texcoord = input.texcoord;
#else
	float2 texcoord = 0.0f;
#endif

	// sample diffuse texture
	float4 diffuse_sample = g_tex2d_diffuse.Sample(g_sampler, texcoord);
#if ALPHA_TEST
	float alpha = diffuse_sample.a;
	if (alpha < 0.25f)
	{
		clip(-1);
	}
#endif // ALPHA_TEST

	// sample normal and material textures
	float4 normal_sample = g_tex2d_normal.Sample(g_sampler, texcoord);
	float4 material_sample = g_tex2d_material.Sample(g_sampler, texcoord);

	// calculate material data
#if INSTANCING
	Material material = g_material[input.instance_id];
#else
	Material material = g_material;
#endif
	float3 diffuse_color = diffuse_sample.rgb * material.diffuse_mul.rgb + material.diffuse_add.rgb;
	float3 normal_color = normal_sample.rgb;
	float3 material_color = material_sample.rgb * material.material_mul.rgb + material.material_add.rgb;
	float roughness = material_color.r;
	float metallic = material_color.g;

	// calculate final normal
#if VERTEX_HAS_NORMAL && VERTEX_HAS_TANGENT
	float3x3 world_to_tangent = float3x3(input.tangent, input.binormal, input.normal);
	float3 tbn_normal = normalize(-1 + (2 * normal_color));
	float3 normal = normalize(mul(tbn_normal, world_to_tangent));
#elif VERTEX_HAS_NORMAL
	float3 normal = input.normal + normal_sample.a;
#else
	float3 normal = normal_sample.rgb;
#endif

	// output
#if DEFERRED_PASS
	output.diffuse = float4(diffuse_color.rgb, roughness);
	output.normal = float4(normal, metallic);
#else
	SurfaceData surface;
	surface.world_position = input.world_position;
	surface.look = normalize(g_camera_position.xyz - surface.world_position);
	surface.normal = normal;
	surface.diffuse = diffuse_color;
	surface.specular = 0.5f;
	surface.roughness = roughness;
	surface.min_roughness = 0.04f;
	surface.metallic = metallic;
	surface.cavity = 1.0f;

	#if IBL
		CalculateMetallicDiffuseSpecular(surface);
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
			float3 light_amount = ComputeDirectLighting(surface, light);
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
	#endif // LIGHT_PASS
	output.rgb = light_amount;
	output.a = diffuse_sample.a;
#endif // !DEFERRED_PASS

	return output;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//