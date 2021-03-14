
struct VS_INPUT
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	uint instance_id : INSTANCE_ID;
};

struct PS_INPUT 
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : TEXCOORD1;
	uint instance_id : TEXCOORD2;
};

struct PS_OUTPUT
{
	float4 diffuse : COLOR0;
	float4 normal : COLOR1;
};

struct Material
{
	float4 diffuse_add;
	float4 diffuse_mul;
	float4 material_add;
	float4 material_mul;
};

cbuffer g_ub_view : register(b0)
{
	row_major float4x4 g_view_proj;
};

cbuffer g_ub_transform : register(b1)
{
	row_major float4x4 g_transform[BATCH_SIZE];
};

cbuffer g_ub_material : register(b2)
{
	Material g_material[BATCH_SIZE];
};

SamplerState g_sampler : register(s3);
Texture2D g_tex2d_diffuse : register(t4);
Texture2D g_tex2d_normal : register(t5);
Texture2D g_tex2d_material : register(t6);


PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	float3 world_position = mul(float4(input.position.xyz, 1.0f), g_transform[input.instance_id]);
	output.position = mul(float4(world_position, 1.0f), g_view_proj);
	output.texcoord = input.texcoord;
	output.normal = input.normal;
	output.instance_id = input.instance_id;
	return output;
}

PS_OUTPUT PS(PS_INPUT input) : SV_Target
{
	PS_OUTPUT output;

	Material material = g_material[input.instance_id];

	float3 diffuse_color = g_tex2d_diffuse.Sample(g_sampler, input.texcoord).rgb * material.diffuse_mul.rgb + material.diffuse_add.rgb;
	float3 normal_color = g_tex2d_normal.Sample(g_sampler, input.texcoord).rgb;
	float3 material_color = g_tex2d_material.Sample(g_sampler, input.texcoord).rgb * material.material_mul.rgb + material.material_add.rgb;
	float roughness = material_color.r;
	float albedo = material_color.g;

	output.diffuse = float4(diffuse_color.rgb, roughness);
	output.normal = float4(normal_color.xyz, albedo);

	return output;
}