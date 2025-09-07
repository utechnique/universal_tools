//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "systems/render/engine/ve_render_resource.h"
#include "systems/render/engine/ve_render_image_loader.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Represents an image as a render resource.
class Map : public Image, public Resource
{
public:
	// Constructor.
	Map(Image in_img);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const override;
};

//----------------------------------------------------------------------------//
// This is the specialized version of the ve::render::ResourceCreator template
// class for ve::render::Map resources. It's capable of generating simple maps.
template<> class ResourceCreator<Map>
{
public:
	// This structure is used to initialize a map with pixel data.
	struct InitInfo
	{
		Image::Type img_type = Image::Type::planar;
		pixel::Format format = pixel::Format::r8g8b8a8_unorm;
		ut::uint32 width = 1;
		ut::uint32 height = 1;
		ut::uint32 depth = 1;
		ut::uint32 mip_count = 0;
		bool generate_mips = true;
		ut::Optional<ut::String> name;
	};

	// Constructor initializes an image loader and a hashmap of color names.
	ResourceCreator(Device& device, ResourceManager& rc_mgr);

	// Creates a map according to the provided name, path or generator prompt.
	ut::Result<RcRef<Map>, ut::Error> Create(const ut::String& name);

	// Generates a map initialized with provided color data.
	//    @param init_info - reference to the
	//                       ve::render::ResourceCreator<Map>::InitInfo object.
	//    @param data - reference to the array with pixel data for the first mip.
	//    @return - resource reference or ut::Error if failed.
	template<int channel_count = 4,
	         typename ChannelType = ut::byte>
	ut::Result<RcRef<Map>, ut::Error> CreateFromData(const InitInfo& init_info,
	                                                 const ut::Array< ut::Color<channel_count,
	                                                                            ChannelType> >& data)
	{
		constexpr ut::uint32 dst_pixel_size = sizeof(ut::Color<channel_count, ChannelType>);
		const ut::uint32 src_pixel_size = pixel::GetSize(init_info.format);
		if (dst_pixel_size != src_pixel_size)
		{
			return ut::MakeError(ut::error::types_not_match);
		}

		// initialize image info
		Image::Info img_info;
		img_info.type = init_info.img_type;
		img_info.format = init_info.format;
		img_info.usage = render::memory::Usage::gpu_immutable;
		img_info.width = init_info.width;
		img_info.height = init_info.height;
		img_info.depth = init_info.depth;
		img_info.mip_count = init_info.mip_count == 0 ?
		                     ImageLoader::CountMips(img_info.width,
		                                            img_info.height,
		                                            img_info.depth) :
		                     init_info.mip_count;

		// allocate and copy pixel data
		img_info.data.Resize(data.Count() * dst_pixel_size);
		ut::memory::Copy(img_info.data.GetAddress(),
		                 data.GetAddress(),
		                 img_info.data.GetSize());

		// generate mips
		if (init_info.generate_mips)
		{
			ut::Optional<ut::Error> mip_gen_error = ImageLoader::GenerateMipTail<channel_count,
			                                                                     ChannelType>(img_info.data,
			                                                                                  img_info.width,
			                                                                                  img_info.height,
			                                                                                  img_info.depth,
			                                                                                  img_info.mip_count);
			if (mip_gen_error)
			{
				return ut::MakeError(mip_gen_error.Move());
			}
		}

		// create final gpu image
		ut::Result<Image, ut::Error> image = device.CreateImage(ut::Move(img_info));
		if (!image)
		{
			return ut::MakeError(image.MoveAlt());
		}

		// add this map as a resource
		return AddImgToRcMgr(image.Move(), init_info.name);
	}

	// Returns a hashmap of color names. Available colors are:
	// Red colors:
	//     indian_red, light_coral, salmon, dark_salmon, light_salmon,
	//     crimson, red, fire_brick, dark_red.
	// Pink colors:
	//     pink, light_pink, hot_pink, deep_pink, medium_violet_red,
	//     pale_violet_red.
	// Orange colors:
	//     coral, tomato, orange_red, dark_orange, orange.
	// Yellow colors:
	//     gold, yellow, light_yellow, lemon, light_goldenrod_yellow,
	//     papaya_whip, moccasin, peach_puff, pale_goldenrod, khaki,
	//     dark_khaki.
	// Purple colors:
	//     lavender, thistle, plum, violet, orchid, fuchsia, magenta,
	//     medium_orchid, medium_purple, rebecca_purple, blue_violet,
	//     dark_violet, dark_orchid, dark_magenta, purple, indigo,
	//     slate_blue, dark_slate_blue, medium_slate_blue.
	// Green colors:
	//     green_yellow, chartreuse, lawn_green, lime, lime_green, pale_green,
	//     light_green, medium_spring_green, spring_green, medium_sea_green,
	//     sea_green, forest_green, green, dark_green, yellow_green,
	//     olive_drab, olive, dark_olive_green, medium_aquamarine,
	//     dark_sea_green, light_sea_green, dark_cyan, teal.
	// Blue colors:
	//     aqua, cyan, light_cyan, pale_turquoise, aquamarine, turquoise,
	//     medium_turquoise, dark_turquoise, cadet_blue, steel_blue,
	//     light_steel_blue, powder_blue, light_blue, sky_blue, light_sky_blue,
	//     deep_sky_blue, dodger_blue, cornflower_blue, royal_blue, blue,
	//     medium_blue, dark_blue, navy, midnight_blue.
	// Brown colors:
	//     cornsilk, blanched_almond, bisque, navajo_white, wheat, burly_wood,
	//     tan, rosy_brown, sandy_brown, goldenrod, dark_goldenrod, peru,
	//     chocolate, saddle_brown, sienna, brown, maroon.
	// White colors:
	//     white, snow, honey_dew, mint_cream, azure, alice_blue, ghost_white,
	//     white_smoke, sea_shell, beige, old_lace, floral_white, ivory,
	//     antique_white, linen, lavender_blush, misty_rose.
	// Grey colors:
	//     gainsboro, light_gray, silver, dark_gray, gray, dim_gray,
	//     light_slate_gray, slate_gray, dark_slate_gray, black.
	static ut::HashMap<ut::String, ut::Color<3, ut::byte> > GenColorNameMap();

	// Below is the list of all possible map types generated with generator prompt.
	static const char* skTypeChecker; // Checkmates texture, not adjustable.
	static const char* skTypeSolid; // Single color for the whole map.

private:
	// Generates a map using provided generator prompt.
	// Acceptable @prompt types are:
	//  1) ve::render::ResourceCreator<Map>::skTypeSolid, see
	//     ve::render::ResourceCreator<Map>::GenerateSolidColorMap() for
	//     acceptable attributes.
	//  2) ve::render::ResourceCreator<Map>::skTypeChecker, without attributes.
	ut::Result<RcRef<Map>, ut::Error> Generate(const ut::String& prompt);

	// Generates a map filled with solid color.
	// Acceptable @attributes are:
	//  'w': width in pixels, must be unsigned integer, default is 0.
	//  'h': height in pixels, must be unsigned integer, default is 0.
	//  'd': depth in pixels, must be unsigned integer, default is 0.
	//  't': map type, valid values are '1d', '2d', '3d' and 'cube'.
	//  'r': red color, must be an integer from 0 to 255, default is 0.
	//  'g': green color, must be an integer from 0 to 255, default is 0.
	//  'b': blue color, must be an integer from 0 to 255, default is 0.
	//  'a': alpha color, must be an integer from 0 to 255, default is 255.
	//  'm': number of mips, must be unsigned integer, 0 means full tail,
	//       default value is 0.
	//  'c': named color, see ResourceCreator<Map>::GenColorNameMap()
	//       for details, this parameter is optional and can override or
	// 	     be used instead of 'r', 'g' and 'b' parameters.
	ut::Result<RcRef<Map>, ut::Error> GenerateSolidColorMap(const Resource::GeneratorPrompt::Attributes& attributes,
	                                                        ut::Optional<ut::String> name);

	// Generates a checker map with ve::render::ResourceCreator<Map>::skTypeChecker
	// resource name.
	ut::Result<RcRef<Map>, ut::Error> GenerateCheckerMap();

    // Adds provided ve::render::Image object to the resource manager as a ve::render::Map resource.
    ut::Result<RcRef<Map>, ut::Error> AddImgToRcMgr(Image image, ut::Optional<ut::String> name);

	// color name map to quickly pick a color by its name
	ut::HashMap<ut::String, ut::Color<3, ut::byte> > color_map;

	// ve::render::Device and ve::renderResourceManager objects are essential
	// for creating a mesh resource.
	Device& device;
	ResourceManager& rc_mgr;

	// ve::render:ImageLoader is needed to load images from file.
	ImageLoader img_loader;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//