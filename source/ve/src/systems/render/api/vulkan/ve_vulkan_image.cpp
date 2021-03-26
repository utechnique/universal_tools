//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_image.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformImage::PlatformImage(VkDevice device_handle,
                             VkImage image_handle,
                             VkImageView image_view,
                             VkImageView* cube_faces_arr,
                             VkRc<vk::memory> memory_rc,
                             VkImageAspectFlags aspect_flags,
                             const State& image_state) : VkRc<vk::image>(image_handle, device_handle)
                                                       , view(image_view, device_handle)
                                                       , aspect_mask(aspect_flags)
                                                       , state(image_state)
                                                       , memory(ut::Move(memory_rc))
{
	if (cube_faces_arr)
	{
		for (ut::uint32 i = 0; i < 6; i++)
		{
			VkRc<vk::image_view> face(cube_faces_arr[i], device_handle);
			cube_faces[i] = ut::Move(face);
		}
	}
}

// Move constructor.
PlatformImage::PlatformImage(PlatformImage&&) noexcept = default;

// Move operator.
PlatformImage& PlatformImage::operator =(PlatformImage&&) noexcept = default;

//----------------------------------------------------------------------------//
// Creates image state that is common for all colored render targets.
PlatformImage::State PlatformImage::State::CreateForColorTarget()
{
	return State(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	             VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
	             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

// Creates image state that is common for all depth stencil targets.
PlatformImage::State PlatformImage::State::CreateForDepthStencilTarget()
{
	return State(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
	             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
}

// Creates image state that is common for all shader resources.
PlatformImage::State PlatformImage::State::CreateForShaderResource()
{
	return State(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	             VK_ACCESS_SHADER_READ_BIT,
	             VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
	             VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
	             VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
	             VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
	             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
	             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

// Creates image state that makes possible to use it as a transfer source.
PlatformImage::State PlatformImage::State::CreateForTransferSrc()
{
	return State(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	             VK_ACCESS_TRANSFER_READ_BIT,
	             VK_PIPELINE_STAGE_TRANSFER_BIT);
}

// Creates image state that makes possible to use it as a transfer destination.
PlatformImage::State PlatformImage::State::CreateForTransferDst()
{
	return State(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	             VK_ACCESS_TRANSFER_WRITE_BIT,
	             VK_PIPELINE_STAGE_TRANSFER_BIT);
}

// Constructor.
PlatformImage::State::State(VkImageLayout in_layout,
                            VkAccessFlags in_access_mask,
                            VkPipelineStageFlags in_stages) : layout(in_layout)
                                                            , access_mask(in_access_mask)
                                                            , stages(in_stages)
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//