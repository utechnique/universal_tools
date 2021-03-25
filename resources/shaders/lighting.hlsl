//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// This file contains common lighting functions.
//----------------------------------------------------------------------------//
#include "brdf.hlsl"

//----------------------------------------------------------------------------//
// Describes a surface properties.
struct SurfaceData
{
	float3 look;
	float3 normal;
	float3 diffuse;
	float3 specular;
	float roughness;
	float min_roughness;
	float cavity;
};

// Describes a light source properties.
struct LightingData
{
	float3 direction;
	float3 color;
	float3 tube_direction;
	float ray_length;
	float source_radius;
	float source_length;
	bool is_radial;
};

//----------------------------------------------------------------------------//
float3 AreaLightSpecular(SurfaceData surface,
                         LightingData light,
                         inout float3 lobe_roughness,
                         inout float3 to_light,
                         inout float3 L,
                         float3 V,
                         float3 N)
{
	float3 lobe_energy = 1;

	lobe_roughness = max(lobe_roughness, surface.min_roughness);
	float3 m = lobe_roughness * lobe_roughness;

	const float source_radius = light.source_radius;
	const float source_length = light.source_length;

	float3 R = reflect(-V, N);
	float inv_dist_to_light = rsqrt(dot(to_light, to_light));

	if (source_length > 0)
	{
		// Energy conservation
		// asin(x) is angle to sphere, atan(x) is angle to disk, saturate(x) is free and in the middle
		float line_angle = saturate(source_length * inv_dist_to_light);
		lobe_energy *= m / saturate(m + 0.5 * line_angle);

		// Closest point on line segment to ray
		float3 L01 = light.tube_direction * source_length;
		float3 L0 = to_light - 0.5 * L01;
		float3 L1 = to_light + 0.5 * L01;

#if 1
		// Shortest distance
		float a = SQUARE(source_length);
		float b = dot(R, L01);
		float t = saturate(dot(L0, b*R - L01) / (a - b*b));
#else
		// Smallest angle
		float A = SQUARE(source_length);
		float B = 2 * dot(L0, L01);
		float C = dot(L0, L0);
		float D = dot(R, L0);
		float E = dot(R, L01);
		float t = saturate((B*D - 2 * C*E) / (B*E - 2 * A*D));
#endif

		to_light = L0 + t * L01;
	}

	if (source_radius > 0)
	{
		// Energy conservation
		// asin(x) is angle to sphere, atan(x) is angle to disk, saturate(x) is free and in the middle
		float sphere_angle = saturate(source_radius * inv_dist_to_light);
		lobe_energy *= SQUARE(m / saturate(m + 0.5 * sphere_angle));

		// Closest point on sphere to ray
		float3 closest_point_on_ray = dot(to_light, R) * R;
		float3 center_to_ray = closest_point_on_ray - to_light;
		float3 closest_point_on_sphere = to_light + center_to_ray * saturate(source_radius * rsqrt(dot(center_to_ray, center_to_ray)));
		to_light = closest_point_on_sphere;
	}

	L = normalize(to_light);

	return lobe_energy;
}

//----------------------------------------------------------------------------//
float3 StandardShading(SurfaceData surface,
                       float3 lobe_roughness,
                       float3 lobe_energy,
                       float3 L,
                       float3 V,
                       float3 N,
                       float2 diff_spec_mask)
{
	float3 H = normalize(V + L);
	float NoL = saturate(dot(N, L));
	float NoV = abs(dot(N, V)) + 1e-5;
	float NoH = saturate(dot(N, H));
	float VoH = saturate(dot(V, H));

	float D = Spec_GGX(lobe_roughness[1], NoH) * lobe_energy[1];
	float vis = Vis_SmithJointApprox(lobe_roughness[1], NoV, NoL) * surface.cavity;

	float3 F = F_Schlick(surface.specular, VoH);

	return Diffuse_Lambert(surface.diffuse) * (lobe_energy[2] * diff_spec_mask.r) + (D * vis * diff_spec_mask.g) * F;
}

//----------------------------------------------------------------------------//
float4 ComputeDirectLighting(SurfaceData surface, LightingData light)
{
	float3 N = surface.normal;
	float3 V = surface.look;
	float3 to_light = light.direction;
	float3 L = to_light;
	float NoL = saturate(dot(N, L));

	if (light.is_radial)
	{
		to_light = light.direction * light.ray_length;

		float distance_sqr = dot(to_light, to_light);
		L = to_light * rsqrt(distance_sqr);

		const float source_length = light.source_length;

		if (source_length > 0) // tube
		{
			float3 L01 = light.tube_direction * source_length;
			float3 L0 = to_light - 0.5 * L01;
			float3 L1 = to_light + 0.5 * L01;

			NoL = saturate(0.5 * (dot(N, L0) / length(L0) + dot(N, L1) / length(L1)));
		}
		else // sphere
		{
			NoL = saturate(dot(N, L));
		}
	}

	float3 out_lighting = 0;

	float3 lobe_roughness = float3(0.1, surface.roughness, 1);
	float3 lobe_energy = AreaLightSpecular(surface, light, lobe_roughness, to_light, L, V, N);

	float3 diffuse = StandardShading(surface, lobe_roughness, lobe_energy, L, V, N, float2(1, 0));
	float3 specular = StandardShading(surface, lobe_roughness, lobe_energy, L, V, N, float2(0, 1));

	out_lighting += (diffuse + specular) * NoL;

	return float4(out_lighting * light.color, 0);
}

//----------------------------------------------------------------------------//
// Caution! Use this function only if at least one of these macros is declared:
// DIRECTIONAL_LIGHT, POINT_LIGHT, SPOT_LIGHT.
// Also light_buffer.hlsl must be included because this function uses
// g_ub_light uniform buffer, 
float ComputeAttenuation(LightingData light)
{
#ifdef UB_ID_LIGHT // Existence of the UB_ID_LIGHT macro makes an assumption
                   // that g_ub_light buffer is available.
#if POINT_LIGHT || SPOT_LIGHT
	const float attenuation_distance = g_light_attenuation.x;
	float point_attenuation = saturate(attenuation_distance / light.ray_length);
	point_attenuation *= point_attenuation;
#if SPOT_LIGHT
	const float light_cos = dot(light.direction, -g_light_direction.xyz);
	const float inner_cone = g_light_attenuation.y;
	const float outer_cone = g_light_attenuation.z;
	float spot_attenuation = saturate((outer_cone - light_cos) / (outer_cone - inner_cone));
	spot_attenuation *= spot_attenuation;
	point_attenuation *= spot_attenuation;
#endif // SPOT_LIGHT
	return point_attenuation;
#endif // POINT_LIGHT || SPOT_LIGHT
#endif // UB_ID_LIGHT
	return 1.0f;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//