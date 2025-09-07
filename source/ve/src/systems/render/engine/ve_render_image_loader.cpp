//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_image_loader.h"
#include "stb_image.h"
#include "stb_image_resize.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
struct StbiDeleter
{
	void operator()(stbi_uc* ptr) const
	{
		stbi_image_free(ptr);
	}
};

//----------------------------------------------------------------------------//
// Constructor.
ImageLoader::ImageLoader(Device& device_ref) noexcept : device(device_ref)
{}

//----------------------------------------------------------------------------->
// Returns the number of mips in a full mip set for the desired metrics.
//    @param width - width of the first mip in pixels.
//    @param height - height of the first mip in pixels.
//    @param depth - depth of the first mip in pixels.
//    @return - number of mips.
ut::uint32 ImageLoader::CountMips(ut::uint32 width,
                                  ut::uint32 height,
                                  ut::uint32 depth)
{
	const float max_size = static_cast<float>(ut::Max(ut::Max(width, height), depth));
	return static_cast<ut::uint32>(ut::Logarithm2<float>(max_size)) + 1;
}

//----------------------------------------------------------------------------->
// Loads an image from file.
//    @param filename - path to the image to be loaded.
//    @param info - const reference to the ImageLoader::Info object.
//    @return - new render::Image object or ut::Error if failed.
ut::Result<Image, ut::Error> ImageLoader::Load(const ut::String& filename,
                                               const ImageLoader::Info& info)
{
	// get file path
	ut::Optional<ut::String> path = FindResourceFile(filename);
	if (!path)
	{
		ut::log.Lock() << "Render: image file " << filename << " not found." << ut::cret;
		return ut::MakeError(ut::error::not_found);
	}

	// load image
	int width, height, channels;
	ut::UniquePtr<stbi_uc, StbiDeleter> pixels(stbi_load(path->GetAddress(),
	                                                     &width,
	                                                     &height,
	                                                     &channels,
	                                                     STBI_rgb_alpha));
	if (!pixels)
	{
		return ut::MakeError(ut::error::fail, ut::String("Render: failed to load ") + filename);
	}

	// check number of channels
	if (channels > 4)
	{
		ut::String err_desc = ut::String("Render: error while loading \"") +
		                      filename + "\": too much channels";
		return ut::MakeError(ut::error::not_supported, ut::Move(err_desc));
	}

	// initialize image info
	Image::Info img_info;
	img_info.type = Image::Type::planar;
	img_info.format = info.srgb ?
	                  pixel::Format::r8g8b8a8_srgb :
	                  pixel::Format::r8g8b8a8_unorm;
	img_info.usage = render::memory::Usage::gpu_immutable;
	img_info.width = info.width ? info.width.Get() : width;
	img_info.height = info.height ? info.height.Get() : height;
	img_info.depth = 1;
	img_info.mip_count = info.mip_count ? info.mip_count.Get() :
	                     CountMips(img_info.width, img_info.height, 1);

	// allocate enough memory for the mip tail
	const ut::uint32 first_mip_size = img_info.width * img_info.height *
	                                  pixel::GetSize(img_info.format);
	const bool has_mip_tail = img_info.mip_count > 1;
	const ut::uint32 mip_tail_max_size = has_mip_tail ? (first_mip_size / 2 + first_mip_size / 4) : 0;
	img_info.data.Resize(first_mip_size + mip_tail_max_size);

	// initialize first mip
	if (img_info.width != width || img_info.height != height)
	{
		stbir_resize_uint8(pixels.Get(),
			               width,
			               height,
			               0,
		                   img_info.data.GetAddress(),
			               img_info.width,
			               img_info.height,
			               0,
		                   STBI_rgb_alpha);
	}
	else
	{
		const ut::uint32 first_mip_size = img_info.width * img_info.height * 4;
		ut::memory::Copy(img_info.data.GetAddress(), pixels.Get(), first_mip_size);
	}

	// generate other mips
	if (info.high_quality_mips)
	{
		GenerateHighQualityMipTail2D(img_info.data,
		                             img_info.width,
		                             img_info.height,
		                             img_info.mip_count);
	}
	else
	{
		GenerateMipTail<4, ut::byte, ut::uint32>(img_info.data,
		                                         img_info.width,
		                                         img_info.height,
		                                         1,
		                                         img_info.mip_count);
	}

	// success
	return device.CreateImage(ut::Move(img_info));
}

//----------------------------------------------------------------------------//
// Generates mip tail using stb downscaling. Slow, but quality is good.
//    @param data - array of pixel data with initialized first mip, it's
//                  better to provide sufficient space for all mips,
//                  however this array will be expanded internally to
//                  accomodate all mips anyway.
//    @param width - width of the first mip in pixels.
//    @param height - height of the first mip in pixels.
//    @param mip_count - number of mips in a tail.
//    @return - optional ut::Error if failed.
ut::Optional<ut::Error> ImageLoader::GenerateHighQualityMipTail2D(ut::Array<ut::byte>& data,
                                                                  ut::uint32 width,
                                                                  ut::uint32 height,
                                                                  ut::uint32 mip_count)
{
	// check if data has enough pixels for the first mip
	if (data.GetSize() < width * height * sizeof(Pixel))
	{
		return ut::Error(ut::error::out_of_bounds);
	}

	// calculate metrics of the second mip
	ut::uint32 prev_mip = 0;
	ut::uint32 prev_mip_w = width;
	ut::uint32 prev_mip_h = height;
	ut::uint32 mip_width = width / 2;
	ut::uint32 mip_height = height / 2;
	ut::uint32 mip_offset = width * height * sizeof(Pixel);

	// process all mips starting from the second
	for (ut::uint32 i = 1; i < mip_count; i++)
	{
		const ut::uint32 mip_size = mip_width * mip_height * sizeof(Pixel);
		const size_t min_expected_size = mip_offset + mip_size;
		if (data.GetSize() < min_expected_size)
		{
			data.Resize(min_expected_size);
		}

		stbir_resize_uint8(&data[prev_mip], prev_mip_w, prev_mip_h, 0,
		                   &data[mip_offset], mip_width, mip_height, 0,
		                   STBI_rgb_alpha);

		prev_mip = mip_offset;
		prev_mip_w = mip_width;
		prev_mip_h = mip_height;

		mip_offset += mip_size;
		mip_width /= 2;
		mip_height /= 2;
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//