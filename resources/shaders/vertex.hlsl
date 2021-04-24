//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// This file contains a universal vertex type for the vertex uber shader input.
//----------------------------------------------------------------------------//
// Compile this shader with macros listed below to enable or disable desired
// vertex components.
#ifndef VERTEX_HAS_POSITION
#define VERTEX_HAS_POSITION 0
#endif

#ifndef VERTEX_2D_POSITION
#define VERTEX_2D_POSITION 0
#endif

#ifndef VERTEX_3D_POSITION
#define VERTEX_3D_POSITION 0
#endif

#ifndef VERTEX_HAS_TEXCOORD
#define VERTEX_HAS_TEXCOORD 0
#endif

#ifndef VERTEX_HAS_NORMAL
#define VERTEX_HAS_NORMAL 0
#endif

#ifndef VERTEX_HAS_TANGENT
#define VERTEX_HAS_TANGENT 0
#endif

#ifndef VERTEX_HAS_WEIGHTS
#define VERTEX_HAS_WEIGHTS 0
#endif

#ifndef VERTEX_HAS_INSTANCE_ID
#define VERTEX_HAS_INSTANCE_ID 0
#endif

//----------------------------------------------------------------------------//
// Vertex shader input.
struct Vertex
{
#if VERTEX_HAS_POSITION
#if VERTEX_2D_POSITION
	float2 position : POSITION;
#elif VERTEX_3D_POSITION
	float3 position : POSITION;
#endif
#endif // VERTEX_HAS_POSITION

#if VERTEX_HAS_TEXCOORD
	float2 texcoord : TEXCOORD;
#endif

#if VERTEX_HAS_NORMAL
	float3 normal : NORMAL;
#endif

#if VERTEX_HAS_TANGENT
	float3 tangent : TANGENT;
#endif

#if VERTEX_HAS_WEIGHTS
	float4 weights : WEIGHTS;
#endif

#if VERTEX_HAS_BONES
	uint4 bones : BONES;
#endif

#if VERTEX_HAS_INSTANCE_ID
	uint instance_id : INSTANCE_ID;
#endif
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//