
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
	output.position = float4(input.position.xy, 0.5f, 1.0f);
	output.position.xy *= 0.5f;
	output.texcoord = input.texcoord;
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return float4(1, 0, 0, 1);
}