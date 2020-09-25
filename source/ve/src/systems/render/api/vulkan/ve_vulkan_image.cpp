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
                             VkImageLayout image_layout,
                             VkRc<vk::memory> memory_rc) : VkRc<vk::image>(image_handle, device_handle)
                                                         , view(image_view, device_handle)
                                                         , layout(image_layout)
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
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//