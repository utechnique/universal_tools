//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Binding indices for shader resources included from separate files.
#define UB_ID_VIEW 0

//----------------------------------------------------------------------------//
// Define 'ALPHA_TEST' macro to enable or disable alpha test.
#ifndef ALPHA_TEST
#define ALPHA_TEST 0
#endif

//----------------------------------------------------------------------------//
#include "view_buffer.hlsl"
#include "vertex.hlsl"

//----------------------------------------------------------------------------//
// VS -> PS data.
struct PS_INPUT 
{
	float4 position : SV_POSITION;

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

	uint instance_id : TEXCOORD4;
};

// Pixel shader output.
struct PS_OUTPUT
{
	float4 diffuse : COLOR0;
	float4 normal : COLOR1;
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
cbuffer g_ub_transform : register(b1)
{
	row_major float4x4 g_transform[BATCH_SIZE];
};

cbuffer g_ub_material : register(b2)
{
	Material g_material[BATCH_SIZE];
};

SamplerState g_sampler : register(s3);
Texture2D g_tex2d_diffuse : register(t4);
Texture2D g_tex2d_normal : register(t5);
Texture2D g_tex2d_material : register(t6);

//----------------------------------------------------------------------------//
// Vertex shader entry.
PS_INPUT VS(Vertex input)
{
	PS_INPUT output;

	// position
#if VERTEX_2D_POSITION
	float3 position_3d = float3(input.position.xy, 0.0f);
#else
	float3 position_3d = input.position.xyz;
#endif
	float3 world_position = mul(float4(position_3d, 1.0f), g_transform[input.instance_id]);
	output.position = mul(float4(world_position, 1.0f), g_view_proj);

	// texcoord
#if VERTEX_HAS_TEXCOORD
	output.texcoord = input.texcoord;
#endif

	// normal
#if VERTEX_HAS_NORMAL
	output.normal = mul(input.normal, g_transform[input.instance_id]);
#endif

	// tangent, binormal
#if VERTEX_HAS_TANGENT
	output.tangent = mul(input.tangent, g_transform[input.instance_id]);
	output.binormal = cross(output.normal, output.tangent);
#endif

	// instance id
	output.instance_id = input.instance_id;

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
	Material material = g_material[input.instance_id];
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
	output.diffuse = float4(diffuse_color.rgb, roughness);
	output.normal = float4(normal, metallic);

	return output;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//