//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
#include "systems/render/api/vulkan/ve_vulkan_cmd_buffer.h"
#include "systems/render/api/vulkan/ve_vulkan_image.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan context.
class PlatformContext
{
	friend class Device;
public:
	// Constructor.
	PlatformContext(VkDevice device_handle,
	                PlatformCmdBuffer& cmd_buffer_ref);

	// Move constructor.
	PlatformContext(PlatformContext&&) noexcept;

	// Move operator is prohibited.
	PlatformContext& operator =(PlatformContext&&) noexcept = delete;

	// Copying is prohibited.
	PlatformContext(const PlatformContext&) = delete;
	PlatformContext& operator =(const PlatformContext&) = delete;

protected:
	// Image transitions are grouped by the combination of stage transitions.
	// If a set of flags for old and new stages is the same for multiple
	// images - transition can be batched in one vkCmdPipelineBarrier call.
	struct ImageTransitionGroup
	{
		ut::Array<VkImageMemoryBarrier> barriers;
		VkPipelineStageFlags old_stage;
		VkPipelineStageFlags new_stage;
	};

	// Contains an image willing to change its state and the state itself.
	struct ImageTransitionRequest
	{
		PlatformImage& image;
		PlatformImage::State new_state;
	};

	// Performs image layout transition.
	//    @param requests - array of transition requests.
	//    @param cmd_buffer - command buffer handle to record transition command.
	static void ChangeImageState(ut::Array<ImageTransitionRequest>& requests,
	                             VkCommandBuffer cmd_buffer);

	VkDevice device;
	PlatformCmdBuffer& cmd_buffer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//