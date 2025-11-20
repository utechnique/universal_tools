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

// Define 'IBL_PASS' macro to compile ibl reflection pass shader.
#ifndef IBL_PASS
#define IBL_PASS 0
#endif

// Define 'LIGHT_PASS' macro to compile light pass shader.
#ifndef LIGHT_PASS
#define LIGHT_PASS 0
#endif
#define AMBIENT_PASS (LIGHT_PASS && AMBIENT_LIGHT)

// Define 'EMISSIVE_PASS' macro to compile emissive pass shader.
#ifndef EMISSIVE_PASS
#define EMISSIVE_PASS 0
#endif

// Define 'HITMASK_PASS' macro to compile shader rendering the color
// associated with the desired entity id.
#ifndef HITMASK_PASS
#define HITMASK_PASS 0
#endif

// Define 'UNLIT_PASS' macro to compile shader rendering a mesh
// in unlit mode.
#ifndef UNLIT_PASS
#define UNLIT_PASS 0
#endif

// Define 'IBL_MIP_COUNT' macro to set the number of
// mip levels in the ibl cubemap.
#ifndef IBL_MIP_COUNT
#define IBL_MIP_COUNT 8
#endif

#define NEEDS_BASE_COLOR_MAP ((!HITMASK_PASS && !EMISSIVE_PASS) || ALPHA_TEST)
#define NEEDS_NORMAL_MAP (!AMBIENT_PASS && !HITMASK_PASS && !EMISSIVE_PASS && !UNLIT_PASS)
#define NEEDS_METALLIC_ROUGHNESS_MAP ((!AMBIENT_PASS || IBL) && !HITMASK_PASS && !EMISSIVE_PASS && !UNLIT_PASS)
#define NEEDS_OCCLUSION_MAP (DEFERRED_PASS || (LIGHT_PASS && AMBIENT_LIGHT))
#define NEEDS_EMISSIVE_MAP (DEFERRED_PASS || EMISSIVE_PASS)
#define NEEDS_MATERIAL_BUFFER ((!HITMASK_PASS) || ALPHA_TEST)
#define NEEDS_TEXTURE_COORD (NEEDS_BASE_COLOR_MAP || NEEDS_NORMAL_MAP || NEEDS_METALLIC_ROUGHNESS_MAP || NEEDS_EMISSIVE_MAP)

//----------------------------------------------------------------------------//
#include "vertex.hlsl"
#include "view_buffer.hlsl"
#if LIGHT_PASS
#include "light_buffer.hlsl"
#endif
#if LIGHT_PASS || IBL_PASS
#include "lighting.hlsl"
#endif
#if UNLIT_PASS
#include "gamma.hlsl"
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

#if VERTEX_HAS_TEXCOORD && NEEDS_TEXTURE_COORD
	float2 texcoord : TEXCOORD0;
#endif

#if HITMASK_PASS
	float4 hitmask_id : TEXCOORD1;
#elif !UNLIT_PASS
	#if VERTEX_HAS_NORMAL
		float3 normal : TEXCOORD1;
	#endif

	#if VERTEX_HAS_TANGENT
		float3 tangent : TEXCOORD2;
		float3 binormal : TEXCOORD3;
	#endif

	#if !DEFERRED_PASS
		float3 world_position : TEXCOORD4;
	#endif
#endif // !UNLIT_PASS

#if INSTANCING
	uint instance_id : TEXCOORD5;
#endif
};

// Per-instance material buffer.
struct Material
{
	float4 base_color_factor;

	float roughness_factor;
	float metallic_factor;
	float emissive_strength;
	float occlusion_strength;

	float alpha_cutoff;
	float normal_scale;
	float reserved1;
	float reserved2;
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

#if NEEDS_MATERIAL_BUFFER
cbuffer g_ub_material : register(b3)
{
	#if INSTANCING
		Material g_material[BATCH_SIZE];
	#else
		Material g_material;
	#endif
};
#endif // NEEDS_MATERIAL_BUFFER

#if HITMASK_PASS
cbuffer g_ub_hitmask_id : register(b4)
{
	#if INSTANCING
		float4 g_hitmask_id[BATCH_SIZE];
	#else
		float4 g_hitmask_id;
	#endif
};
#endif // HITMASK_PASS

#if NEEDS_TEXTURE_COORD
SamplerState g_sampler : register(s5);
#endif

#if NEEDS_BASE_COLOR_MAP
Texture2D g_tex2d_base_color : register(t6);
#endif

#if NEEDS_NORMAL_MAP
Texture2D g_tex2d_normal : register(t7);
#endif

#if NEEDS_METALLIC_ROUGHNESS_MAP
Texture2D g_tex2d_metallic_roughness : register(t8);
#endif

#if IBL_PASS
TextureCube g_ibl_cubemap : register(t9);
#else // IBL_PASS
#if NEEDS_OCCLUSION_MAP
Texture2D g_tex2d_occlusion : register(t9);
#endif

#if NEEDS_EMISSIVE_MAP
Texture2D g_tex2d_emissive : register(t10);
#endif
#endif // else IBL_PASS

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

	// texcoord
#if VERTEX_HAS_TEXCOORD && NEEDS_TEXTURE_COORD
	output.texcoord = input.texcoord;
#endif

#if HITMASK_PASS
	// hitmask id
	#if INSTANCING
		output.hitmask_id = g_hitmask_id[input.instance_id];
	#else
		output.hitmask_id = g_hitmask_id;
	#endif
#elif !UNLIT_PASS && !EMISSIVE_PASS
	// world position
	#if !DEFERRED_PASS
		output.world_position = world_position;
	#endif

