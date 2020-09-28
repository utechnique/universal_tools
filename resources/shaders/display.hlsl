
cbuffer g_ub_display_quad : register(b0)
{
	float4 g_color;
};

Texture2D g_tex2d : register(t1);
SamplerState g_sampler : register(s2);

struct VS_INPUT
{
	float2 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct PS_INPUT 
{
	float4 position  : SV_POSITION;
	float2 texcoord  : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	output.position = float4(input.position.xy, 0.0f, 1.0f);
	output.texcoord = input.texcoord;
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return g_tex2d.Sample(g_sampler, input.texcoord) * g_color;
}