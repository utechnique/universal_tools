//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Post-processing highlighting techniques.
//----------------------------------------------------------------------------//
// Width of the animated lines in pixels.
#ifndef LINE_WIDTH
#define LINE_WIDTH 2
#endif
#define LINE_HALF_WIDTH (LINE_WIDTH / 2)

// Distance between animated lines in pixels.
#ifndef LINE_DISTANCE
#define LINE_DISTANCE 20
#endif

//----------------------------------------------------------------------------//
// Pixel shader vertex input format.
struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

//----------------------------------------------------------------------------//
// Multi-purpose 4d uniform.
cbuffer g_ub_highlight_color : register(b0)
{
	float4 g_highlight_color;
}

// Original screen texture.
Texture2D<float4> g_input_texture : register(t1);

// Highlighting mask to blend with @g_input_texture.
Texture2D<float4> g_highlight_texture : register(t2);

// Sampler for blending.
SamplerState g_sampler : register(s3);

//----------------------------------------------------------------------------//
// Just fills a surface with solid color.
float4 FillStencilPS(PS_INPUT input) : SV_Target
{
	return g_highlight_color;
}

// Draws black background with greyscale animated lined.
float4 DrawLinesPS(PS_INPUT input) : SV_Target
{
    float2 resolution = g_highlight_color.xy;
	float line_visibility = g_highlight_color.z;
    float movement_offset = g_highlight_color.w;
    float2 pos = input.texcoord * resolution;
    float line_id = (pos.x + pos.y + movement_offset) / LINE_DISTANCE;
    float dist = abs(frac(line_id) - 0.5f) * LINE_DISTANCE;
    float bline = dist < LINE_HALF_WIDTH ? 1.0f : 0.0f;
    return bline > 0 ? line_visibility : 0.0f;
}

// Blends scene with highlighting mask.
float4 BlendPS(PS_INPUT input) : SV_Target
{
	float4 source_sample = g_input_texture.Sample(g_sampler, input.texcoord);
	float4 highlight_sample = g_highlight_texture.Sample(g_sampler, input.texcoord);
	return lerp(source_sample, highlight_sample * g_highlight_color, highlight_sample.a);
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//