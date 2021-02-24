//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Preprocessor macros.
#define FxaaSel3(f, t, b) ((f)*(!b) + (t)*(b))
#define FxaaToFloat3(a) float3((a), (a), (a))

#ifndef FXAA_PRESET
    #define FXAA_PRESET 5
#endif

#if (FXAA_PRESET == 0)
    #define FXAA_EDGE_THRESHOLD      (1.0/4.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/12.0)
    #define FXAA_SEARCH_STEPS        2
    #define FXAA_SEARCH_ACCELERATION 4
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       1
    #define FXAA_SUBPIX_CAP          (2.0/3.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif

#if (FXAA_PRESET == 1)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/16.0)
    #define FXAA_SEARCH_STEPS        4
    #define FXAA_SEARCH_ACCELERATION 3
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif

#if (FXAA_PRESET == 2)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        8
    #define FXAA_SEARCH_ACCELERATION 2
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif

#if (FXAA_PRESET == 3)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        16
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif

#if (FXAA_PRESET == 4)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        24
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif

#if (FXAA_PRESET == 5)
    #define FXAA_EDGE_THRESHOLD      (1.0/8.0)
    #define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
    #define FXAA_SEARCH_STEPS        32
    #define FXAA_SEARCH_ACCELERATION 1
    #define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
    #define FXAA_SUBPIX              1
    #define FXAA_SUBPIX_FASTER       0
    #define FXAA_SUBPIX_CAP          (3.0/4.0)
    #define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif

#define FXAA_SUBPIX_TRIM_SCALE ( 1.0/ (1.0 - FXAA_SUBPIX_TRIM) )

//----------------------------------------------------------------------------//
// Shader resources.
cbuffer g_ub_fxaa : register(b0)
{
	float4 g_texel_size;
};

Texture2D	 g_tex2d : register(t1);
SamplerState g_sampler : register(s2);

//----------------------------------------------------------------------------//
// Pixel shader input structure.
struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

struct FxaaSrcImage
{
	SamplerState smpl;
	Texture2D tex;
};

//----------------------------------------------------------------------------//
// FXAA functions.
float4 FxaaTexLod0(FxaaSrcImage tex, float2 pos)
{
    return tex.tex.SampleLevel(tex.smpl, pos.xy, 0.0);
}

float4 FxaaTexGrad(FxaaSrcImage tex, float2 pos, float2 grad)
{
    return tex.tex.SampleGrad(tex.smpl, pos.xy, grad, grad);
}

float4 FxaaTexOff(FxaaSrcImage tex, float2 pos, int2 off, float2 texel_size)
{
    return tex.tex.SampleLevel(tex.smpl, pos.xy, 0.0, off.xy);
}

float FxaaLuma(float3 rgb)
{
	return sqrt(dot(rgb, float3(0.299, 0.587, 0.114)));
} 

float3 FxaaLerp3(float3 a, float3 b, float amount_of_a)
{
    return (FxaaToFloat3(-amount_of_a) * b) + ((a * FxaaToFloat3(amount_of_a)) + b);
} 

