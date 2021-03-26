//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan image.
class PlatformImage : public VkRc<vk::image>
{
	friend class Device;
	friend class PlatformContext;
	friend class Context;
public:
	// Current state of this image. All changeable image
	// options are collected here.
	struct State
	{
		static State CreateForColorTarget();
		static State CreateForDepthStencilTarget();
		static State CreateForShaderResource();
		static State CreateForTransferSrc();
		static State CreateForTransferDst();

		State(VkImageLayout in_layout,
		      VkAccessFlags in_access_mask,
		      VkPipelineStageFlags in_stages);

		VkImageLayout layout;
		VkAccessFlags access_mask;
		VkPipelineStageFlags stages;
	};

	// Constructor.
	PlatformImage(VkDevice device_handle,
	              VkImage image_handle,
	              VkImageView image_view,
	              VkImageView* cube_faces_arr, // pass nullptr if not a cubemap
	              VkRc<vk::memory> memory_rc,
	              VkImageAspectFlags aspect_flags,
	              const State& image_state);

	// Move constructor.
	PlatformImage(PlatformImage&&) noexcept;

	// Move operator.
	PlatformImage& operator =(PlatformImage&&) noexcept;

	// Copying is prohibited.
	PlatformImage(const PlatformImage&) = delete;
	PlatformImage& operator =(const PlatformImage&) = delete;

	// Returns image layout.
	VkImageLayout GetLayout() const
	{
		return state.layout;
	}

	// Returns image view.
	VkImageView GetView() const
	{
		return view.GetVkHandle();
	}

	// Returns view of the specified cube face.
	VkImageView GetCubeFaceView(ut::uint32 face_id) const
	{
		UT_ASSERT(face_id < 6);
		return cube_faces[face_id].GetVkHandle();
	}

private:
	VkImageAspectFlags aspect_mask;
	VkRc<vk::image_view> view;
	VkRc<vk::image_view> cube_faces[6];
	VkRc<vk::memory> memory;
	State state;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//