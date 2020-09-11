//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ImageLoader encapsulates loading images from file.
class ImageLoader
{
public:
	// Helper struct to control loading of an image.
	// Any of it's members can be modified to override appropriate
	// value on loading.
	struct Info
	{
		ut::Optional<ut::uint32> width;
		ut::Optional<ut::uint32> height;
		ut::Optional<ut::uint32> mip_count;
		bool srgb = false;
		bool high_quality_mips = false;
	};

	// Constructor.
	ImageLoader(Device& device_ref) noexcept;

	// Loads an image from file.
	//    @param filename - path to the image to be loaded.
	//    @param info - const reference to the ImageLoader::Info object.
	//    @return - new render::Image object or ut::Error if failed.
	ut::Result<Image, ut::Error> Load(const ut::String& filename,
	                                  const Info& info = Info());

private:
	// Representation of a pixel of the image being loaded.
	typedef ut::Color<4, ut::byte> Pixel;

	// Generates mip tail using averaging 4 source pixels into
	// the one destination pixel. Fast, but quality is low.
	//    @param data - image data with initialized first mip and
	//                  empty space for the mip tail.
	//    @param width - width of the first mip in pixels.
	//    @param height - height of the first mip in pixels.
	//    @param mip_count - number of mips in a tail.
	void GenerateMipTail4x1(ut::Array<ut::byte>& data,
	                        ut::uint32 width,
	                        ut::uint32 height,
	                        ut::uint32 mip_count);

	// Generates mip tail using stb downscaling. Slow, but quality is good.
	//    @param data - image data with initialized first mip and
	//                  empty space for the mip tail.
	//    @param width - width of the first mip in pixels.
	//    @param height - height of the first mip in pixels.
	//    @param mip_count - number of mips in a tail.
	void GenerateHighQualityMipTail(ut::Array<ut::byte>& data,
	                                ut::uint32 width,
	                                ut::uint32 height,
	                                ut::uint32 mip_count);

	// reference to the render device
	Device& device;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
