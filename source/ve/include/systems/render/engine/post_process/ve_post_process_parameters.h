//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_gaussian_blur.h"
#include "ve_tone_mapping.h"
#include "ve_stencil_highlight.h"
#include "ve_dithering.h"
#include "ve_fxaa.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Parameters for all post-processing effects gathered in one struct.
struct Parameters
{
	ToneMapper::Parameters tone_mapping;
	StencilHighlight::Parameters stencil_highlight;
	Dithering::Parameters dithering;
	Fxaa::Parameters fxaa;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
