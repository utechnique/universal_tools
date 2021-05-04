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
	float4 diffuse : COLOR0;
	float4 normal : COLOR1;
};

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
// Initializes a surface with G-Buffer data.
SurfaceData DecodeDeferredSurface(float depth,
                                  float4 diffuse,
                                  float4 normal,
                                  float2 texcoord,
                                  float4x4 inv_view_projection,
                                  float3 camera_position)
{
	SurfaceData surface;
	surface.world_position = RecoverPositionFromDepth(inv_view_projection, texcoord, depth);
	surface.look = normalize(camera_position - surface.world_position);
	surface.normal = normal.xyz;
	surface.diffuse = diffuse.rgb;
	surface.specular = 0.5f;
	surface.roughness = diffuse.a;
	surface.min_roughness = 0.04f;
	surface.metallic = normal.a;
	surface.cavity = 1.0f;

	return surface;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//