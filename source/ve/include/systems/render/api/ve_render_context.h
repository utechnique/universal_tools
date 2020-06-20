//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_target.h"
#include "systems/render/api/ve_render_pass.h"
#include "systems/render/api/ve_render_framebuffer.h"
#include "systems/render/api/ve_render_display.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Context class represents a device context interface which
// generates rendering commands.
class Context : public PlatformContext
{
public:
	// Constructor.
	Context(PlatformContext platform_context);

	// Move constructor.
	Context(Context&&) = default;

	// Move operator.
	Context& operator =(Context&&) = default;

	// Copying is prohibited.
	Context(const Context&) = delete;
	Context& operator =(const Context&) = delete;

	// Begin a new render pass.
	//    @param render_pass - reference to the render pass object.
	//    @param framebuffer - reference to the framebuffer to be bound.
	//    @param render_area - reference to the rectangle representing
	//                         rendering area in pixels.
	//    @param color_clear_values - array of colors to clear color
	//                                render targets with.
	//    @param depth_clear_value - value to clear depth buffer with.
	//    @param stencil_clear_value - value to clear stencil buffer with.
	void BeginRenderPass(RenderPass& render_pass,
	                     Framebuffer& framebuffer,
	                     const ut::Rect<ut::uint32>& render_area,
	                     const ut::Array< ut::Color<4> >& color_clear_values,
	                     float depth_clear_value = 1.0f,
	                     ut::uint32 stencil_clear_value = 0);

	// End current render pass.
	void EndRenderPass();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//