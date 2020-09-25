//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_context.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Maximum number of clear values.
const ut::uint32 skMaxClearValues = 16;

//----------------------------------------------------------------------------//
// Constructor.
PlatformContext::PlatformContext(VkDevice device_handle,
                                 PlatformCmdBuffer& cmd_buffer_ref) : device(device_handle)
                                                                    , cmd_buffer(cmd_buffer_ref)
{}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&&) noexcept = default;

//----------------------------------------------------------------------------//
// Constructor.
Context::Context(PlatformContext platform_context) : PlatformContext(ut::Move(platform_context))
{}

// Maps a memory object associated with provided buffer
// into application address space. Note that buffer must be created with
// ve::render::Buffer::gpu_cpu flag to be compatible with this function.
//    @param buffer - reference to the ve::render::Buffer object to be mapped.
//    @param access - ut::Access value specifying purpose of the mapping
//                    operation - read, write or both.
//    @return - pointer to the mapped area or error if failed.
ut::Result<void*, ut::Error> Context::MapBuffer(Buffer& buffer, ut::Access access)
{
	if (buffer.info.usage != render::memory::gpu_read_cpu_write)
	{
		return ut::MakeError(ut::Error(ut::error::invalid_arg,
			"Vulkan: Attempt to map buffer that wasn\'t created with gpu_read_cpu_write flag"));
	}

	void* address;
	VkResult res = vkMapMemory(device,                      // VkDevice handle
	                           buffer.memory.GetVkHandle(), // VkDeviceMemory handle
	                           0,                           // offset
	                           buffer.info.size,            // size
	                           0,                           // flags
	                           &address);                   // out ptr
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkMapMemory(buffer)"));
	}

	return address;
}

// Unmaps a previously mapped memory object associated with provided buffer.
void Context::UnmapBuffer(Buffer& buffer)
{
	vkUnmapMemory(device, buffer.memory.GetVkHandle());
}

// Maps a memory object associated with provided image
// into application address space. Note that image must be created with
// usage flag to be compatible with this function.
//    @param image - reference to the ve::render::Image object to be mapped.
//    @param mip_level - id of the mip to be mapped.
//    @param array_layer - id of the layer to be mapped.
//    @param access - ut::Access value specifying purpose of the mapping
//                    operation - read, write or both.
ut::Result<Image::MappedResource, ut::Error> Context::MapImage(Image& image,
                                                               ut::Access access,
                                                               ut::uint32 mip_level,
                                                               ut::uint32 array_layer)
{
	Image::MappedResource mapped_rc;

	const Image::Info& info = image.GetInfo();

	// only images created with gpu_read_cpu_write flag have linear layout
	// and can be accessed without staging buffer
	if(info.usage == render::memory::gpu_read_cpu_write)
	{
		VkSubresourceLayout layout;
		VkImageSubresource subrc;
		subrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subrc.mipLevel = mip_level;
		subrc.arrayLayer = array_layer;
		vkGetImageSubresourceLayout(device, image.GetVkHandle(), &subrc, &layout);

		VkResult res = vkMapMemory(device,
		                           image.memory.GetVkHandle(),
		                           0, // offset
		                           image.memory.GetDetail().GetSize(),
		                           0, // flags
		                           &mapped_rc.data);
		if (res != VK_SUCCESS)
		{
			return ut::MakeError(VulkanError(res, "vkMapMemory(image)"));
		}

		mapped_rc.row_pitch = layout.rowPitch;
		mapped_rc.depth_pitch = layout.depthPitch;
		mapped_rc.array_pitch = layout.arrayPitch;
	}
	else
	{
		return ut::MakeError(ut::error::not_supported);
	}

	return mapped_rc;
}

