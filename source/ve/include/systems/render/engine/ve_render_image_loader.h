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
        Info() {}
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
	ut::Result<Image, ut::Error> LoadFromFile(const ut::String& filename,
	                                          const Info& info = Info()) const;

	// Loads an image from memory.
	//    @param data - a byte array containing image data.
	//    @param info - const reference to the ImageLoader::Info object.
	//    @return - new render::Image object or ut::Error if failed.
	ut::Result<Image, ut::Error> LoadFromMemory(const ut::Array<ut::byte>& data,
	                                            const Info& info = Info()) const;

	// Returns the number of mips in a mip set for the desired metrics.
	//    @param width - width of the first mip in pixels.
	//    @param height - height of the first mip in pixels.
	//    @param depth - depth of the first mip in pixels.
	//    @return - number of mips.
	static ut::uint32 CountMips(ut::uint32 width = 1,
	                            ut::uint32 height = 1,
	                            ut::uint32 depth = 1);

	// Generates mip tail using averaging nearest source pixels into
	// the one destination pixel. Fast, but quality is low. Can be used
	// for 1D, 2D (except cubemaps) and 3D maps.
	//    @param data - array of pixel data with initialized first mip, it's
	//                  better to provide sufficient space for all mips,
	//                  however this array will be expanded internally to
	//                  accomodate all mips anyway.
	//    @param width - width of the first mip in pixels.
	//    @param height - height of the first mip in pixels.
	// 	  @param depth - depth of the first mip in pixels.
	//    @param mip_count - number of mips in a tail.
	//    @return - optional ut::Error if failed.
	template<int channel_count = 4,
	         typename ChannelType = ut::byte,
             typename AvgPrecisionType = double>
	static ut::Optional<ut::Error> GenerateMipTail(ut::Array<ut::byte>& data,
	                                               ut::uint32 width,
	                                               ut::uint32 height,
	                                               ut::uint32 depth,
	                                               ut::uint32 mip_count)
	{
		constexpr ut::uint32 pixel_size = sizeof(ut::Color<channel_count, ChannelType>);

		// check if data has enough pixels for the first mip
		if (data.GetSize() < width * height * depth * pixel_size)
		{
			return ut::Error(ut::error::out_of_bounds);
		}

		// calculate metrics of the second mip
		ut::uint32 prev_mip = 0;
		ut::uint32 mip_offset = width * height * depth * pixel_size;
		ut::uint32 mip_width = ut::Max<ut::uint32>(width / 2, 1);
		ut::uint32 mip_height = ut::Max<ut::uint32>(height / 2, 1);
		ut::uint32 mip_depth = ut::Max<ut::uint32>(depth / 2, 1);

		// process all mips starting from the second
		for (ut::uint32 mip = 1; mip < mip_count; mip++)
		{
			// calculate mip size and resize image data buffer
			const ut::uint32 mip_size = mip_width * mip_height * mip_depth * pixel_size;
			const size_t min_expected_size = mip_offset + mip_size;
			if (data.GetSize() < min_expected_size)
			{
				data.Resize(min_expected_size);
			}

			// iterate dimensions to calculate average color of nearest pixels
			for (ut::uint32 z = 0; z < mip_depth; z++)
			{
				const ut::uint32 dst_layer_offset = z * mip_width * mip_height;
				const ut::uint32 src_layer_size = width * height;
				const ut::uint32 src_layer_id = z * 2;				

				for (ut::uint32 y = 0; y < mip_height; y++)
				{
					const ut::uint32 dst_row_offset = dst_layer_offset + y * mip_width;
					const ut::uint32 src_row_size = width;
					const ut::uint32 src_row_id = y * 2;

					for (ut::uint32 x = 0; x < mip_width; x++)
					{
						// find the destination pixel 
						const ut::uint32 dst_offset = mip_offset + (dst_row_offset + x) * pixel_size;
						ut::Color<channel_count, ChannelType>& dst = *reinterpret_cast<ut::Color<channel_count, ChannelType>*>(&data[dst_offset]);

						// calculate average separately for each channel
						const ut::uint32 src_column_id = x * 2;
						for (ut::uint32 channel_id = 0; channel_id < channel_count; channel_id++)
						{
							AvgPrecisionType sum = static_cast<AvgPrecisionType>(0.0);

							for (ut::uint32 src_z = 0; src_z < 2; src_z++)
							{
								for (ut::uint32 src_y = 0; src_y < 2; src_y++)
								{
									for (ut::uint32 src_x = 0; src_x < 2; src_x++)
									{
										const ut::uint32 src_offset = ut::Min(src_layer_id + src_z, depth - 1) * src_layer_size +
										                              ut::Min(src_row_id + src_y, height - 1) * src_row_size +
										                              ut::Min(src_column_id + src_x, width - 1);
										ut::Color<channel_count, ChannelType>& src =
											*reinterpret_cast<ut::Color<channel_count, ChannelType>*>(&data[prev_mip + src_offset * pixel_size]);
										sum += static_cast<AvgPrecisionType>(src(0, channel_id));
									}
								}
							}

							dst(0, channel_id) = static_cast<ChannelType>(sum / static_cast<AvgPrecisionType>(8.0));
						}
					}
				}
			}

			// calculate data for the next mip
			prev_mip = mip_offset;
			width = mip_width;
			height = mip_height;
			depth = mip_depth;
			mip_offset += mip_size;
			mip_width = ut::Max<ut::uint32>(mip_width / 2, 1);
			mip_height = ut::Max<ut::uint32>(mip_height / 2, 1);
			mip_depth = ut::Max<ut::uint32>(mip_depth / 2, 1);
		}

		// success
		return ut::Optional<ut::Error>();
	}

	// Generates 2D mip tail using stb downscaling. Slow, but quality is good.
	// Accepts only R8G8B8A8 4 bytes per pixel data.
	//    @param data - array of pixel data with initialized first mip, it's
	//                  better to provide sufficient space for all mips,
	//                  however this array will be expanded internally to
	//                  accomodate all mips anyway.
	//    @param width - width of the first mip in pixels.
	//    @param height - height of the first mip in pixels.
	//    @param mip_count - number of mips in a tail.
	//    @return - optional ut::Error if failed.
	static ut::Optional<ut::Error> GenerateHighQualityMipTail2D(ut::Array<ut::byte>& data,
	                                                            ut::uint32 width,
	                                                            ut::uint32 height,
	                                                            ut::uint32 mip_count);

private:
	// Creates an image from intermediate decoded data.
	//    @param data - a pointer to the data to be loaded.
	// 	  @param width - width in pixels of the decoded image.
	// 	  @param height - height in pixels of the decoded image.
	// 	  @param channel_count - the number of channels in the decoded image.
	//    @param info - const reference to the ImageLoader::Info object
	//                  describing the final image.
	//    @return - new render::Image object or ut::Error if failed.
	ut::Result<Image, ut::Error> CreateImage(const ut::byte* data,
	                                         int width,
	                                         int height,
	                                         int channel_count,
	                                         const Info& info) const;

	// Representation of a pixel of the image being loaded.
	typedef ut::Color<4, ut::byte> Pixel;

	// reference to the render device
	Device& device;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
