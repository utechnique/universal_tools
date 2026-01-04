//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Shader resources.
cbuffer g_ub_gradient_dithering : register(b0)
{
	float4 g_parameters;
};

Texture2D	 g_tex2d : register(t1);
Texture2D	 g_tex2d_lut : register(t2);
SamplerState g_sampler : register(s3);
SamplerState g_sampler_lut : register(s4);

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
	float4 original_color = g_tex2d.Sample(g_sampler, input.texcoord);

	// initialize parameters (width, height, mip)
	float2 img_size = float2(g_parameters.x, g_parameters.y);
	float lut_size = g_parameters.z;
	float lut_mip = g_parameters.w;

	// sample dithering lookup texture
	float2 lut_texcoord = input.texcoord * img_size / lut_size;
	float2 offset = g_tex2d_lut.SampleLevel(g_sampler_lut, lut_texcoord, lut_mip);

	// calculate the accurate offset
	offset = offset * 2.0f - 1.0f;
	offset *= 0.5f;
	offset *= lut_size;
	offset /= img_size;

	// sample the displaced color and calculate the difference
	float4 displaced_color = g_tex2d.Sample(g_sampler, input.texcoord + offset);
	float3 color_difference = abs(displaced_color.rgb - original_color.rgb);
	float max_difference = max(max(color_difference.r,
	                               color_difference.g),
	                           color_difference.b);

	// darker areas are allowed to have a larger displacement radius
	float brightness = dot(original_color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
	float brightness_weight = 1.0f - pow(length(brightness), 4.0f);

	// return displaced color if it exceeds the threshold
	float threshold = lerp(1.0, 2.5, brightness_weight) / 255.0;
	if (max_difference <= threshold)
	{
		return displaced_color;
	}

	return original_color;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//