	// normal
	#if VERTEX_HAS_NORMAL
		output.normal = normalize(mul(input.normal, transform));
	#endif

	// tangent, binormal
	#if VERTEX_HAS_TANGENT
		output.tangent = normalize(mul(input.tangent, transform));
		output.binormal = cross(output.normal, output.tangent);
	#endif
#endif // !UNLIT_PASS

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
#if VERTEX_HAS_TEXCOORD && NEEDS_TEXTURE_COORD
	float2 texcoord = input.texcoord;
#else
	float2 texcoord = 0.0f;
#endif

	// sample base color texture
#if NEEDS_BASE_COLOR_MAP
	float4 base_color_sample = g_tex2d_base_color.Sample(g_sampler, texcoord);
#endif

#if NEEDS_MATERIAL_BUFFER
	#if INSTANCING
		Material material = g_material[input.instance_id];
	#else
		Material material = g_material;
	#endif
#endif

	// perform alpha test
#if ALPHA_TEST
	float alpha = base_color_sample.a;
	if (alpha < material.alpha_cutoff)
	{
		clip(-1);
	}
#endif // ALPHA_TEST

	// sample emissive texture
#if NEEDS_EMISSIVE_MAP
	float3 emissive = g_tex2d_emissive.Sample(g_sampler, texcoord).rgb *
		material.emissive_strength;
#endif

	// process appropriate render pass
#if HITMASK_PASS
	output = input.hitmask_id;
#elif UNLIT_PASS
	output.rgb = ApproxRgb2Srgb(base_color_sample.rgb) * material.base_color_factor.rgb;
	output.a = base_color_sample.a * material.base_color_factor.a;
#elif EMISSIVE_PASS
	output.rgb = emissive;
	output.a = 1.0f;
#else // not HITMASK_PASS and not UNLIT_PASS
	// sample normal map
#if NEEDS_NORMAL_MAP
	float4 normal_sample = g_tex2d_normal.Sample(g_sampler, texcoord);
#else
	float4 normal_sample = float4(0.5f, 0.5f, 1.0f, 1.0f);
#endif

	// sample metallic-roughness map
#if NEEDS_METALLIC_ROUGHNESS_MAP
	float4 metallic_roughness_sample = g_tex2d_metallic_roughness.Sample(g_sampler, texcoord);
#else
	float4 metallic_roughness_sample = float4(0.0f, 1.0f, 0.0f, 0.0f);
#endif

	// calculate material data
	float3 base_color = base_color_sample.rgb * material.base_color_factor.rgb;
	float3 normal_color = normal_sample.rgb;
	float roughness = metallic_roughness_sample.g * material.roughness_factor;
	float metallic = metallic_roughness_sample.b * material.metallic_factor;

	// calculate final normal
	#if VERTEX_HAS_NORMAL && VERTEX_HAS_TANGENT
		float3x3 world_to_tangent = float3x3(input.tangent, input.binormal, input.normal);
		float3 scaled_normal = float3(material.normal_scale, material.normal_scale, 1.0f) *
		                       (2.0f * normal_color - 1.0f);
		float3 tbn_normal = normalize(scaled_normal);
		float3 normal = normalize(mul(tbn_normal, world_to_tangent));
	#elif VERTEX_HAS_NORMAL
		float3 normal = input.normal + normal_sample.a;
	#else
		float3 normal = normal_sample.rgb;
	#endif

	// calculate occlusion factor
	#if NEEDS_OCCLUSION_MAP
		float occlusion = g_tex2d_occlusion.Sample(g_sampler, texcoord).r *
		                  material.occlusion_strength;
	#endif

	#if DEFERRED_PASS
		output = EncodeGBuffer(normal,
		                       base_color,
		                       emissive,
		                       roughness,
		                       metallic,
		                       occlusion);
	#else
		SurfaceData surface;
		surface.world_position = input.world_position;
		surface.look = normalize(g_camera_position.xyz - surface.world_position);
		surface.normal = normalize(normal);
		surface.diffuse = base_color;
		surface.specular = 0.5f;
		surface.roughness = roughness;
		surface.min_roughness = 0.001f;
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
			#if AMBIENT_LIGHT
				float3 light_amount = ComputeAmbientLighting(surface, light) * occlusion;
			#else
				float3 light_amount = ComputeDirectLighting(surface, light);
			#endif
			light_amount *= ComputeAttenuation(light);
		#elif IBL_PASS
			float3 view_direction = -surface.look;
			float3 reflection_vector = normalize(reflect(view_direction, surface.normal));
			float NoV = saturate(dot(surface.normal, surface.look));
			float3 light_amount = GetImageBasedReflectionLighting(g_ibl_cubemap,
			                                                      g_sampler,
			                                                      surface.roughness,
			                                                      reflection_vector,
			                                                      NoV,
			                                                      surface.specular,
			                                                      IBL_MIP_COUNT);
		#endif // LIGHT_PASS
		output.rgb = light_amount;
		output.a = base_color_sample.a * material.base_color_factor.a;
	#endif // !DEFERRED_PASS
#endif // !HITMASK_PASS

	return output;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//