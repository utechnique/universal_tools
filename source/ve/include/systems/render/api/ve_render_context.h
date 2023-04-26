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
	void UnmapImage(Image& image, ut::Access access);

	// Copies the provided image to the staging buffer that could be read from
	// the cpu. The image must be created with the
	// ve::render::Image::Info::has_staging_gpu_cpu_buffer flag enabled.
	//    @param image - reference to the ve::render::Image object.
	//    @param first_slice - first array slice id.
	//    @param slice_count - the number of slices to copy,
	//                         0 means all remaining.
	//    @param first_mip - first mip id.
	//    @param mip_count - the number of mips to copy,
	//                       0 means all remaining.
	//    @return - optional ve::Error if operation failed.
	ut::Optional<ut::Error> CopyImageToStagingCpuReadBuffer(Image& image,
	                                                        ut::uint32 first_slice = 0,
	                                                        ut::uint32 slice_count = 0,
	                                                        ut::uint32 first_mip = 0,
	                                                        ut::uint32 mip_count = 0);

	// Copies data between render targets.
	//    @param dst - the destination target, must be in transfer_dst state.
	//    @param src - the source target, must be in transfer_src state.
	//    @param first_slice - first array slice id.
	//    @param slice_count - the number of slices to copy,
	//                         0 means all remaining.
	//    @param first_mip - first mip id.
	//    @param mip_count - the number of mips to copy,
	//                       0 means all remaining.
	void CopyTarget(Target& dst,
	                Target& src,
	                ut::uint32 first_slice = 0,
	                ut::uint32 slice_count = 0,
	                ut::uint32 first_mip = 0,
	                ut::uint32 mip_count = 0);

	// Clears provided render target. This function is slow, don't use it often,
	// call BeginRenderPass() instead to clear targets.
	//    @param target - target to be cleared.
	//    @param color - clear color.
	//    @param first_slice - first array slice id.
	//    @param slice_count - the number of slices to copy,
	//                         0 means all remaining.
	//    @param first_mip - first mip id.
	//    @param mip_count - the number of mips to copy,
	//                       0 means all remaining.
	void ClearTarget(Target& target,
	                 ut::Color<4> color,
	                 ut::uint32 first_slice = 0,
	                 ut::uint32 slice_count = 0,
	                 ut::uint32 first_mip = 0,
	                 ut::uint32 mip_count = 0);

	// Uses the largest mipmap level of the provided target to recursively
	// generate the lower levels of the mip and stops with the smallest level.
	//    @param target - reference to the render target.
	//    @param new_state - optional target state to be set after all mip levels
	//                       were generated.
	void GenerateMips(Target& target,
	                  const ut::Optional<Target::Info::State>& new_state = ut::Optional<Target::Info::State>());

	// Toggles specified state for all provided targets.
	//    @param targets - array of references to render targets.
	//    @param state - the new state.
	template<ut::uint32 target_count>
	void SetTargetState(ut::Ref<Target> targets[],
	                    Target::Info::State state)
	{
		SharedTargetData* shared_targets[target_count];
		for (ut::uint32 i = 0; i < target_count; i++)
		{
			shared_targets[i] = &targets[i].Get();
		}

		SetTargetState(shared_targets, target_count, state);
	}

	// Toggles specified state for a single render target.
	//    @param target - reference to the target.
	//    @param state - new state of the target.
	inline void SetTargetState(Target& target,
	                           Target::Info::State state)
	{
		SharedTargetData* targets[1] = { &target };
		SetTargetState(targets, 1, state);
	}

	// Begin a new render pass.
	//    @param render_pass - reference to the render pass object.
	//    @param framebuffer - reference to the framebuffer to be bound.
	//    @param render_area - reference to the rectangle representing
	//                         rendering area in pixels.
	//    @param clear_color - color to clear render targets with.
	//    @param depth_clear_value - value to clear depth buffer with.
	//    @param stencil_clear_value - value to clear stencil buffer with.
	//    @param aims_secondary_buffers - indicates that this render pass will
	//                                    be used only by secondary buffers.
	//                                    Otherwise it can be used only by
	//                                    one primary buffer.
	void BeginRenderPass(RenderPass& render_pass,
	                     Framebuffer& framebuffer,
	                     const ut::Rect<ut::uint32>& render_area,
	                     const ClearColor& clear_color,
	                     float depth_clear_value = 1.0f,
	                     ut::uint32 stencil_clear_value = 0,
	                     bool aims_secondary_buffers = false);

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

	// Executes provided secondary command buffers. Note that this function
	// does nothing if this context is writing to the non-primary command buffer.
	//    @param buffers - array of references to the secondary buffer.
	void ExecuteSecondaryBuffers(ut::Array< ut::Ref<CmdBuffer> >& buffers);

private:
	// Toggles render target's state.
	//    @param targets - array of pointers to render targets.
	//    @param target_count - the number of elements in @targets array.
	//    @param state - the new state.
	void SetTargetState(SharedTargetData** targets,
	                    ut::uint32 target_count,
	                    Target::Info::State state);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//