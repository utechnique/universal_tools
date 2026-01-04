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
// Returns appropriate image state for the provided target info.
PlatformImage::State CreateTargetImageState(Target::Info target_info,
                                            Target::Info::State state)
{
	if (state == Target::Info::State::target)
	{
		if (target_info.usage == Target::Info::Usage::depth)
		{
			return PlatformImage::State::CreateForDepthStencilTarget();
		}
		else
		{
			return PlatformImage::State::CreateForColorTarget();
		}
	}
	else if (state == Target::Info::State::transfer_src)
	{
		return PlatformImage::State::CreateForTransferSrc();
	}
	else if (state == Target::Info::State::transfer_dst)
	{
		return PlatformImage::State::CreateForTransferDst();
	}
	return PlatformImage::State::CreateForShaderResource();
}

//----------------------------------------------------------------------------//
// Constructor.
PlatformContext::PlatformContext(VkDevice device_handle,
                                 VmaAllocator allocator_handle,
                                 PlatformCmdBuffer& cmd_buffer_ref,
                                 float in_timestamp_period,
                                 ut::Optional< ut::Vector<2, ut::uint32> > framebuffer_size) : device(device_handle)
                                                                                             , allocator(allocator_handle)
                                                                                             , cmd_buffer(cmd_buffer_ref)
                                                                                             , timestamp_period(static_cast<double>(in_timestamp_period))
                                                                                             , bound_framebuffer_size(ut::Move(framebuffer_size))
{}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&&) noexcept = default;

