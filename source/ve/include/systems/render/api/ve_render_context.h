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
	// Context::ClearColors is an object incapsulating color information to
	// clear color attachments. It can hold either an array of colors to clear
	// every attachment separately or a single color to clear all attachments
	// with one color. Use Context::ClearColors function to create array of
	// colors.
	typedef const ut::Array< ut::Color<4> >& ColorArrayRef;
	typedef ut::Result<ut::Color<4>, ColorArrayRef> ClearColor;

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
	// appropriate usage flag to be compatible with this function.
	//    @param buffer - reference to the ve::render::Buffer object to be mapped.
	//    @param access - ut::Access value specifying purpose of the mapping
	//                    operation - read, write or both.
	//    @return - pointer to the mapped area or error if failed.
	ut::Result<void*, ut::Error> MapBuffer(Buffer& buffer, ut::Access access);

	// Unmaps a previously mapped memory object associated with provided buffer.
	void UnmapBuffer(Buffer& buffer);

	// Maps a memory object associated with provided image
	// into application address space. Note that image must be created with
	// usage flag to be compatible with this function.
	//    @param image - reference to the ve::render::Image object to be mapped.
	//    @param mip_level - id of the mip to be mapped.
	//    @param array_layer - id of the layer to be mapped.
	//    @param access - ut::Access value specifying purpose of the mapping
	//                    operation - read, write or both.
	ut::Result<Image::MappedResource, ut::Error> MapImage(Image& image,
	                                                      ut::Access access,
	                                                      ut::uint32 mip_level = 0,
	                                                      ut::uint32 array_layer = 0);

	// Unmaps a previously mapped memory object associated with provided image.
	void UnmapImage(Image& image);

	// Copies data between render targets.
	//    @param dst - the destination target, must be in transfer_dst state.
	//    @param src - the source target, must be in transfer_src state.
	void CopyTarget(Target& dst, Target& src);

	// Toggles render target's state.
	//    @param target - reference to the target.
	//    @param state - new state of the target.
	inline void SetTargetState(Target& target, Target::Info::State state)
	{
		ut::Array<SharedTargetData> target_data;
		target_data.Add(target);
		SetTargetState(target_data, state);
	}

	// Toggles render target state for all targets in the provided framebuffer.
	//    @param framebuffer - reference to the framebuffer.
	//    @param state - new state of the target.
	inline void SetTargetState(Framebuffer& framebuffer, Target::Info::State state)
	{
		const ut::uint32 color_target_count = static_cast<ut::uint32>(framebuffer.color_targets.GetNum());
		ut::Array<SharedTargetData> target_data(color_target_count);
		for (ut::uint32 i = 0; i < color_target_count; i++)
		{
			target_data[i] = framebuffer.color_targets[i];
		}
		if (framebuffer.depth_stencil_target)
		{
			target_data.Add(framebuffer.depth_stencil_target.Get());
		}
		SetTargetState(target_data, state);
	}

	// Begin a new render pass.
	//    @param render_pass - reference to the render pass object.
	//    @param framebuffer - reference to the framebuffer to be bound.
	//    @param render_area - reference to the rectangle representing
	//                         rendering area in pixels.
	//    @param clear_color - color to clear render targets with.
	//    @param depth_clear_value - value to clear depth buffer with.
	//    @param stencil_clear_value - value to clear stencil buffer with.
	void BeginRenderPass(RenderPass& render_pass,
	                     Framebuffer& framebuffer,
	                     const ut::Rect<ut::uint32>& render_area,
	                     const ClearColor& clear_color,
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

	// Binds vertex and instance buffers to the current context.
	//    @param vertex_buffer - reference to the vertex buffer to be bound.
	//    @param vertex_offset - number of bytes between the first element
	//                           of a vertex buffer and the first element
	//                           that will be used.
	//    @param instance_buffer - reference to the instance buffer to be bound.
	//    @param instance_offset - number of bytes between the first element
	//                             of an instance buffer and the first element
	//                             that will be used.
	void BindVertexAndInstanceBuffer(Buffer& vertex_buffer,
	                                 size_t vertex_offset,
	                                 Buffer& instance_buffer,
	                                 size_t instance_offset);

	// Binds index buffer to the current context.
	//    @param buffer - reference to the buffer to be bound.
	//    @param offset - number of bytes between the first element
	//                    of an index buffer and the first index
	//                    that will be used.
	//    @param index_type - type of index buffer indices (16 or 32).
	void BindIndexBuffer(Buffer& buffer, size_t offset, IndexType index_type);

	// Draw non-indexed, non-instanced primitives.
	//    @param vertex_count - number of vertices to draw.
	//    @param first_vertex_id - index of the first vertex.
	void Draw(ut::uint32 vertex_count, ut::uint32 first_vertex_id);

	// Draw non-indexed, instanced primitives.
	//    @param vertex_count - number of vertices to draw.
	//    @param instance_count - number of instances to draw.
	//    @param first_vertex_id - index of the first vertex.
	//    @param first_instance_id - a value added to each index before reading
	//                               per-instance data from a vertex buffer.
	void DrawInstanced(ut::uint32 vertex_count,
	                   ut::uint32 instance_count,
	                   ut::uint32 first_vertex_id,
	                   ut::uint32 first_instance_id);

	// Draw indexed, non-instanced primitives.
	//    @param index_count - number of vertices to draw.
	//    @param first_index_id - the base index within the index buffer.
	//    @param vertex_offset - the value added to the vertex index before
	//                           indexing into the vertex buffer.
	void DrawIndexed(ut::uint32 index_count,
	                 ut::uint32 first_index_id,
	                 ut::int32 vertex_offset);

	// Draw indexed, instanced primitives.
	//    @param index_count - number of vertices to draw.
	//    @param instance_count - number of instances to draw.
	//    @param first_index_id - the base index within the index buffer.
	//    @param vertex_offset - the value added to the vertex index before
	//                           indexing into the vertex buffer.
	//    @param first_instance_id - a value added to each index before reading
	//                               per-instance data from a vertex buffer.
	void DrawIndexedInstanced(ut::uint32 index_count,
	                          ut::uint32 instance_count,
	                          ut::uint32 first_index_id,
	                          ut::int32 vertex_offset,
	                          ut::uint32 first_instance_id);

	// Returns Context::ClearColor object initialized with multiple colors.
	static ut::Alternate<ColorArrayRef> ClearColors(ColorArrayRef color_array)
	{
		return ut::MakeAlt<ColorArrayRef>(color_array);
	}

private:
	// Toggles render target's state.
	//    @param targets - reference to the shared target data array.
	//    @param state - new state of the target.
	void SetTargetState(ut::Array<SharedTargetData>& targets, Target::Info::State state);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//