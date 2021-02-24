//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// This shader draws textured quad.
//----------------------------------------------------------------------------//
// Enable RGB_TO_SRGB macro to convert rgb to srgb.
#ifndef RGB_TO_SRGB
	#define RGB_TO_SRGB 0
#endif

// Enable SRGB_TO_RGB macro to convert srgb to rgb.
#ifndef SRGB_TO_RGB
	#define SRGB_TO_RGB 0
#endif

//----------------------------------------------------------------------------//
// Resources.
cbuffer g_ub_display_quad : register(b0)
{
	float4 g_color; // this color is multiplied with the sampled texture color.
};
Texture2D g_tex2d : register(t1);
SamplerState g_sampler : register(s2);

//----------------------------------------------------------------------------//

struct PS_INPUT 
{
	float4 position  : SV_POSITION;
	float2 texcoord  : TEXCOORD0;
};

//----------------------------------------------------------------------------//

float4 PS(PS_INPUT input) : SV_Target
{
	float4 c = g_tex2d.Sample(g_sampler, input.texcoord) * g_color;
#if RGB_TO_SRGB
	return pow(c, 2.2f);
#elif SRGB_TO_RGB
	return pow(c, 1.0f / 2.2f);
#else
	return c;
#endif
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//