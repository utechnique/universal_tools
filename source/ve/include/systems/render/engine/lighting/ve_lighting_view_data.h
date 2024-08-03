//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_deferred_shading.h"
#include "ve_forward_shading.h"
#include "ve_image_based_lighting.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// ve::render::lighting::ViewData encapsulates render targets, uniform
// buffers, pipelines and other resources needed to apply lighting techniques.
struct ViewData
{
	// Constructor, all necessary resources must be provided.
	ViewData(Target lightbuf_target,
	         DeferredShading::ViewData deferred_shading_data,
	         ForwardShading::ViewData forward_shading_data) : light_buffer(ut::Move(lightbuf_target))
	                                                        , deferred_shading(ut::Move(deferred_shading_data))
	                                                        , forward_shading(ut::Move(forward_shading_data))
	{}

	// Move constructor and operator.
	ViewData(ViewData&&) = default;
	ViewData& operator =(ViewData&&) = default;

	// Copying is prohibited.
	ViewData(const ViewData&) = delete;
	ViewData& operator =(const ViewData&) = delete;
	
	Target light_buffer;
	DeferredShading::ViewData deferred_shading;
	ForwardShading::ViewData forward_shading;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
