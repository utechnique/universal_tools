//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Uniform buffer for light sources. Include this file to the final shader file
// to have access to the light buffer.
//----------------------------------------------------------------------------//
#include "preprocessor.hlsl"

//----------------------------------------------------------------------------//
// You can manually define UB_ID_LIGHT macro before including this file to
// control binding id of the light buffer.
#ifndef UB_ID_LIGHT
#define UB_ID_LIGHT 0
#endif

//----------------------------------------------------------------------------//

cbuffer g_ub_light : register(SET_BUFFER_INDEX(UB_ID_LIGHT))
{
	float4 g_light_position;
	float4 g_light_direction; // .w - source shape radius
	float4 g_light_color; // .w - source shape length
	float4 g_light_attenuation; // .x - attenuation distance
	                            // .y - inner cone
	                            // .z - outer cone
	float4 g_light_orientation; // orientation of the source shape
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//