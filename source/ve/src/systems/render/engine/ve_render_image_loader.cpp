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
	ut::UniquePtr<stbi_uc, StbiDeleter> pixels(stbi_load(path->ToCStr(),
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
	img_info.type = Image::type_2D;
	img_info.format = info.srgb ? pixel::r8g8b8a8_srgb : pixel::r8g8b8a8;
	img_info.usage = render::memory::gpu_immutable;
	img_info.width = info.width ? info.width.Get() : width;
	img_info.height = info.height ? info.height.Get() : height;
	img_info.depth = 1;
	img_info.mip_count = info.mip_count ? info.mip_count.Get() : 0;

	// calculate mip data
	ut::uint32 mip_width = img_info.width;
	ut::uint32 mip_height = img_info.height;
	ut::uint32 mip_id = 0;
	ut::uint32 total_size = 0;
	while (img_info.mip_count == 0 || mip_id < img_info.mip_count)
	{
		total_size += mip_width * mip_height;

		mip_width /= 2;
		mip_height /= 2;
		mip_id++;

		if (mip_width == 0 || mip_height == 0)
		{
			break;
		}
	}

	img_info.mip_count = mip_id;
	img_info.data.Resize(total_size * 4);

	// create first mip
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
		GenerateHighQualityMipTail(img_info.data,
		                           img_info.width,
		                           img_info.height,
		                           img_info.mip_count);
	}
	else
	{
		GenerateMipTail4x1(img_info.data,
		                   img_info.width,
		                   img_info.height,
		                   img_info.mip_count);
	}

	// success
	return device.CreateImage(ut::Move(img_info));
}

//----------------------------------------------------------------------------//
// Generates mip tail using averaging 4 source pixels into
// the one destination pixel. Fast, but quality is low.
//    @param data - image data with initialized first mip and
//                  empty space for the mip tail.
//    @param width - width of the first mip in pixels.
//    @param height - height of the first mip in pixels.
//    @param mip_count - number of mips in a tail.
void ImageLoader::GenerateMipTail4x1(ut::Array<ut::byte>& data,
                                     ut::uint32 width,
                                     ut::uint32 height,
                                     ut::uint32 mip_count)
{
	ut::uint32 prev_mip = 0;
	ut::uint32 mip_offset = width * height * sizeof(Pixel);
	ut::uint32 mip_width = width / 2;
	ut::uint32 mip_height = height / 2;

	for (ut::uint32 mip = 1; mip < mip_count; mip++)
	{
		for (ut::uint32 i = 0; i < mip_height; i++)
		{
			// calculate offest
			const ut::uint32 dst_row_offset = mip_offset + i * mip_width * sizeof(Pixel);
			const ut::uint32 src_row_offset_up = prev_mip + i * 2 * mip_width * 2 * sizeof(Pixel);
			const ut::uint32 src_row_offset_down = prev_mip + (i * 2 + 1) * mip_width * 2 * sizeof(Pixel);

			for (ut::uint32 j = 0; j < mip_width; j++)
			{
				const ut::uint32 column_offset = j * sizeof(Pixel);

				// calculate pointers to the source and destination pixels
				Pixel* dst = reinterpret_cast<Pixel*>(&data[dst_row_offset + column_offset]);
				Pixel* prev_up = reinterpret_cast<Pixel*>(&data[src_row_offset_up + column_offset * 2]);
				Pixel* prev_down = reinterpret_cast<Pixel*>(&data[src_row_offset_down + column_offset * 2]);

				// combine 4 source pixels into 1 dst pixel
				ut::uint32 r = prev_up->R() + prev_up[1].R() + prev_down->R() + prev_down[1].R();
				ut::uint32 g = prev_up->G() + prev_up[1].G() + prev_down->G() + prev_down[1].G();
				ut::uint32 b = prev_up->B() + prev_up[1].B() + prev_down->B() + prev_down[1].B();
				ut::uint32 a = prev_up->A() + prev_up[1].A() + prev_down->A() + prev_down[1].A();
				*dst = Pixel(r / 4, g / 4, b / 4, a / 4);
			}
		}

		prev_mip = mip_offset;
		mip_offset += mip_width * mip_height * sizeof(Pixel);
		mip_width /= 2;
		mip_height /= 2;
	}
}

//----------------------------------------------------------------------------//
// Generates mip tail using stb downscaling. Slow, but quality is good.
//    @param data - image data with initialized first mip and
//                  empty space for the mip tail.
//    @param width - width of the first mip in pixels.
//    @param height - height of the first mip in pixels.
//    @param mip_count - number of mips in a tail.
void ImageLoader::GenerateHighQualityMipTail(ut::Array<ut::byte>& data,
                                             ut::uint32 width,
                                             ut::uint32 height,
                                             ut::uint32 mip_count)
{
	
	ut::uint32 prev_mip = 0;
	ut::uint32 prev_mip_w = width;
	ut::uint32 prev_mip_h = height;
	ut::uint32 mip_width = width / 2;
	ut::uint32 mip_height = height / 2;
	ut::uint32 mip_offset = width * height * sizeof(Pixel);
	for (ut::uint32 i = 1; i < mip_count; i++)
	{
		ut::uint32 mip_size = mip_width * mip_height * sizeof(Pixel);

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
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//