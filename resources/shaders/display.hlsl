
cbuffer g_cb_display : register(b1)
{
	float4 g_color;
};

struct VS_INPUT
{
	float3 position : POSITION;
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
	output.position = float4(input.position.xyz, 1.0f);
	output.texcoord = input.texcoord;
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	float4 final_color = g_color;
	return final_color;
}