//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// This shader performs IBL filtering.
//----------------------------------------------------------------------------//
#include "preprocessor.hlsl"

//----------------------------------------------------------------------------//
// MIP_COUNT macro value must match the number of mips in the ibl cubemap to be
// filtered.
#ifndef MIP_COUNT
#define MIP_COUNT 8
#endif

//----------------------------------------------------------------------------//
#define NUM_FILTER_SAMPLES 16
#define REFLECTION_MAX_BRIGHTNESS 15.0

//----------------------------------------------------------------------------//
struct VS_INPUT
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

struct PS_INPUT
{
	float4 position	 : SV_POSITION;
	float3 view_direction : TEXCOORD0;
};

//----------------------------------------------------------------------------//
// Shader resources
cbuffer g_filter_ub : register(b0)
{
	row_major float4x4 g_ibl_view_proj;
	float4 g_filter; //.x - source mip id .y - environment brightness
};

SamplerState g_sampler : register(s1);
TextureCube g_texcube_scene : register(t2);

//----------------------------------------------------------------------------//
float ComputeReflectionCaptureRoughnessFromMip(float mip, float mip_count)
{
	const float roughest_mip = 1.0f;
	const float roughness_mip_scale = 1.2f;
	float level_from_1x1 = mip_count - 1 - mip;
	return exp2((roughest_mip - level_from_1x1) / roughness_mip_scale);
}

uint ReverseBits32(uint bits)
{
	return reversebits(bits);
}

float2 Hammersley(uint index, uint sample_count, uint2 random)
{
	float E1 = frac((float)index / sample_count + float(random.x & 0xffff) / (1 << 16));
	float E2 = float(ReverseBits32(index) ^ random.y) * 2.3283064365386963e-10;
	return float2(E1, E2);
}

float4 ImportanceSampleGGX(float2 E, float roughness)
{
	float m = roughness * roughness;
	float m2 = m * m;

	float phi = 2 * PI * E.x;
	float cos_theta = sqrt((1 - E.y) / (1 + (m2 - 1) * E.y));
	float sin_theta = sqrt(1 - cos_theta * cos_theta);

	float3 H;
	H.x = sin_theta * cos(phi);
	H.y = sin_theta * sin(phi);
	H.z = cos_theta;

	float d = (cos_theta * m2 - cos_theta) * cos_theta + 1;
	float D = m2 / (PI*d*d);
	float PDF = D * cos_theta;

	return float4(H, PDF);
}

float3 TangentToWorldZ(float3 vec, float3 tangent_z)
{
	float3 up = abs(tangent_z.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 tangent_x = normalize(cross(up, tangent_z));
	float3 tangent_y = cross(tangent_z, tangent_x);
	return tangent_x * vec.x + tangent_y * vec.y + tangent_z * vec.z;
}

//----------------------------------------------------------------------------//
// IBL filtering vertex shader.
PS_INPUT FilterVS(VS_INPUT input)
{
	PS_INPUT output;
	output.position = mul(float4(input.position.xyz, 1.0f), g_ibl_view_proj);
	output.view_direction = normalize(input.position.xyz);
	return output;
}

//----------------------------------------------------------------------------//
// IBL filtering pixel shader.
float4 FilterPS(PS_INPUT input) : SV_Target
{
	float3 CubeCoordinates = input.view_direction;

	float3 N = normalize(CubeCoordinates);
	float Roughness = ComputeReflectionCaptureRoughnessFromMip(g_filter.x, MIP_COUNT);

	float4 FilteredColor = 0;
	float Weight = 0;

	for (int i = 0; i < NUM_FILTER_SAMPLES; i++)
	{
		float2 E = Hammersley(i, NUM_FILTER_SAMPLES, 0);
		float3 H = TangentToWorldZ(ImportanceSampleGGX(E, Roughness).xyz, N);
		float3 L = 2 * dot(N, H) * H - N;

		float NoL = saturate(dot(N, L));
		if (NoL > 0)
		{
			FilteredColor += max(min(REFLECTION_MAX_BRIGHTNESS, g_texcube_scene.SampleLevel(g_sampler, L, g_filter.x)), 0.0f) * NoL * g_filter.y;
			Weight += NoL;
		}
	}

	return abs(FilteredColor) / max(Weight, 0.001);
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//