//----------------------------------------------------------------------------//
// Applies anti-aliasing to the provided texel.
float3 Fxaa(FxaaSrcImage tex, float2 pos,  float2 texel_size)
{
    float3 rgbN = FxaaTexOff(tex, pos.xy, int2( 0,-1), texel_size).xyz;
    float3 rgbW = FxaaTexOff(tex, pos.xy, int2(-1, 0), texel_size).xyz;
    float3 rgbM = FxaaTexOff(tex, pos.xy, int2( 0, 0), texel_size).xyz;
    float3 rgbE = FxaaTexOff(tex, pos.xy, int2( 1, 0), texel_size).xyz;
    float3 rgbS = FxaaTexOff(tex, pos.xy, int2( 0, 1), texel_size).xyz;
    float lumaN = FxaaLuma(rgbN);
    float lumaW = FxaaLuma(rgbW);
    float lumaM = FxaaLuma(rgbM);
    float lumaE = FxaaLuma(rgbE);
    float lumaS = FxaaLuma(rgbS);
    float range_min = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    float range_max = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
    float range = range_max - range_min;
      
    if( range < max( FXAA_EDGE_THRESHOLD_MIN, range_max * FXAA_EDGE_THRESHOLD ) )
        return rgbM;// R  E  T  U  R  N
    
    #if FXAA_SUBPIX > 0
        #if FXAA_SUBPIX_FASTER
            float3 rgbL = (rgbN + rgbW + rgbE + rgbS + rgbM) * 
                FxaaToFloat3(1.0/5.0);
        #else
            float3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
        #endif
    #endif        
    
    #if FXAA_SUBPIX != 0
        float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
        float rangeL = abs(lumaL - lumaM);
    #endif        
    #if FXAA_SUBPIX == 1
        float blendL = max( 0,(rangeL/range)-FXAA_SUBPIX_TRIM )*FXAA_SUBPIX_TRIM_SCALE; 
        blendL = min( FXAA_SUBPIX_CAP, blendL );
    #endif
    #if FXAA_SUBPIX == 2
        float blendL = rangeL / range; 
    #endif    
    
    float3 rgbNW = FxaaTexOff(tex, pos.xy, int2(-1,-1), texel_size).xyz;
    float3 rgbNE = FxaaTexOff(tex, pos.xy, int2( 1,-1), texel_size).xyz;
    float3 rgbSW = FxaaTexOff(tex, pos.xy, int2(-1, 1), texel_size).xyz;
    float3 rgbSE = FxaaTexOff(tex, pos.xy, int2( 1, 1), texel_size).xyz;
    #if (FXAA_SUBPIX_FASTER == 0) && (FXAA_SUBPIX > 0)
        rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
        rgbL *= FxaaToFloat3(1.0/9.0);
    #endif
    float lumaNW = FxaaLuma(rgbNW);
    float lumaNE = FxaaLuma(rgbNE);
    float lumaSW = FxaaLuma(rgbSW);
    float lumaSE = FxaaLuma(rgbSE);
    float edge_vert = abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
                      abs((0.50 * lumaW ) + (-1.0 * lumaM) + (0.50 * lumaE )) +
                      abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
    float edge_horz = abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
                      abs((0.50 * lumaN ) + (-1.0 * lumaM) + (0.50 * lumaS )) +
                      abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
    bool horz_span = edge_horz >= edge_vert;

    float length_sign = horz_span ? -texel_size.y : -texel_size.x;
    if(!horz_span) lumaN = lumaW;
    if(!horz_span) lumaS = lumaE;
    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);
    lumaN = (lumaN + lumaM) * 0.5;
    lumaS = (lumaS + lumaM) * 0.5;
    
    bool pairN = gradientN >= gradientS;

    if(!pairN)
        lumaN = lumaS;
    if(!pairN)
        gradientN = gradientS;
    if(!pairN)
		length_sign *= -1.0;
        
    float2 posN;
    posN.x = pos.x + (horz_span ? 0.0 : length_sign * 0.5);
    posN.y = pos.y + (horz_span ? length_sign * 0.5 : 0.0);
    
    gradientN *= FXAA_SEARCH_THRESHOLD;
    
    float2 posP = posN;
    float2 offNP = horz_span ? float2(texel_size.x, 0.0) : float2(0.0f, texel_size.y);
    float lumaEndN = lumaN;
    float lumaEndP = lumaN;
    bool doneN = false;
    bool doneP = false;
    #if FXAA_SEARCH_ACCELERATION == 1
        posN += offNP * float2(-1.0, -1.0);
        posP += offNP * float2( 1.0,  1.0);
    #endif
    #if FXAA_SEARCH_ACCELERATION == 2
        posN += offNP * float2(-1.5, -1.5);
        posP += offNP * float2( 1.5,  1.5);
        offNP *= float2(2.0, 2.0);
    #endif
    #if FXAA_SEARCH_ACCELERATION == 3
        posN += offNP * float2(-2.0, -2.0);
        posP += offNP * float2( 2.0,  2.0);
        offNP *= float2(3.0, 3.0);
    #endif
    #if FXAA_SEARCH_ACCELERATION == 4
        posN += offNP * float2(-2.5, -2.5);
        posP += offNP * float2( 2.5,  2.5);
        offNP *= float2(4.0, 4.0);
    #endif
    for(int i = 0; i < FXAA_SEARCH_STEPS; i++)
    {
        #if FXAA_SEARCH_ACCELERATION == 1
            if(!doneN)
                lumaEndN = FxaaLuma(FxaaTexLod0(tex, posN.xy).xyz);
            if(!doneP)
                lumaEndP = FxaaLuma(FxaaTexLod0(tex, posP.xy).xyz);
        #else
            if(!doneN)
                lumaEndN = FxaaLuma(FxaaTexGrad(tex, posN.xy, offNP).xyz);
            if(!doneP)
                lumaEndP = FxaaLuma(FxaaTexGrad(tex, posP.xy, offNP).xyz);
        #endif
        doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
        doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
        if(doneN && doneP)
            break;
        if(!doneN)
            posN -= offNP;
        if(!doneP)
            posP += offNP;
    }
    
    float dstN = horz_span ? pos.x - posN.x : pos.y - posN.y;
    float dstP = horz_span ? posP.x - pos.x : posP.y - pos.y;
    bool directionN = dstN < dstP;

    lumaEndN = directionN ? lumaEndN : lumaEndP;
    
    if(((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0)) 
		length_sign = 0.0;
 
    float span_length = (dstP + dstN);
    dstN = directionN ? dstN : dstP;
    float sub_pixel_offset = (0.5 + (dstN * (-1.0/span_length))) * length_sign;

    float3 rgbF = FxaaTexLod0(tex, float2(
        pos.x + (horz_span ? 0.0 : sub_pixel_offset),
        pos.y + (horz_span ? sub_pixel_offset : 0.0))).xyz;
    #if FXAA_SUBPIX == 0
        return rgbF;// R  E  T  U  R  N
    #else        
        return FxaaLerp3(rgbL, rgbF, blendL);// R  E  T  U  R  N 
    #endif
}

//----------------------------------------------------------------------------//
// Pixel shader entry.
float4 PS(PS_INPUT input) : SV_TARGET
{
	FxaaSrcImage img = { g_sampler, g_tex2d };
	return float4(Fxaa(img, input.texcoord.xy, g_texel_size.xy), 1.0f );
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
