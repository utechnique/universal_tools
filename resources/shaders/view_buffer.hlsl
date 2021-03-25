//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// Uniform buffer for light sources. Include this file to the final shader file
// to have access to the view buffer.
//----------------------------------------------------------------------------//
#include "preprocessor.hlsl"

//----------------------------------------------------------------------------//
// You can manually define UB_ID_VIEW macro before including this file to
// control binding id of the view buffer.
#ifndef UB_ID_VIEW
#define UB_ID_VIEW 0
#endif

//----------------------------------------------------------------------------//
cbuffer g_ub_view : register(SET_BUFFER_INDEX(UB_ID_VIEW))
{
	row_major float4x4 g_view_proj;
	row_major float4x4 g_inv_view_proj;
	float4 g_camera_position;
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//