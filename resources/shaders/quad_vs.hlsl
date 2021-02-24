//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//

struct VS_INPUT
{
	float2 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT_PS_INPUT
{
	float4 position  : SV_POSITION;
	float2 texcoord  : TEXCOORD0;
};

//----------------------------------------------------------------------------//
// Vertex shader entry point.
VS_OUTPUT_PS_INPUT VS(VS_INPUT input)
{
	VS_OUTPUT_PS_INPUT output;
	output.position = float4(input.position.xy, 0.0f, 1.0f);
	output.texcoord = input.texcoord;
	return output;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//