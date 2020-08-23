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
#include "systems/render/api/ve_render_buffer.h"
#include "systems/render/api/ve_render_pipeline_state.h"
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

	// Maps a memory object associated with provided buffer
	// into application address space. Note that buffer must be created with
	// ve::render::Buffer::gpu_cpu flag to be compatible with this function.
	//    @param buffer - reference to the ve::render::Buffer object to be mapped.
	//    @param access - ut::Access value specifying purpose of the mapping
	//                    operation - read, write or both.
	//    @return - pointer to the mapped area or error if failed.
	ut::Result<void*, ut::Error> MapBuffer(Buffer& buffer, ut::Access access);

	// Unmaps a previously mapped memory object associated with provided buffer.
	void UnmapBuffer(Buffer& buffer);

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

	// Binds provided pipeline state to the current context.
	//    @param pipeline_state - reference to the pipeline state.
	void BindPipelineState(PipelineState& pipeline_state);

	// Binds provided descriptor set to the current pipeline.
	// Note that BindPipelineState() function must be called before.
	//    @param pipeline_state - reference to the pipeline state.
	void BindDescriptorSet(DescriptorSet& descriptor_set);

	// Binds vertex buffer to the current context.
	//    @param buffer - reference to the buffer to be bound.
	//    @param offset - number of bytes between the first element
	//                    of a vertex buffer and the first element
	//                    that will be used.
	void BindVertexBuffer(Buffer& buffer, size_t offset);

	// Draw non-indexed, non-instanced primitives.
	//    @param vertex_count - number of vertices to draw.
	//    @param first_vertex_id - index of the first vertex.
	void Draw(ut::uint32 vertex_count, ut::uint32 first_vertex_id);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//