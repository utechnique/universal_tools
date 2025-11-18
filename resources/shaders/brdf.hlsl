//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// This file contains common models for diffuse, specular, fresnel, etc.
//----------------------------------------------------------------------------//
#include "preprocessor.hlsl"

//----------------------------------------------------------------------------//
// 0: Lambert
// 1: Burley
// 2: Oren-Nayar
#define LM_DIFFUSE        0

// 0: Blinn
// 1: Beckmann
// 2: GGX-Nayar
#define LM_DISTRIBUTION   2

// 0: Implicit
// 1: Neumann
// 2: Kelemen
// 3: Schlick
// 4: Smith
#define LM_SPEC_G         3

// 0: None
// 1: Schlick
// 2: Fresnel
#define LM_FRESNEL        1

//----------------------------------------------------------------------------//
// Diffuse.
//----------------------------------------------------------------------------//
float3 Diffuse_Lambert(float3 diffuse_color)
{
	return diffuse_color * (1 / PI);
}

float3 Diffuse_Burley(float3 diffuse_color, float roughness, float NoV, float NoL, float VoH)
{
	float FD90 = 0.5 + 2 * VoH * VoH * roughness;
	float FdV = 1 + (FD90 - 1) * pow(1 - NoV, 5);
	float FdL = 1 + (FD90 - 1) * pow(1 - NoL, 5);
	return diffuse_color * (1 / PI * FdV * FdL);
}

float3 Diffuse_OrenNayar(float3 diffuse_color, float roughness, float NoV, float NoL, float VoH)
{
	float VoL = 2 * VoH - 1;
	float m = roughness * roughness;
	float m2 = m * m;
	float C1 = 1 - 0.5 * m2 / (m2 + 0.33);
	float Cosri = VoL - NoV * NoL;
	float C2 = 0.45 * m2 / (m2 + 0.09) * Cosri * (Cosri >= 0 ? min(1, NoL / NoV) : NoL);
	return diffuse_color / PI * (NoL * C1 + C2);
}

//----------------------------------------------------------------------------//
// Specular distribution.
//----------------------------------------------------------------------------//
float Spec_Blinn(float roughness, float NoH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float n = 2 / m2 - 2;
	return (n + 2) / (2 * PI) * pow(max(abs(NoH), 0.000001f), n);
}

float Spec_Beckmann(float roughness, float NoH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float NoH2 = NoH * NoH;
	return exp((NoH2 - 1) / (m2 * NoH2)) / (PI * m2 * NoH2 * NoH2);
}

float Spec_GGX(float roughness, float NoH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float d = (NoH * m2 - NoH) * NoH + 1;
	return m2 / (PI*d*d);
}

//----------------------------------------------------------------------------//
// Attenuation (visibility).
//----------------------------------------------------------------------------//
float Vis_Implicit()
{
	return 0.25;
}

float Vis_Neumann(float NoV, float NoL)
{
	return 1 / (4 * max(NoL, NoV));
}

float Vis_Kelemen(float VoH)
{
	return rcp(4 * VoH * VoH);
}

float Vis_Schlick(float roughness, float NoV, float NoL)
{
	float k = SQUARE(roughness) * 0.5;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

float Vis_Smith(float roughness, float NoV, float NoL)
{
	float a = SQUARE(roughness);
	float a2 = a*a;

	float Vis_SmithV = NoV + sqrt(NoV * (NoV - NoV * a2) + a2);
	float Vis_SmithL = NoL + sqrt(NoL * (NoL - NoL * a2) + a2);
	return rcp(Vis_SmithV * Vis_SmithL);
}

float Vis_SmithJointApprox(float roughness, float NoV, float NoL)
{
	float a = SQUARE(roughness);
	float Vis_SmithV = NoL * (NoV * (1 - a) + a);
	float Vis_SmithL = NoV * (NoL * (1 - a) + a);
	return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
}

//----------------------------------------------------------------------------//
// Fresnel.
//----------------------------------------------------------------------------//
float3 F_None(float3 specular_color)
{
	return specular_color;
}

float3 F_Schlick(float3 specular_color, float VoH)
{
	float Fc = pow(1 - VoH, 5);
	return saturate(50.0 * specular_color.g) * Fc + (1 - Fc) * specular_color;

}

float3 F_Fresnel(float3 specular_color, float VoH)
{
	float3 specular_color_sqrt = sqrt(clamp(float3(0, 0, 0), float3(0.99, 0.99, 0.99), specular_color));
	float3 n = (1 + specular_color_sqrt) / (1 - specular_color_sqrt);
	float3 g = sqrt(n*n + VoH*VoH - 1);
	return 0.5 * SQUARE((g - VoH) / (g + VoH)) * (1 + SQUARE(((g + VoH)*VoH - 1) / ((g - VoH)*VoH + 1)));
}

//----------------------------------------------------------------------------//
// Environment IBL.
//----------------------------------------------------------------------------//
half3 EnvBRDFApprox(half3 specular_color, half roughness, half NoV)
{
	const half4 c0 = { -1, -0.0275, -0.572, 0.022 };
	const half4 c1 = { 1, 0.0425, 1.04, -0.04 };
	half4 r = roughness * c0 + c1;
	half a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	half2 AB = half2(-1.04, 1.04) * a004 + r.zw;
	float F90 = saturate(50.0 * specular_color.g);
	return specular_color * AB.x + F90 * AB.y;
}

half EnvBRDFApproxNonmetal(half roughness, half NoV)
{
	// Same as EnvBRDFApprox( 0.04, roughness, NoV )
	const half2 c0 = { -1, -0.0275 };
	const half2 c1 = { 1, 0.0425 };
	half2 r = roughness * c0 + c1;
	return min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
}

//----------------------------------------------------------------------------//
// Final variants.
//----------------------------------------------------------------------------//
float3 Diffuse(float3 diffuse_color, float roughness, float NoV, float NoL, float VoH)
{
#if   LM_DIFFUSE == 0
	return Diffuse_Lambert(diffuse_color);
#elif LM_DIFFUSE == 1
	return Diffuse_Burley(diffuse_color, roughness, NoV, NoL, VoH);
#elif LM_DIFFUSE == 2
	return Diffuse_OrenNayar(diffuse_color, roughness, NoV, NoL, VoH);
#endif
}

float Distribution(float roughness, float NoH)
{
#if   LM_DISTRIBUTION == 0
	return Spec_Blinn(roughness, NoH);
#elif LM_DISTRIBUTION == 1
	return Spec_Beckmann(roughness, NoH);
#elif LM_DISTRIBUTION == 2
	return Spec_GGX(roughness, NoH);
#endif
}

float GeometricVisibility(float roughness, float NoV, float NoL, float VoH)
{
#if   LM_SPEC_G == 0
	return Vis_Implicit();
#elif LM_SPEC_G == 1
	return Vis_Neumann(NoV, NoL);
#elif LM_SPEC_G == 2
	return Vis_Kelemen(VoH);
#elif LM_SPEC_G == 3
	return Vis_Schlick(roughness, NoV, NoL);
#elif LM_SPEC_G == 4
	return Vis_Smith(roughness, NoV, NoL);
#endif
}

float3 Fresnel(float3 specular_color, float VoH)
{
#if   LM_FRESNEL == 0
	return F_None(specular_color);
#elif LM_FRESNEL == 1
	return F_Schlick(specular_color, VoH);
#elif LM_FRESNEL == 2
	return F_Fresnel(specular_color, VoH);
#endif
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//