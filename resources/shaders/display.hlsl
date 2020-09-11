
cbuffer g_cb_display : register(b1)
{
	float4 g_color;
};

Texture1D g_tex1d : register(t0);
Texture2D g_tex2d : register(t1);
TextureCube g_tex_cube : register(t3);
Texture3D g_tex3d : register(t4);
Texture2D g_tex_cube_face[6] : register(t5);
SamplerState g_sampler : register(s0);

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
	//float4 final_color = g_color;
	float4 final_color = g_color;
	
	if (input.texcoord.x < 0.25 && input.texcoord.y < 0.25) // 2d texture
	{
		final_color = g_tex2d.SampleLevel(g_sampler, input.texcoord * 4, 0.0f);
	}
	else if (input.texcoord.x > 0.25 && input.texcoord.x < 0.5 && input.texcoord.y < 0.25) // 2d texture mip
	{
		input.texcoord.x -= 0.25;
		final_color = g_tex2d.SampleLevel(g_sampler, input.texcoord * 4, 6.0f);
	}
	else if (input.texcoord.x > 0.5 && input.texcoord.y < 0.25) // 1d texture
	{
		input.texcoord.x -= 0.5;
		final_color = g_tex1d.SampleLevel(g_sampler, input.texcoord.x * 2, 0.0f);
	}
	else if (input.texcoord.y >= 0.25 && input.texcoord.y <= 0.625 && input.texcoord.x < 0.5) // cubemap
	{
		float3 ct = 0;
		if (input.texcoord.y <= 0.4375)
		{
			input.texcoord.y -= 0.25;
			input.texcoord.y /= 0.1875;
			if (input.texcoord.x < 0.166) // -x
			{
				input.texcoord.x -= 0;
				input.texcoord.x /= 0.166;
				ct = float3(-1, input.texcoord.x*2 - 1, input.texcoord.y*2 - 1);
			}
			else if (input.texcoord.x < 0.333) // -y
			{
				input.texcoord.x -= 0.166;
				input.texcoord.x /= 0.166;
				ct = float3(input.texcoord.x * 2 - 1 , -1, input.texcoord.y * 2 - 1);
			}
			else // -z
			{
				input.texcoord.x -= 0.333;
				input.texcoord.x /= 0.166;
				ct = float3(-input.texcoord.x * 2 + 1, input.texcoord.y * 2 - 1, -1);
			}
		}
		else
		{
			input.texcoord.y -= 0.4375;
			input.texcoord.y /= 0.1875;
			if (input.texcoord.x < 0.166) // +x
			{
				input.texcoord.x -= 0;
				input.texcoord.x /= 0.166;
				ct = float3(1, input.texcoord.x * 2 - 1, input.texcoord.y * 2 - 1);
			}
			else if (input.texcoord.x < 0.333) // +y
			{
				input.texcoord.x -= 0.166;
				input.texcoord.x /= 0.166;
				ct = float3(input.texcoord.x * 2 - 1, 1, input.texcoord.y * 2 - 1);
			}
			else // +z
			{
				input.texcoord.x -= 0.333;
				input.texcoord.x /= 0.166;
				ct = float3(input.texcoord.x * 2 - 1, -input.texcoord.y * 2 + 1, 1);
			}
		}
		final_color = g_tex_cube.SampleLevel(g_sampler, normalize(ct), 0.0f);
	}
	else if (input.texcoord.y >= 0.625 && input.texcoord.x < 0.5) // cubemap
	{
		float3 ct = 0;
		if (input.texcoord.y <= 0.8125)
		{
			input.texcoord.y -= 0.625;
			input.texcoord.y /= 0.1875;
			if (input.texcoord.x < 0.166) // -x
			{
				input.texcoord.x -= 0;
				input.texcoord.x /= 0.166;
				ct = float3(-1, input.texcoord.x * 2 - 1, input.texcoord.y * 2 - 1);
			}
			else if (input.texcoord.x < 0.333) // -y
			{
				input.texcoord.x -= 0.166;
				input.texcoord.x /= 0.166;
				ct = float3(input.texcoord.x * 2 - 1, -1, input.texcoord.y * 2 - 1);
			}
			else // -z
			{
				input.texcoord.x -= 0.333;
				input.texcoord.x /= 0.166;
				ct = float3(-input.texcoord.x * 2 + 1, input.texcoord.y * 2 - 1, -1);
			}
		}
		else
		{
			input.texcoord.y -= 0.8125;
			input.texcoord.y /= 0.1875;
			if (input.texcoord.x < 0.166) // +x
			{
				input.texcoord.x -= 0;
				input.texcoord.x /= 0.166;
				ct = float3(1, input.texcoord.x * 2 - 1, input.texcoord.y * 2 - 1);
			}
			else if (input.texcoord.x < 0.333) // +y
			{
				input.texcoord.x -= 0.166;
				input.texcoord.x /= 0.166;
				ct = float3(input.texcoord.x * 2 - 1, 1, input.texcoord.y * 2 - 1);
			}
			else // +z
			{
				input.texcoord.x -= 0.333;
				input.texcoord.x /= 0.166;
				ct = float3(input.texcoord.x * 2 - 1, -input.texcoord.y * 2 + 1, 1);
			}
		}
		final_color = g_tex_cube.SampleLevel(g_sampler, normalize(ct), 6.0f);
	}
	else if (input.texcoord.y >= 0.25 && input.texcoord.y <= 0.625 && input.texcoord.x > 0.5 && input.texcoord.x < 0.75) // cubemap
	{
		input.texcoord.x -= 0.5;
		input.texcoord.x *= 4;
		input.texcoord.y -= 0.25;
		input.texcoord.y /= 0.375;

		final_color = g_tex3d.SampleLevel(g_sampler, float3(input.texcoord, 0), 0.0f);
	}
	else if (input.texcoord.y >= 0.25  && input.texcoord.y <= 0.625 && input.texcoord.x > 0.75) // 3d texture
	{
		input.texcoord.x -= 0.75;
		input.texcoord.x *= 4;
		input.texcoord.y -= 0.25;
		input.texcoord.y /= 0.375;

		final_color = g_tex3d.SampleLevel(g_sampler, float3(input.texcoord, 1), 0.0f);
	}
	else if (input.texcoord.y >= 0.625 && input.texcoord.x >= 0.5) // cube face as individual
	{
		if (input.texcoord.y <= 0.8125)
		{
			input.texcoord.y -= 0.625;
			input.texcoord.y /= 0.1875;
			if (input.texcoord.x < 0.666) // -x
			{
				input.texcoord.x -= 0.5;
				input.texcoord.x /= 0.166;
				final_color = g_tex_cube_face[1].SampleLevel(g_sampler, input.texcoord, 0.0f);
			}
			else if (input.texcoord.x < 0.833) // -y
			{
				input.texcoord.x -= 0.666;
				input.texcoord.x /= 0.166;
				final_color = g_tex_cube_face[3].SampleLevel(g_sampler, input.texcoord, 0.0f);
			}
			else // -z
			{
				input.texcoord.x -= 0.833;
				input.texcoord.x /= 0.166;
				final_color = g_tex_cube_face[5].SampleLevel(g_sampler, input.texcoord, 0.0f);
			}
		}
		else
		{
			input.texcoord.y -= 0.8125;
			input.texcoord.y /= 0.1875;
			if (input.texcoord.x < 0.666) // +x
			{
				input.texcoord.x -= 0.5;
				input.texcoord.x /= 0.166;
				final_color = g_tex_cube_face[0].SampleLevel(g_sampler, input.texcoord, 0.0f);
			}
			else if (input.texcoord.x < 0.833) // +y
			{
				input.texcoord.x -= 0.666;
				input.texcoord.x /= 0.166;
				final_color = g_tex_cube_face[2].SampleLevel(g_sampler, input.texcoord, 0.0f);
			}
			else // +z
			{
				input.texcoord.x -= 0.833;
				input.texcoord.x /= 0.166;
				final_color = g_tex_cube_face[4].SampleLevel(g_sampler, input.texcoord, 0.0f);
			}
		}
	}
	else
	{
		final_color = g_color;
	}

	return final_color;
}