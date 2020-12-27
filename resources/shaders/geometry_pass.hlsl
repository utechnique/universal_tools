
struct VS_INPUT
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

struct PS_INPUT 
{
	float4 position  : SV_POSITION;
	float2 texcoord  : TEXCOORD0;
};

cbuffer g_ub_view : register(b0)
{
	row_major float4x4 g_view_proj;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	output.position = mul(float4(input.position.xyz, 1.0f), g_view_proj);
	output.texcoord = input.texcoord;
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return float4(1, 0, 0, 1);
}