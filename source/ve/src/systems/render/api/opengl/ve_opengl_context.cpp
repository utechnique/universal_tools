//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_context.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformContext::PlatformContext(OpenGLContext opengl_context) : OpenGLContext(ut::Move(opengl_context))
{
	ui::DisplayScopeLock ui_lock;
	ut::Optional<ut::Error> apply_error = MakeCurrent();
	if (apply_error)
	{
		throw ut::Error(apply_error.Move());
	}
}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&& other) noexcept = default;

// Move operator.
PlatformContext& PlatformContext::operator =(PlatformContext&& other) noexcept = default;


//----------------------------------------------------------------------------//
// Constructor.
Context::Context(PlatformContext platform_context) : PlatformContext(ut::Move(platform_context))
{}

// Begin a new render pass.
//    @param render_pass - reference to the render pass object.
//    @param framebuffer - reference to the framebuffer to be bound.
//    @param render_area - reference to the rectangle representing
//                         rendering area in pixels.
//    @param color_clear_values - array of colors to clear color
//                                render targets with.
//    @param depth_clear_value - value to clear depth buffer with.
//    @param stencil_clear_value - value to clear stencil buffer with.
void Context::BeginRenderPass(RenderPass& render_pass,
	                          Framebuffer& framebuffer,
	                          const ut::Rect<ut::uint32>& render_area,
	                          const ut::Array< ut::Color<4> >& color_clear_values,
	                          float depth_clear_value,
	                          ut::uint32 stencil_clear_value)
{
	// validate arguments
	UT_ASSERT(color_clear_values.Count() <= render_pass.color_slots.Count());
	UT_ASSERT(render_pass.color_slots.Count() == framebuffer.color_targets.Count());
	UT_ASSERT(render_pass.depth_stencil_slot ? framebuffer.depth_stencil_target : !framebuffer.depth_stencil_target);

	// bind framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.GetGlHandle());

	// set draw buffers
	GLenum draw_buffers[skMaxGlColorAttachments];
	const ut::uint32 color_attachment_count = static_cast<ut::uint32>(framebuffer.color_targets.Count());
	for (ut::uint32 i = 0; i < color_attachment_count; i++)
	{
		// set draw buffer
		const GLuint color_attachment_id = GetColorAttachmentId(i);
		draw_buffers[i] = color_attachment_id;

		// if framebuffer has at least one present surface as a draw buffer, then
		// it must be added to a present queue
		if (framebuffer.color_targets[i]->GetInfo().usage == RenderTargetInfo::usage_present)
		{
			PresentRequest present_request;
			present_request.framebuffer = framebuffer.GetGlHandle();
			present_request.attachment_id = color_attachment_id;
			if (present_queue.Insert(framebuffer.color_targets[i]->image.GetGlHandle(), present_request))
			{
				throw ut::Error(ut::error::out_of_memory);
			}
		}
	}
	glDrawBuffers(color_attachment_count, draw_buffers);
	
	// clear color buffer
	const ut::uint32 clear_value_count = static_cast<ut::uint32>(color_clear_values.Count());
	for (ut::uint32 i = 0; i < clear_value_count; i++)
	{
		draw_buffers[i] = GetColorAttachmentId(i);

		glClearBufferfv(GL_COLOR, static_cast<GLint>(i), color_clear_values[i].GetData());
	}

	// clear depth
	if (framebuffer.depth_stencil_target)
	{
		glClearDepth(depth_clear_value);
	}
}

// End current render pass.
void Context::EndRenderPass()
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
