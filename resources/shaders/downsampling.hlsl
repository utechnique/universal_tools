//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Shader resources.
cbuffer g_ub_downsampling : register(b0)
{
	float4 g_parameters;
};

Texture2D	 g_tex2d : register(t1);
SamplerState g_sampler : register(s3);

//----------------------------------------------------------------------------//
// Pixel shader input structure.
struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

//----------------------------------------------------------------------------//
// Pixel shader entry.
float4 PS(PS_INPUT input) : SV_Target
{
	float3 col = 0.0f;
	
	input.texcoord *= g_parameters.zw;

	float2 offset_scale = g_parameters.xy * g_parameters.zw;

#if GRANULARITY_2x2
#if FILTER_BOX
	col += 0.25 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-0.5, -0.5) * offset_scale).rgb;
	col += 0.25 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0.5, -0.5) * offset_scale).rgb;
	col += 0.25 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0.5, 0.5) * offset_scale).rgb;
	col += 0.25 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-0.5, 0.5) * offset_scale).rgb;
#elif FILTER_BILINEAR
	col += 0.37487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-0.75777, -0.75777) * offset_scale).rgb;
	col += 0.37487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0.75777, -0.75777) * offset_scale).rgb;
	col += 0.37487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0.75777, 0.75777) * offset_scale).rgb;
	col += 0.37487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-0.75777, 0.75777) * offset_scale).rgb;

	col += -0.12487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-2.907, 0.0) * offset_scale).rgb;
	col += -0.12487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(2.907, 0.0) * offset_scale).rgb;
	col += -0.12487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0.0, -2.907) * offset_scale).rgb;
	col += -0.12487566 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0.0, 2.907) * offset_scale).rgb;
#endif
#elif GRANULARITY_3x3
#if FILTER_BOX
	float weight = 1.0 / 9.0;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(1, 0) * offset_scale).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(1, 1) * offset_scale).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(0, 1) * offset_scale).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(-1, 1) * offset_scale).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(-1, 0) * offset_scale).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(-1, -1) * offset_scale).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(1, -1) * offset_scale).rgb;
	col += weight * g_tex2d.Sample(g_sampler, input.texcoord + float2(0, -1) * offset_scale).rgb;
#elif FILTER_BILINEAR
	col += 0.18 * g_tex2d.Sample(g_sampler, input.texcoord).rgb;

	col += 0.065 * g_tex2d.Sample(g_sampler, input.texcoord + float2(1, 0) * offset_scale).rgb;
	col += 0.065 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0, 1) * offset_scale).rgb;
	col += 0.065 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0, -1) * offset_scale).rgb;
	col += 0.065 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-1, 0) * offset_scale).rgb;

	col += 0.055 * g_tex2d.Sample(g_sampler, input.texcoord + float2(1, 1) * offset_scale).rgb;
	col += 0.055 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-1, 1) * offset_scale).rgb;
	col += 0.055 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-1, -1) * offset_scale).rgb;
	col += 0.055 * g_tex2d.Sample(g_sampler, input.texcoord + float2(1, -1) * offset_scale).rgb;

	col += 0.05 * g_tex2d.Sample(g_sampler, input.texcoord + float2(2, 0) * offset_scale).rgb;
	col += 0.05 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-2, 0) * offset_scale).rgb;
	col += 0.05 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0, 2) * offset_scale).rgb;
	col += 0.05 * g_tex2d.Sample(g_sampler, input.texcoord + float2(0, -2) * offset_scale).rgb;

	col += 0.035 * g_tex2d.Sample(g_sampler, input.texcoord + float2(2, 2) * offset_scale).rgb;
	col += 0.035 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-2, 2) * offset_scale).rgb;
	col += 0.035 * g_tex2d.Sample(g_sampler, input.texcoord + float2(-2, -2) * offset_scale).rgb;
	col += 0.035 * g_tex2d.Sample(g_sampler, input.texcoord + float2(2, -2) * offset_scale).rgb;
#endif
#endif	

	return float4(col, 1.0f);

}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//