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

// Performs image layout transition.
//    @param requests - array of transition requests.
//    @param cmd_buffer - command buffer handle to record transition command.
void PlatformContext::ChangeImageState(ut::Array<ImageTransitionRequest>& requests,
                                       VkCommandBuffer cmd_buffer)
{
	ut::Array<ImageTransitionGroup> transition_groups;

	const ut::uint32 request_count = static_cast<ut::uint32>(requests.GetNum());
	for (ut::uint32 i = 0; i < request_count; i++)
	{
		ImageTransitionRequest& request = requests[i];

		// initialize barrier data
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.oldLayout = request.image.state.layout;
		barrier.newLayout = request.new_state.layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = request.image.GetVkHandle();
		barrier.subresourceRange.aspectMask = request.image.aspect_mask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.srcAccessMask = request.image.state.access_mask;
		barrier.dstAccessMask = request.new_state.access_mask;

		// search this transition variant
		ut::Optional<ImageTransitionGroup&> suitable_group;
		const ut::uint32 group_count = static_cast<ut::uint32>(transition_groups.GetNum());
		for (ut::uint32 t = 0; t < group_count; t++)
		{
			ImageTransitionGroup& current_group = transition_groups[t];
			if (current_group.old_stage == request.image.state.stages &&
				current_group.new_stage == request.new_state.stages)
			{
				suitable_group = current_group; break;
			}
		}

		// if there is no such transition - add a new group
		if (!suitable_group)
		{
			transition_groups.Add({ 0, request.image.state.stages, request.new_state.stages });
			suitable_group = transition_groups.GetLast();
		}

		// update state information
		suitable_group->barriers.Add(barrier);
		request.image.state = request.new_state;
	}

	// perform pipeline barriers - one for each group
	const ut::uint32 group_count = static_cast<ut::uint32>(transition_groups.GetNum());
	for (ut::uint32 t = 0; t < group_count; t++)
	{
		ImageTransitionGroup& group = transition_groups[t];
		const uint32_t barrier_count = static_cast<uint32_t>(group.barriers.GetNum());
		vkCmdPipelineBarrier(cmd_buffer,
		                     group.old_stage,
		                     group.new_stage,
		                     0, 0, nullptr, 0, nullptr,
		                     barrier_count, group.barriers.GetAddress());
	}
}

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

// Toggles render target's state.
//    @param targets - reference to the shared target data array.
//    @param state - new state of the target.
void Context::SetTargetState(ut::Array<SharedTargetData>& targets, Target::Info::State state)
{
	ut::Array<ImageTransitionRequest> transition_requests;

	const ut::uint32 target_count = static_cast<ut::uint32>(targets.GetNum());
	for (ut::uint32 i = 0; i < target_count; i++)
	{
		TargetData& target_data = targets[i].GetRef();
		const Target::Info target_info = target_data.info;
		if (target_info.state == state)
		{
			continue;
		}

		// choose corresponding image state
		PlatformImage::State image_state = PlatformImage::State::CreateForShaderResource();
		if (state == Target::Info::state_target)
		{
			if (target_info.usage == Target::Info::usage_depth)
			{
				image_state = PlatformImage::State::CreateForDepthStencilTarget();
			}
			else
			{
				image_state = PlatformImage::State::CreateForColorTarget();
			}
		}

		// create transition request and update state information
		transition_requests.Add({ target_data.image, image_state });
		target_data.info.state = state;
	}

	// pipeline barrier
	ChangeImageState(transition_requests, cmd_buffer.GetVkHandle());
}

// Begin a new render pass.
//    @param render_pass - reference to the render pass object.
//    @param framebuffer - reference to the framebuffer to be bound.
//    @param render_area - reference to the rectangle representing
//                         rendering area in pixels.
//    @param clear_color - color to clear render targets with.
//    @param depth_clear_value - value to clear depth buffer with.
//    @param stencil_clear_value - value to clear stencil buffer with.
void Context::BeginRenderPass(RenderPass& render_pass,
                              Framebuffer& framebuffer,
                              const ut::Rect<ut::uint32>& render_area,
                              const ClearColor& clear_color,
                              float depth_clear_value,
                              ut::uint32 stencil_clear_value)
{
	// validate arguments
	if (!clear_color)
	{
		UT_ASSERT(clear_color.GetAlt().GetNum() <= skMaxClearValues);
		UT_ASSERT(clear_color.GetAlt().GetNum() == render_pass.color_slots.GetNum());
	}
	UT_ASSERT(render_pass.color_slots.GetNum() == framebuffer.color_targets.GetNum());
	UT_ASSERT(render_pass.depth_stencil_slot ? framebuffer.depth_stencil_target : !framebuffer.depth_stencil_target);
	
	const ut::uint32 color_slot_count = static_cast<ut::uint32>(render_pass.color_slots.GetNum());
	ut::uint32 total_clear_val_count = color_slot_count;

	// color clear values
	VkClearValue vk_clear_values[skMaxClearValues];
	for (ut::uint32 i = 0; i < color_slot_count; i++)
	{
		const ut::Color<4>& color = clear_color ?
		                            clear_color.Get() :
		                            clear_color.GetAlt()[i];

		vk_clear_values[i].color.float32[0] = color.R();
		vk_clear_values[i].color.float32[1] = color.G();
		vk_clear_values[i].color.float32[2] = color.B();
		vk_clear_values[i].color.float32[3] = color.A();
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

	// all render targets must be in 'target' state before render pass begins
	SetTargetState(framebuffer, Target::Info::state_target);

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
	// check if a pipeline state is set
	if (!cmd_buffer.bound_pipeline)
	{
		ut::log.Lock() << "Warning! BindDescriptorSet function was called, but there is no bound pipeline." << ut::cret;
		return;
	}

	// bind descriptors
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