// Performs image layout transition.
//    @param requests - array of transition requests.
//    @param cmd_buffer - command buffer handle to record transition command.
void PlatformContext::ChangeImageState(ut::Array<ImageTransitionRequest>& requests)
{
	ut::Array<ImageTransitionGroup> transition_groups;
	const ut::uint32 request_count = static_cast<ut::uint32>(requests.Count());
	for (ut::uint32 i = 0; i < request_count; i++)
	{
		ImageTransitionRequest& request = requests[i];

		// initialize barrier data
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.oldLayout = request.old_state.layout;
		barrier.newLayout = request.new_state.layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = request.image.GetVkHandle();
		barrier.subresourceRange.aspectMask = request.image.aspect_mask;
		barrier.subresourceRange.baseMipLevel = request.first_mip;
		barrier.subresourceRange.levelCount = request.mip_count;
		barrier.subresourceRange.baseArrayLayer = request.first_slice;
		barrier.subresourceRange.layerCount = request.slice_count;
		barrier.srcAccessMask = request.old_state.access_mask;
		barrier.dstAccessMask = request.new_state.access_mask;

		// search this transition variant
		ut::Optional<ImageTransitionGroup&> suitable_group;
		const ut::uint32 group_count = static_cast<ut::uint32>(transition_groups.Count());
		for (ut::uint32 t = 0; t < group_count; t++)
		{
			ImageTransitionGroup& current_group = transition_groups[t];
			if (current_group.old_stage == request.old_state.stages &&
				current_group.new_stage == request.new_state.stages)
			{
				suitable_group = current_group; break;
			}
		}

		// if there is no such transition - add a new group
		if (!suitable_group)
		{
			transition_groups.Add({ 0, request.old_state.stages, request.new_state.stages });
			suitable_group = transition_groups.GetLast();
		}

		// create memory barrier and update state information
		suitable_group->barriers.Add(barrier);
		if (request.update_image_state)
		{
			request.image.state = request.new_state;
		}
	}

	// perform pipeline barriers - one for each group
	const ut::uint32 group_count = static_cast<ut::uint32>(transition_groups.Count());
	for (ut::uint32 t = 0; t < group_count; t++)
	{
		ImageTransitionGroup& group = transition_groups[t];
		const uint32_t barrier_count = static_cast<uint32_t>(group.barriers.Count());
		vkCmdPipelineBarrier(cmd_buffer.GetVkHandle(),
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
//    @param access - ve::render::memory::CpuAccess value specifying purpose
//                    of the mapping operation - read, write or both.
//    @return - pointer to the mapped area or error if failed.
ut::Result<void*, ut::Error> Context::MapBuffer(Buffer& buffer, memory::CpuAccess access)
{
	if (buffer.info.usage != render::memory::Usage::gpu_read_cpu_write)
	{
		return ut::MakeError(ut::Error(ut::error::invalid_arg,
			"Vulkan: Attempt to map buffer that wasn\'t created with gpu_read_cpu_write flag"));
	}

	void* address;
	VkResult res = vmaMapMemory(allocator,
	                            buffer.GetDetail().GetAllocation(),
	                            &address);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkMapMemory(buffer)"));
	}

	return address;
}

// Unmaps a previously mapped memory object associated with provided buffer.
void Context::UnmapBuffer(Buffer& buffer)
{
	vmaUnmapMemory(allocator, buffer.GetDetail().GetAllocation());
}

// Maps a memory object associated with provided image
// into application address space. Note that image must be created with
// usage flag to be compatible with this function.
//    @param image - reference to the ve::render::Image object to be mapped.
//    @param mip_level - id of the mip to be mapped.
//    @param array_layer - id of the layer to be mapped.
//    @param access - ve::render::memory::CpuAccess value specifying purpose
//                    of the mapping operation - read, write or both.
ut::Result<Image::MappedResource, ut::Error> Context::MapImage(Image& image,
                                                               memory::CpuAccess access,
                                                               ut::uint32 mip_level,
                                                               ut::uint32 array_layer)
{
	Image::MappedResource mapped_rc;

	const Image::Info& info = image.GetInfo();

	// only images created with gpu_read_cpu_write flag have linear layout
	// and can be accessed without staging buffer
	if(info.usage == render::memory::Usage::gpu_read_cpu_write)
	{
		VkSubresourceLayout layout;
		VkImageSubresource subrc;
		subrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subrc.mipLevel = mip_level;
		subrc.arrayLayer = array_layer;
		vkGetImageSubresourceLayout(device, image.GetVkHandle(), &subrc, &layout);

		VkResult res = vmaMapMemory(allocator,
		                            image.GetDetail().GetAllocation(),
		                            &mapped_rc.data);
		if (res != VK_SUCCESS)
		{
			return ut::MakeError(VulkanError(res, "vkMapMemory(image)"));
		}

		mapped_rc.row_pitch = static_cast<size_t>(layout.rowPitch);
		mapped_rc.depth_pitch = static_cast<size_t>(layout.depthPitch);
		mapped_rc.array_pitch = static_cast<size_t>(layout.arrayPitch);
	}
	else if (info.usage == render::memory::Usage::gpu_read_write &&
	         access == memory::CpuAccess::read &&
	         info.has_staging_cpu_read_buffer)
	{
		void* address;
		VkResult res = vmaMapMemory(allocator,
		                            image.staging_gpu_cpu_buffer.GetDetail().GetAllocation(),
		                            &address);
		if (res != VK_SUCCESS)
		{
			return ut::MakeError(VulkanError(res, "vkMapMemory(image staging cpu-read buffer)"));
		}

		VkDeviceSize buffer_offset = 0;
		ut::uint32 mip_width = info.width;
		ut::uint32 mip_height = info.height;
		ut::uint32 mip_depth = info.depth;
		for (ut::uint32 slice = 0; slice < array_layer; array_layer++)
		{
			mip_width = info.width;
			mip_height = info.height;
			mip_depth = info.depth;

			for (ut::uint32 mip = 0; mip < mip_level; mip++)
			{
				mip_width = ut::Max<ut::uint32>(mip_width / 2, 1);
				mip_height = ut::Max<ut::uint32>(mip_height / 2, 1);
				mip_depth = ut::Max<ut::uint32>(mip_depth / 2, 1);
				buffer_offset += static_cast<VkDeviceSize>(mip_width) *
				                 mip_height * mip_depth * pixel::GetSize(info.format);
			}
		}

		mapped_rc.data = static_cast<ut::byte*>(address) + buffer_offset;
		mapped_rc.row_pitch = mip_width * pixel::GetSize(info.format);
		mapped_rc.depth_pitch = mip_height * mip_width * pixel::GetSize(info.format);
		mapped_rc.array_pitch = 0;
	}
	else
	{
		return ut::MakeError(ut::error::not_supported);
	}

	return mapped_rc;
}

// Unmaps a previously mapped memory object associated with provided image.
void Context::UnmapImage(Image& image, memory::CpuAccess access)
{
	const Image::Info& info = image.GetInfo();
	if (info.usage == render::memory::Usage::gpu_read_cpu_write)
	{
		vmaUnmapMemory(allocator, image.GetDetail().GetAllocation());
	}
	else if (info.usage == render::memory::Usage::gpu_read_write &&
	         access == memory::CpuAccess::read &&
	         info.has_staging_cpu_read_buffer)
	{
		vmaUnmapMemory(allocator, image.staging_gpu_cpu_buffer.GetDetail().GetAllocation());
	}
}

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
ut::Optional<ut::Error> Context::CopyImageToStagingCpuReadBuffer(Image& image,
                                                                 ut::uint32 first_slice,
                                                                 ut::uint32 slice_count,
                                                                 ut::uint32 first_mip,
                                                                 ut::uint32 mip_count)
{
	if (image.GetInfo().has_staging_cpu_read_buffer == false)
	{
		return ut::Error(ut::error::not_supported);
	}

	const Image::Info& img_info = image.GetInfo();

	// calculate the number of subresources
	const ut::uint32 img_slice_count = img_info.type == Image::Type::cubic ? 6 : 1;
	const ut::uint32 slice_end = slice_count == 0 ? img_slice_count : (first_slice + slice_count);
	const ut::uint32 mip_end = mip_count == 0 ? img_info.mip_count : (first_mip + mip_count);

	// initialize copy regions
	VkDeviceSize buffer_offset = 0;
	ut::Array<VkBufferImageCopy> img_copy_regions;
	for (ut::uint32 slice = first_slice; slice < slice_end; slice++)
	{
		ut::uint32 mip_width = img_info.width;
		ut::uint32 mip_height = img_info.height;
		ut::uint32 mip_depth = img_info.depth;

		for (ut::uint32 mip = first_mip; mip < mip_end; mip++)
		{
			VkBufferImageCopy region{};
			region.bufferOffset = buffer_offset;
			region.bufferRowLength = 0; // Specifying 0 means tightly packed data.
			region.bufferImageHeight = 0; // Specifying 0 means tightly packed data.
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = mip;
			region.imageSubresource.baseArrayLayer = slice;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { mip_width, mip_height, mip_depth };

			img_copy_regions.Add(region);

			// calculate data for the next mip
			mip_width = ut::Max<ut::uint32>(mip_width / 2, 1);
			mip_height = ut::Max<ut::uint32>(mip_height / 2, 1);
			mip_depth = ut::Max<ut::uint32>(mip_depth / 2, 1);
			buffer_offset += static_cast<VkDeviceSize>(mip_width) *
			                 mip_height * mip_depth * pixel::GetSize(img_info.format);
		}
	}

	vkCmdCopyImageToBuffer(cmd_buffer.GetVkHandle(),
	                       image.GetVkHandle(),
	                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	                       image.staging_gpu_cpu_buffer.GetVkHandle(),
	                       static_cast<uint32_t>(img_copy_regions.Count()),
	                       img_copy_regions.GetAddress());

	// success
	return ut::Optional<ut::Error>();
}

// Copies data between render targets.
//    @param dst - the destination target, must be in transfer_dst state.
//    @param src - the source target, must be in transfer_src state.
//    @param first_slice - first array slice id.
//    @param slice_count - the number of slices to copy.
//    @param first_mip - first mip id.
//    @param mip_count - the number of mips to copy.
void Context::CopyTarget(Target& dst,
                         Target& src,
                         ut::uint32 first_slice,
                         ut::uint32 slice_count,
                         ut::uint32 first_mip,
                         ut::uint32 mip_count)
{
	const Target::Info& src_info = src.GetInfo();
	const Target::Info& dst_info = dst.GetInfo();
	Image& src_image = src.GetImage();
	Image& dst_image = dst.GetImage();

	if (src_info.format != dst_info.format ||
	    src_info.type != dst_info.type)
	{
		throw ut::Error(ut::error::types_not_match,
		                "Unable to copy images with different formats.");
	}
	const Image::Type type = src_info.type;

	// calculate metrics of the region to copy
	const ut::uint32 width = ut::Min(src_info.width, dst_info.width);
	const ut::uint32 height = ut::Min(src_info.height, dst_info.height);
	const ut::uint32 depth = ut::Min(src_info.depth, dst_info.depth);

	// calculate the number of subresources
	const ut::uint32 img_slice_count = type == Image::Type::cubic ? 6 : 1;
	const ut::uint32 img_mip_count = ut::Min(src_info.mip_count, dst_info.mip_count);
	const ut::uint32 slice_end = slice_count == 0 ? img_slice_count : (first_slice + slice_count);
	const ut::uint32 mip_end = mip_count == 0 ? img_mip_count : (first_mip + mip_count);

	// initialize copy regions
	ut::Array<VkImageCopy> img_copy_regions;
	for (ut::uint32 slice = first_slice; slice < slice_end; slice++)
	{
		ut::uint32 mip_width = width;
		ut::uint32 mip_height = height;
		ut::uint32 mip_depth = depth;

		for (ut::uint32 mip = first_mip; mip < mip_end; mip++)
		{
			VkImageCopy region;

			region.srcSubresource.aspectMask = src_image.aspect_mask;
			region.srcSubresource.mipLevel = mip;
			region.srcSubresource.baseArrayLayer = slice;
			region.srcSubresource.layerCount = 1;
			region.srcOffset.x = 0;
			region.srcOffset.y = 0;
			region.srcOffset.z = 0;

			region.dstSubresource.aspectMask = dst_image.aspect_mask;
			region.dstSubresource.mipLevel = mip;
			region.dstSubresource.baseArrayLayer = slice;
			region.dstSubresource.layerCount = 1;
			region.dstOffset.x = 0;
			region.dstOffset.y = 0;
			region.dstOffset.z = 0;

			region.extent.width = mip_width;
			region.extent.height = mip_height;
			region.extent.depth = mip_depth;

			img_copy_regions.Add(region);

			// calculate metrics of the next mip
			mip_width = ut::Max<ut::uint32>(mip_width / 2, 1);
			mip_height = ut::Max<ut::uint32>(mip_height / 2, 1);
			mip_depth = ut::Max<ut::uint32>(mip_depth / 2, 1);
		}
	}

	// copy images
	vkCmdCopyImage(cmd_buffer.GetVkHandle(),
	               src_image.GetVkHandle(),
	               src_image.GetLayout(),
	               dst_image.GetVkHandle(),
	               dst_image.GetLayout(),
	               static_cast<uint32_t>(img_copy_regions.Count()),
	               img_copy_regions.GetAddress());
}

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
void Context::ClearTarget(Target& target,
                          ut::Color<4> color,
                          ut::uint32 first_slice,
                          ut::uint32 slice_count,
                          ut::uint32 first_mip,
                          ut::uint32 mip_count)
{
	const Target::Info& info = target.GetInfo();
	Target::Info::State img_state = info.state;
	Image& img = target.GetImage();

	// clear requires transfer_dst state
	SetTargetState(target, Target::Info::State::transfer_dst);

	VkClearColorValue clear_color;
	clear_color.float32[0] = color.R();
	clear_color.float32[1] = color.G();
	clear_color.float32[2] = color.B();
	clear_color.float32[3] = color.A();

	VkImageSubresourceRange range;
	range.aspectMask = img.aspect_mask;
	range.baseMipLevel = first_mip;
	range.levelCount = mip_count == 0 ? VK_REMAINING_MIP_LEVELS : mip_count;
	range.baseArrayLayer = first_slice;
	range.layerCount = slice_count == 0 ? VK_REMAINING_ARRAY_LAYERS : slice_count;

	vkCmdClearColorImage(cmd_buffer.GetVkHandle(),
	                     img.GetVkHandle(),
	                     img.GetLayout(),
	                     &clear_color,
	                     1, &range);

	// set original state back
	SetTargetState(target, img_state);
}

// Uses the largest mipmap level of the provided target to recursively
// generate the lower levels of the mip and stops with the smallest level.
//    @param target - reference to the render target.
//    @param new_state - optional target state to be set after all mip levels
//                       were generated.
void Context::GenerateMips(Target& target,
                           const ut::Optional<Target::Info::State>& new_state)
{
	// skip if this render target has only 1 mip level
	const Target::Info& info = target.GetInfo();
	if (info.mip_count <= 1)
	{
		return;
	}

	// get the number of array layers
	Image& image = target.GetImage();
	const ut::uint32 slice_count = info.type == Image::Type::cubic ? 6 : 1;

	// mips must have different layouts in order to perform vkCmdBlitImage:
	// 1) source mip must have VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	// 2) destination mip must have VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	PlatformImage::State transfer_src_state = PlatformImage::State::CreateForTransferSrc();
	PlatformImage::State transfer_dst_state = PlatformImage::State::CreateForTransferDst();
	ut::Array<ImageTransitionRequest> transition_requests;
	transition_requests.SetCapacityReduction(false);

	// perform vkCmdBlitImage for all mips
	ut::uint32 mip_width = info.width;
	ut::uint32 mip_height = info.height;
	ut::uint32 mip_depth = info.depth;
	for (ut::uint32 mip = 1; mip < info.mip_count; mip++)
	{
		// change destination and source mip layout
		transition_requests.Reset();
		ImageTransitionRequest mip_src_request = { image, mip == 1 ? image.state : transfer_dst_state,
		                                           transfer_src_state, false, 0, slice_count, mip - 1, 1 };
		ImageTransitionRequest mip_dst_request = { image, image.state, transfer_dst_state,
		                                           false, 0, slice_count, mip, 1 };
		transition_requests.Add(mip_src_request);
		transition_requests.Add(mip_dst_request);
		ChangeImageState(transition_requests);

		// perform blit
		VkImageBlit img_blit = {};
		img_blit.srcSubresource.aspectMask = image.aspect_mask;
		img_blit.srcSubresource.baseArrayLayer = 0;
		img_blit.srcSubresource.layerCount = slice_count;
		img_blit.srcSubresource.mipLevel = mip - 1;
		img_blit.srcOffsets[0] = { 0, 0, 0 };
		img_blit.srcOffsets[1].x = static_cast<int32_t>(mip_width);
		img_blit.srcOffsets[1].y = static_cast<int32_t>(mip_height);
		img_blit.srcOffsets[1].z = static_cast<int32_t>(mip_depth);

		mip_width = ut::Max<ut::uint32>(mip_width / 2, 1);
		mip_height = ut::Max<ut::uint32>(mip_height / 2, 1);
		mip_depth = ut::Max<ut::uint32>(mip_depth / 2, 1);

		img_blit.dstSubresource.aspectMask = image.aspect_mask;
		img_blit.dstSubresource.baseArrayLayer = 0;
		img_blit.dstSubresource.layerCount = slice_count;
		img_blit.dstSubresource.mipLevel = mip;
		img_blit.dstOffsets[0] = { 0, 0, 0 };
		img_blit.dstOffsets[1].x = static_cast<int32_t>(mip_width);
		img_blit.dstOffsets[1].y = static_cast<int32_t>(mip_height);
		img_blit.dstOffsets[1].z = static_cast<int32_t>(mip_depth);

		vkCmdBlitImage(cmd_buffer.GetVkHandle(),
		               image.GetVkHandle(),
		               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		               image.GetVkHandle(),
		               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		               1,
		               &img_blit,
		               VK_FILTER_LINEAR);
	}

	// set back the original image state
	PlatformImage::State final_state = new_state ? CreateTargetImageState(info, new_state.Get()) : image.state;
	ImageTransitionRequest first_mips_request = { image, transfer_src_state, final_state,
	                                              true, 0, slice_count, 0, info.mip_count - 1 };
	ImageTransitionRequest last_mip_request = { image, transfer_dst_state, final_state,
	                                            true, 0, slice_count, info.mip_count - 1, 1 };
	transition_requests.Reset();
	transition_requests.Add(first_mips_request);
	transition_requests.Add(last_mip_request);
	ChangeImageState(transition_requests);
}

// Toggles render target's state.
//    @param targets - array of pointers to render targets.
//    @param target_count - the number of elements in @targets array.
//    @param state - the new state.
void Context::SetTargetState(SharedTargetData** targets,
                             ut::uint32 target_count,
                             Target::Info::State state)
{
	ut::Array<ImageTransitionRequest> transition_requests;
	for (ut::uint32 i = 0; i < target_count; i++)
	{
		TargetData& target_data = targets[i]->GetRef();
		const Target::Info target_info = target_data.info;
		if (target_info.state == state)
		{
			continue;
		}

		// choose corresponding image state
		PlatformImage::State new_img_state = CreateTargetImageState(target_info, state);

		// create transition request and update state information
		ImageTransitionRequest request = { target_data.image,
		                                   target_data.image.state,
		                                   new_img_state,
		                                   true,
		                                   0, VK_REMAINING_ARRAY_LAYERS,
		                                   0, VK_REMAINING_MIP_LEVELS };
		transition_requests.Add(request);
		target_data.info.state = state;
	}

	// pipeline barrier
	ChangeImageState(transition_requests);
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
void Context::BeginRenderPass(RenderPass& render_pass,
                              Framebuffer& framebuffer,
                              const ut::Rect<ut::uint32>& render_area,
                              const ClearColor& clear_color,
                              float depth_clear_value,
                              ut::uint32 stencil_clear_value,
                              bool aims_secondary_buffers)
{
	// validate arguments
	if (!clear_color)
	{
		UT_ASSERT(clear_color.GetAlt().Count() <= skMaxClearValues);
		UT_ASSERT(clear_color.GetAlt().Count() == render_pass.color_slots.Count());
	}
	UT_ASSERT(render_pass.color_slots.Count() == framebuffer.color_attachments.Count());
	UT_ASSERT(render_pass.depth_stencil_slot ?
	          static_cast<bool>(framebuffer.depth_stencil_attachment) :
	          !static_cast<bool>(framebuffer.depth_stencil_attachment));
	
	const ut::uint32 color_slot_count = static_cast<ut::uint32>(render_pass.color_slots.Count());
	ut::uint32 total_attachment_count = color_slot_count;

	// color attachments
	VkClearValue vk_clear_values[skMaxClearValues];
	SharedTargetData* attachments[skMaxClearValues];
	for (ut::uint32 i = 0; i < color_slot_count; i++)
	{
		const ut::Color<4>& color = clear_color ?
		                            clear_color.Get() :
		                            clear_color.GetAlt()[i];

		vk_clear_values[i].color.float32[0] = color.R();
		vk_clear_values[i].color.float32[1] = color.G();
		vk_clear_values[i].color.float32[2] = color.B();
		vk_clear_values[i].color.float32[3] = color.A();

		attachments[i] = &framebuffer.color_attachments[i].target;
	}

	// depth-stencil attachment
	if (render_pass.depth_stencil_slot)
	{
		// note that depth-stencil slot goes right after all color slots
		vk_clear_values[color_slot_count].depthStencil.depth = depth_clear_value;
		vk_clear_values[color_slot_count].depthStencil.stencil = stencil_clear_value;
		
		attachments[color_slot_count] = &framebuffer.depth_stencil_attachment->target;

		total_attachment_count = color_slot_count + 1;
	}

	// all attachments must be in 'target' state before render pass begins
	SetTargetState(attachments, total_attachment_count, Target::Info::State::target);

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
	rp_begin.clearValueCount = static_cast<uint32_t>(total_attachment_count);
	rp_begin.pClearValues = vk_clear_values;

	// begin render pass
	VkSubpassContents subpass_content = aims_secondary_buffers ?
	                                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS :
	                                    VK_SUBPASS_CONTENTS_INLINE;
	vkCmdBeginRenderPass(cmd_buffer.GetVkHandle(), &rp_begin, subpass_content);

	// remember render area metrics
	const Framebuffer::Info& framebuffer_info = framebuffer.GetInfo();
	bound_framebuffer_size = ut::Vector<2, ut::uint32>(framebuffer_info.width,
	                                                   framebuffer_info.height);
}

// End current render pass.
void Context::EndRenderPass()
{
	vkCmdEndRenderPass(cmd_buffer.GetVkHandle());
	bound_framebuffer_size = ut::Optional< ut::Vector<2, ut::uint32> >();
}

// Binds provided pipeline state to the current context.
//    @param pipeline_state - reference to the pipeline state.
//    @param viewports - optional reference to an array of viewports.
void Context::BindPipelineState(PipelineState& pipeline_state,
                                ut::Optional<const ut::Array<Viewport>&> viewports)
{
	vkCmdBindPipeline(cmd_buffer.GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_state.pipeline.GetVkHandle());

	UT_ASSERT(bound_framebuffer_size);

	const float framebuffer_width = static_cast<float>(bound_framebuffer_size->X());
	const float framebuffer_height = static_cast<float>(bound_framebuffer_size->Y());

	if (viewports)
	{
		const ut::uint32 viewport_count = static_cast<ut::uint32>(viewports->Count());
		ut::Array<VkViewport> vp(viewport_count);
		ut::Array<VkRect2D> scissors(viewport_count);
		for (uint32_t i = 0; i < viewport_count; i++)
		{
			const Viewport& viewport = viewports.Get()[i];

			vp[i].x = viewport.x;
			vp[i].y = viewport.height - viewport.y;
			vp[i].width = viewport.width;
			vp[i].height = -viewport.height;
			vp[i].minDepth = viewport.min_depth;
			vp[i].maxDepth = viewport.max_depth;

			if (viewport.scissor)
			{
				scissors[i].offset.x = viewport.scissor->offset.X();
				scissors[i].offset.y = viewport.scissor->offset.Y();
				scissors[i].extent.width = viewport.scissor->extent.X();
				scissors[i].extent.height = viewport.scissor->extent.Y();
			}
			else
			{
				scissors[i].offset.x = static_cast<int32_t>(viewport.x);
				scissors[i].offset.y = static_cast<int32_t>(viewport.y);
				scissors[i].extent.width = static_cast<uint32_t>(viewport.width);
				scissors[i].extent.height = static_cast<uint32_t>(viewport.height);
			}
		}

		vkCmdSetViewport(cmd_buffer.GetVkHandle(), 0, viewport_count, vp.GetAddress());
		vkCmdSetScissor(cmd_buffer.GetVkHandle(), 0, viewport_count, scissors.GetAddress());
	}
	else
	{
		VkViewport vp;
		vp.x = 0.0f;
		vp.y = framebuffer_height;
		vp.width = framebuffer_width;
		vp.height = -framebuffer_height;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vkCmdSetViewport(cmd_buffer.GetVkHandle(), 0, 1, &vp);

		VkRect2D scissor;
		scissor.extent.width = bound_framebuffer_size->X();
		scissor.extent.height = bound_framebuffer_size->Y();
		scissor.offset.x = 0;
		scissor.offset.y = bound_framebuffer_size->Y() - scissor.extent.height;
		vkCmdSetScissor(cmd_buffer.GetVkHandle(), 0, 1, &scissor);
	}

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

// Binds vertex and instance buffers to the current context.
//    @param vertex_buffer - reference to the vertex buffer to be bound.
//    @param vertex_offset - number of bytes between the first element
//                           of a vertex buffer and the first element
//                           that will be used.
//    @param instance_buffer - reference to the instance buffer to be bound.
//    @param instance_offset - number of bytes between the first element
//                             of an instance buffer and the first element
//                             that will be used.
void Context::BindVertexAndInstanceBuffer(Buffer& vertex_buffer,
                                          size_t vertex_offset,
                                          Buffer& instance_buffer,
                                          size_t instance_offset)
{
	VkBuffer buffers[] = { vertex_buffer.GetVkHandle(), instance_buffer.GetVkHandle() };
	VkDeviceSize offsets[] = { vertex_offset, instance_offset };
	vkCmdBindVertexBuffers(cmd_buffer.GetVkHandle(), 0, 2, buffers, offsets);
}

// Binds index buffer to the current context.
//    @param buffer - reference to the buffer to be bound.
//    @param offset - number of bytes between the first element
//                    of an index buffer and the first index
//                    that will be used.
//    @param index_type - type of index buffer indices (16 or 32).
void Context::BindIndexBuffer(Buffer& buffer,
                              size_t offset,
                              IndexType index_type)
{
	vkCmdBindIndexBuffer(cmd_buffer.GetVkHandle(),
	                     buffer.GetVkHandle(),
	                     offset,
	                     index_type == IndexType::uint32 ?
	                     VK_INDEX_TYPE_UINT32 :
	                     VK_INDEX_TYPE_UINT16);
}

// Draw non-indexed, non-instanced primitives.
//    @param vertex_count - number of vertices to draw.
//    @param first_vertex_id - index of the first vertex.
void Context::Draw(ut::uint32 vertex_count, ut::uint32 first_vertex_id)
{
	vkCmdDraw(cmd_buffer.GetVkHandle(), vertex_count, 1, first_vertex_id, 0);
}

// Draw non-indexed, instanced primitives.
//    @param vertex_count - number of vertices to draw.
//    @param instance_count - number of instances to draw.
//    @param first_vertex_id - index of the first vertex.
//    @param first_instance_id - a value added to each index before reading
//                               per-instance data from a vertex buffer.
void Context::DrawInstanced(ut::uint32 vertex_count,
                            ut::uint32 instance_count,
                            ut::uint32 first_vertex_id,
                            ut::uint32 first_instance_id)
{
	vkCmdDraw(cmd_buffer.GetVkHandle(),
	          vertex_count,
	          instance_count,
	          first_vertex_id,
	          first_instance_id);
}

// Draw indexed, non-instanced primitives.
//    @param index_count - number of vertices to draw.
//    @param first_index_id - the base index within the index buffer.
//    @param vertex_offset - the value added to the vertex index before
//                           indexing into the vertex buffer.
void Context::DrawIndexed(ut::uint32 index_count,
                          ut::uint32 first_index_id,
                          ut::int32 vertex_offset)
{
	vkCmdDrawIndexed(cmd_buffer.GetVkHandle(),
	                 index_count,
	                 1,
	                 first_index_id,
	                 vertex_offset,
	                 0);
}

// Draw indexed, instanced primitives.
//    @param index_count - number of vertices to draw.
//    @param instance_count - number of instances to draw.
//    @param first_index_id - the base index within the index buffer.
//    @param vertex_offset - the value added to the vertex index before
//                           indexing into the vertex buffer.
//    @param first_instance_id - a value added to each index before reading
//                               per-instance data from a vertex buffer.
void Context::DrawIndexedInstanced(ut::uint32 index_count,
                                   ut::uint32 instance_count,
                                   ut::uint32 first_index_id,
                                   ut::int32 vertex_offset,
                                   ut::uint32 first_instance_id)
{
	vkCmdDrawIndexed(cmd_buffer.GetVkHandle(),
	                 index_count,
	                 instance_count,
	                 first_index_id,
	                 vertex_offset,
	                 first_instance_id);
}

// Executes provided secondary command buffers. Note that this function
// does nothing if this context is writing to the non-primary command buffer.
//    @param buffers - array of references to the secondary buffer.
void Context::ExecuteSecondaryBuffers(ut::Array< ut::Ref<CmdBuffer> >& buffers)
{
	const ut::uint32 count = static_cast<ut::uint32>(buffers.Count());

	ut::Array<VkCommandBuffer> handles(count);
	for (ut::uint32 i = 0; i < count; i++)
	{
		handles[i] = buffers[i]->GetVkHandle();
	}

	vkCmdExecuteCommands(cmd_buffer.GetVkHandle(), count, handles.GetAddress());
}

// Write a query object.
//    @param query_buffer - a reference to the buffer containing the
//                          desired query.
//    @param query_id - an index of the query in @query_buffer to be written.
//    @param stage - the pipeline stage for writing a query.
void Context::WriteQuery(QueryBuffer& query_buffer,
                         ut::uint32 query_id,
                         QueryBuffer::PipelineStage stage)
{
	UT_ASSERT(query_id < query_buffer.GetInfo().count);

	const VkPipelineStageFlagBits stage_flags = stage == QueryBuffer::PipelineStage::top_of_pipe ?
	                                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT :
	                                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	vkCmdWriteTimestamp(cmd_buffer.GetVkHandle(),
	                    stage_flags,
	                    query_buffer.GetVkHandle(),
	                    query_id);
}

// Returns the results for the provided query buffer.
//    @param query_buffer - a buffer containing the queries.
//    @param first_query - the initial query index.
//    @param count - the number of queries to read, all queries in the
//                   @query_buffer starting from @first_query will be read
//                   if this parameter is empty.
ut::Array<ut::uint64> Context::ReadAndResetQueryBuffer(QueryBuffer& query_buffer,
                                                       ut::uint32 first_query,
                                                       ut::Optional<ut::uint32> count)
{
	const QueryBuffer::Info& info = query_buffer.GetInfo();

	const ut::uint32 remaining_query_count = count ? count.Get() : (info.count - first_query);

	ut::Array<ut::uint64> result(remaining_query_count);
	
	if (remaining_query_count > 0)
	{
		VkResult res = vkGetQueryPoolResults(device,
		                                     query_buffer.GetVkHandle(),
		                                     first_query,
		                                     remaining_query_count,
		                                     result.GetSize(),
		                                     result.GetAddress(),
		                                     sizeof(ut::uint64),
		                                     VK_QUERY_RESULT_64_BIT);

		UT_ASSERT(res == VK_SUCCESS || res == VK_NOT_READY);
	}

	for (ut::uint64& timestamp : result)
	{
		timestamp = static_cast<ut::uint64>(timestamp * timestamp_period);
	}

	vkCmdResetQueryPool(cmd_buffer.GetVkHandle(),
	                    query_buffer.GetVkHandle(),
	                    0,
	                    info.count);

	return result;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//