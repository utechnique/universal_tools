#ifndef GAUSSIAN_BLUR_RADIUS
#define GAUSSIAN_BLUR_RADIUS 5
#endif

#define GAUSSIAN_BLUR_KERNEL_SIZE (GAUSSIAN_BLUR_RADIUS * 2 + 1)

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

cbuffer g_ub_blur_weights : register(b0)
{
    float4 g_gaussian_blur_weights[GAUSSIAN_BLUR_KERNEL_SIZE];
}

Texture2D<float4> g_input_texture : register(t1);
SamplerState g_linear_sampler : register(s2);

float4 GaussianBlurPS(PS_INPUT input) : SV_TARGET
{
    float4 final_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < GAUSSIAN_BLUR_KERNEL_SIZE; ++i)
    {
        float4 weight = g_gaussian_blur_weights[i];
        float4 sample_color = g_input_texture.Sample(g_linear_sampler, input.texcoord + weight.xy);
        final_color += weight.z * sample_color;
    }

    return final_color;
}