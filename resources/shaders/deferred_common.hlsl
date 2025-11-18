//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// This file contains common functions for deferred shading techniques.
//----------------------------------------------------------------------------//
#include "lighting.hlsl"

//----------------------------------------------------------------------------//
// Pixel shader output structure writing to the geometry buffer.
struct GBuffer
{
	float4 base_color : COLOR0;
	float4 normal : COLOR1;
	float4 emissive : COLOR2;
};

//----------------------------------------------------------------------------//
// Encodes a float3 unit-vector into a projected octahedron float2 vector.
float2 EncodeFloat3ToOctFloat2(float3 v)
{
	// Project the sphere onto the octahedron, and then onto the xy plane
	float2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));

	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ?
	       ((1.0 - abs(p.yx)) * float2((p.x >= 0.0) ? +1.0 : -1.0,
	                                   (p.y >= 0.0) ? +1.0 : -1.0)) :
	       p;
}

// Decodes a float3 unit-vector from a projected octahedron float2 vector.
float3 DecodeFloat3FromOctFloat2(float2 e)
{
	float3 v = float3(e.xy, 1.0 - abs(e.x) - abs(e.y));

	if (v.z < 0)
	{
		v.xy = (1.0 - abs(v.yx)) * float2((v.x >= 0.0) ? 1.0 : -1.0,
		                                  (v.y >= 0.0) ? 1.0 : -1.0);
	}

	return normalize(v);
}

// Packs a signed 2D unit vector with a precision of 12 bits per channel
// into a 3D unsigned unit vector with 8 bits per channel.
float3 PackSNorm12x2ToUNorm8x3(float2 f)
{
	float2 u = float2(round(clamp(f, -1.0, 1.0) * 2047 + 2047));
	float t = floor(u.y / 256.0);
	return floor(float3(u.x / 16.0,
	                    frac(u.x / 16.0) * 256.0 + t,
	                    u.y - t * 256.0)) / 255.0;
}

// Packs a 3D unsigned unit vector with a precision of 8 bits per channel
// into a 2D signed unit vector with 12 bits per channel.
float2 PackUNorm8x3ToSNorm12x2(float3 u)
{
	u *= 255.0;
	u.y *= (1.0 / 16.0);
	float2 s = float2(u.x * 16.0 + floor(u.y),
	                  frac(u.y) * (16.0 * 256.0) + u.z);
	return clamp(s * (1.0 / 2047.0) - 1.0, -1.0, 1.0);
}

// Packs a normal using octahedron mapping.
float3 PackNormalOct(float3 normal)
{
	float2 n2 = EncodeFloat3ToOctFloat2(normal);
	return PackSNorm12x2ToUNorm8x3(n2);
}

// Unpacks a normal using octahedron mapping.
float3 UnpackNormalOct(float3 encodedNormal)
{
	float2 n2 = PackUNorm8x3ToSNorm12x2(encodedNormal);
	return DecodeFloat3FromOctFloat2(n2);
}

//----------------------------------------------------------------------------//
// Calculates world 3d position of the pixel using the depth value from the
// G-Buffer.
float3 RecoverPositionFromDepth(float4x4 view_projection,
                                float2 texcoord,
                                float depth)
{
	float4 out_position;
	out_position.x = texcoord.x * 2.0f - 1.0f;
	out_position.y = -(texcoord.y * 2.0f - 1.0f);
	out_position.z = depth;
	out_position.w = 1.0f;
	out_position = mul(out_position, view_projection);
	out_position /= out_position.w;
	return out_position.xyz;
}

//----------------------------------------------------------------------------//
// Packs provided surface parameters into a GBuffer data.
GBuffer EncodeGBuffer(float3 normal,
                      float3 base_color,
                      float3 emissive,
                      float roughness,
                      float metallic,
                      float occlusion)
{
	GBuffer output;

	output.base_color.rgb = base_color;
	output.base_color.a = roughness;
	output.normal.rgb = PackNormalOct(normal);
	output.normal.a = metallic;
	output.emissive.rgb = emissive;
	output.emissive.a = occlusion;

	return output;
}

//----------------------------------------------------------------------------//
// Initializes a surface with G-Buffer data.
SurfaceData DecodeDeferredSurface(float depth,
                                  float4 base_color,
                                  float4 normal,
                                  float2 texcoord,
                                  float4x4 inv_view_projection,
                                  float3 camera_position)
{
	SurfaceData surface;
	surface.world_position = RecoverPositionFromDepth(inv_view_projection, texcoord, depth);
	surface.look = normalize(camera_position - surface.world_position);
	surface.normal = UnpackNormalOct(normal.xyz);
	surface.diffuse = base_color.rgb;
	surface.specular = 0.5f;
	surface.roughness = base_color.a;
	surface.min_roughness = 0.001f;
	surface.metallic = normal.a;
	surface.cavity = 1.0f;

	return surface;
}

//----------------------------------------------------------------------------//
// Extracts the emissive color from the G-Buffer data.
float3 DecodeDeferredEmissive(float4 emissive)
{
	return emissive.rgb;
}

//----------------------------------------------------------------------------//
// Extracts the occlusion factor from the G-Buffer data.
float DecodeDeferredOcclusion(float4 emissive)
{
	return emissive.a;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//