// Unmaps a previously mapped memory object associated with provided image.
void Context::UnmapImage(Image& image)
{
	const Image::Info& info = image.GetInfo();
	if (info.usage == render::memory::gpu_read_cpu_write)
	{
		vkUnmapMemory(device, image.memory.GetVkHandle());
	}
}

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
	UT_ASSERT(color_clear_values.GetNum() <= skMaxClearValues);
	UT_ASSERT(color_clear_values.GetNum() <= render_pass.color_slots.GetNum());
	UT_ASSERT(render_pass.color_slots.GetNum() == framebuffer.color_targets.GetNum());
	UT_ASSERT(render_pass.depth_stencil_slot ? framebuffer.depth_stencil_target : !framebuffer.depth_stencil_target);

	const ut::uint32 clear_val_count = static_cast<ut::uint32>(color_clear_values.GetNum());
	const ut::uint32 color_slot_count = static_cast<ut::uint32>(render_pass.color_slots.GetNum());
	ut::uint32 total_clear_val_count = clear_val_count;

	// color clear values
	VkClearValue vk_clear_values[skMaxClearValues];
	for (ut::uint32 i = 0; i < clear_val_count; i++)
	{
		vk_clear_values[i].color.float32[0] = color_clear_values[i].R();
		vk_clear_values[i].color.float32[1] = color_clear_values[i].G();
		vk_clear_values[i].color.float32[2] = color_clear_values[i].B();
		vk_clear_values[i].color.float32[3] = color_clear_values[i].A();
	}

	// depth and stencil clear value
	if (render_pass.depth_stencil_slot)
	{
		// note that depth-stencil slot goes right after all color slots
		vk_clear_values[color_slot_count].depthStencil.depth = depth_clear_value;
		vk_clear_values[color_slot_count].depthStencil.stencil = stencil_clear_value;
		total_clear_val_count = color_slot_count + 1;
	}

	// initialize render pass begin info
	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = nullptr;
	rp_begin.renderPass = render_pass.GetVkHandle();
	rp_begin.framebuffer = framebuffer.GetVkHandle();
	rp_begin.renderArea.offset.x = render_area.offset.X();
	rp_begin.renderArea.offset.y = render_area.offset.Y();
	rp_begin.renderArea.extent.width = render_area.extent.X();
	rp_begin.renderArea.extent.height = render_area.extent.Y();
	rp_begin.clearValueCount = static_cast<uint32_t>(total_clear_val_count);
	rp_begin.pClearValues = vk_clear_values;

	// begin render pass
	vkCmdBeginRenderPass(cmd_buffer.GetVkHandle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
}

// End current render pass.
void Context::EndRenderPass()
{
	vkCmdEndRenderPass(cmd_buffer.GetVkHandle());
}

// Binds provided pipeline state to the current context.
//    @param pipeline_state - reference to the pipeline state.
void Context::BindPipelineState(PipelineState& pipeline_state)
{
	vkCmdBindPipeline(cmd_buffer.GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_state.pipeline.GetVkHandle());
	cmd_buffer.bound_pipeline = pipeline_state;
}

// Binds provided descriptor set to the current pipeline.
// Note that BindPipelineState() function must be called before.
//    @param pipeline_state - reference to the pipeline state.
void Context::BindDescriptorSet(DescriptorSet& set)
{
	if (!cmd_buffer.bound_pipeline)
	{
		ut::log.Lock() << "Warning! BindDescriptorSet function was called, but there is no bound pipeline." << ut::cret;
		return;
	}

	cmd_buffer.descriptor_mgr.AllocateAndBindDescriptorSet(set, cmd_buffer.bound_pipeline->dsl.GetVkHandle(),
	                                                            cmd_buffer.bound_pipeline->layout.GetVkHandle());
}

// Binds vertex buffer to the current context.
//    @param buffer - reference to the buffer to be bound.
//    @param offset - number of bytes between the first element
//                    of a vertex buffer and the first element
//                    that will be used.
void Context::BindVertexBuffer(Buffer& buffer, size_t offset)
{
	VkBuffer vertex_buffers[] = { buffer.GetVkHandle() };
	VkDeviceSize offsets[] = { offset };
	vkCmdBindVertexBuffers(cmd_buffer.GetVkHandle(), 0, 1, vertex_buffers, offsets);
}

// Draw non-indexed, non-instanced primitives.
//    @param vertex_count - number of vertices to draw.
//    @param first_vertex_id - index of the first vertex.
void Context::Draw(ut::uint32 vertex_count, ut::uint32 first_vertex_id)
{
	vkCmdDraw(cmd_buffer.GetVkHandle(), vertex_count, 1, first_vertex_id, 0);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//