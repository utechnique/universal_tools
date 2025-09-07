//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/resources/ve_render_map.h"
#include "systems/render/engine/ve_render_rc_mgr.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Resource, ve::render::Map, "map")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
const char* ResourceCreator<Map>::skTypeChecker = "checker";
const char* ResourceCreator<Map>::skTypeSolid = "solid";

//----------------------------------------------------------------------------//
// Constructor.
Map::Map(Image in_img) : Image(ut::Move(in_img))
{}

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& Map::Identify() const
{
	return ut::Identify(this);
}

//----------------------------------------------------------------------------//
// Constructor initializes an image loader and a hashmap of color names.
ResourceCreator<Map>::ResourceCreator(Device& in_device,
                                      ResourceManager& in_rc_mgr) : color_map(GenColorNameMap())
                                                                  , device(in_device)
                                                                  , rc_mgr(in_rc_mgr)
                                                                  , img_loader(device)
{}

// Creates a map according to the provided name, path or generator prompt.
ut::Result<RcRef<Map>, ut::Error> ResourceCreator<Map>::Create(const ut::String& name)
{
	auto create = [&](const ut::String& map_name) -> ut::Result<RcRef<Map>, ut::Error>
	{
		const bool is_generator_prompt = Resource::GeneratorPrompt::Check(map_name);
		if (is_generator_prompt)
		{
			return Generate(map_name);
		}

		ut::Result<Image, ut::Error> load_img_result = img_loader.Load(map_name);
		if (!load_img_result)
		{
			return ut::MakeError(load_img_result.MoveAlt());
		}


		return AddImgToRcMgr(load_img_result.Move(), map_name);
	};

	ut::Result<RcRef<Map>, ut::Error> result = create(name);
	if (!result)
	{
		ut::log.Lock() << "Failed to load a render resource (map): " << name << ut::cret;
		return create(ut::String(Resource::GeneratorPrompt::skStarter) + skTypeChecker);
	}

	return result;
}

// Generates a map using provided generator prompt.
// Acceptable @prompt types are:
//  1) ve::render::ResourceCreator<Map>::skTypeSolid, see
//     ve::render::ResourceCreator<Map>::GenerateSolidColorMap() for
//     acceptable attributes.
//  2) ve::render::ResourceCreator<Map>::skTypeChecker, without attributes.
ut::Result<RcRef<Map>, ut::Error> ResourceCreator<Map>::Generate(const ut::String& prompt)
{
	// parse the prompt into the map type and separate attributes
	Resource::GeneratorPrompt::Attributes generator_attributes;
	ut::Result<ut::String, ut::Error> prompt_parse_result = Resource::GeneratorPrompt::Parse(prompt,
	                                                                                         generator_attributes);
	if (!prompt_parse_result)
	{
		return ut::MakeError(prompt_parse_result.MoveAlt());
	}

	// extract and process map type
	const ut::String map_type(prompt_parse_result.Move());
	if (map_type == skTypeSolid)
	{
		return GenerateSolidColorMap(generator_attributes, prompt);
	}
	else if (map_type == skTypeChecker)
	{
		return GenerateCheckerMap();
	}

	// fail if type is not recognized
	return ut::MakeError(ut::error::not_supported);
}

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
ut::Result<RcRef<Map>, ut::Error> ResourceCreator<Map>::GenerateSolidColorMap(const Resource::GeneratorPrompt::Attributes& attributes,
                                                                              ut::Optional<ut::String> name)
{
	// map parameters
	ut::String type = "2d";
	ut::uint32 width = 1;
	ut::uint32 height = 1;
	ut::uint32 depth = 1;
	ut::uint32 mip_count = 0;
	ut::Color<4, ut::byte> color(0, 0, 0, 255);

	// update parameters using generator prompt attributes
	for (const Resource::GeneratorPrompt::Attribute& attribute : attributes)
	{
		switch (attribute.type)
		{
		case 'w': width = ut::Scan<ut::uint32>(attribute.value); break;
		case 'h': height = ut::Scan<ut::uint32>(attribute.value); break;
		case 'd': depth = ut::Scan<ut::uint32>(attribute.value); break;
		case 't': type = attribute.value; break;
		case 'r': color.R() = ut::Scan<ut::byte>(attribute.value); break;
		case 'g': color.G() = ut::Scan<ut::byte>(attribute.value); break;
		case 'b': color.B() = ut::Scan<ut::byte>(attribute.value); break;
		case 'a': color.A() = ut::Scan<ut::byte>(attribute.value); break;
		case 'm': mip_count = ut::Scan<ut::uint32>(attribute.value); break;
		case 'c':
		{
			ut::Optional<ut::Color<3, ut::byte>&> find_color_result = color_map.Find(attribute.value);
			if (find_color_result)
			{
				color.R() = find_color_result->R();
				color.G() = find_color_result->G();
				color.B() = find_color_result->B();
			}
		} break;
		}
	}

	// get texture type
	const bool is_cubemap = type == "cube";
	Image::Type img_type = is_cubemap ? Image::Type::cubic :
		type == "1d" ? Image::Type::linear :
		type == "2d" ? Image::Type::planar :
		type == "3d" ? Image::Type::volumetric :
		Image::Type::planar;

	// generate final resource
	ResourceCreator<Map>::InitInfo map_info;
	map_info.img_type = Image::Type::planar;
	map_info.format = pixel::Format::r8g8b8a8_unorm;
	map_info.width = width;
	map_info.height = height;
	map_info.depth = depth;
	map_info.generate_mips = false;
	map_info.name = ut::Move(name);
	ut::Array< ut::Color<4, ut::byte> > pixels(width * height * depth);
	for (ut::Color<4, ut::byte>& pixel : pixels)
	{
		pixel = color;
	}

	return CreateFromData<4, ut::byte>(map_info, pixels);
}

// Generates a checker map with ve::render::ResourceCreator<Map>::skTypeChecker
// resource name.
ut::Result<RcRef<Map>, ut::Error> ResourceCreator<Map>::GenerateCheckerMap()
{
	constexpr ut::uint32 checker_size = 256;
	constexpr ut::uint32 checker_cell_size = 32;

	InitInfo init_info;
	init_info.img_type = Image::Type::planar;
	init_info.format = pixel::Format::r8g8b8a8_unorm;
	init_info.width = checker_size;
	init_info.height = checker_size;
	init_info.depth = 1;
	init_info.mip_count = 0;
	init_info.generate_mips = true;
	init_info.name = skTypeChecker;

	ut::Array< ut::Color<4, ut::byte> > pixels(init_info.width * init_info.height);
	for (ut::uint32 y = 0; y < init_info.height; y++)
	{
		for (ut::uint32 x = 0; x < init_info.width; x++)
		{
			const bool y_is_even = (y / checker_cell_size) % 2 == 0;
			const bool cell_is_even = (x / checker_cell_size) % 2 == y_is_even ? 0 : 1;
			pixels[y * init_info.width + x] = cell_is_even ?
			                                  ut::Color<4, ut::byte>(255, 0, 255, 255) :
			                                  ut::Color<4, ut::byte>(0, 0, 0, 255);
		}
	}

	return CreateFromData<4, ut::byte>(init_info, pixels);
}

// Adds provided ve::render::Image object to the resource manager as a ve::render::Map resource.
ut::Result<RcRef<Map>, ut::Error> ResourceCreator<Map>::AddImgToRcMgr(Image image,
                                                                      ut::Optional<ut::String> name)
{
    return rc_mgr.AddResource<Map>(Map(ut::Move(image)), name);
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
ut::HashMap<ut::String, ut::Color<3, ut::byte> > ResourceCreator<Map>::GenColorNameMap()
{
	ut::HashMap<ut::String, ut::Color<3, ut::byte> > map;

	// red colors
	map.Insert("indian_red", ut::Color<3, ut::byte>(205, 92, 92));
	map.Insert("light_coral", ut::Color<3, ut::byte>(240, 128, 128));
	map.Insert("salmon", ut::Color<3, ut::byte>(250, 128, 114));
	map.Insert("dark_salmon", ut::Color<3, ut::byte>(233, 150, 122));
	map.Insert("light_salmon", ut::Color<3, ut::byte>(255, 160, 122));
	map.Insert("crimson", ut::Color<3, ut::byte>(255, 160, 122));
	map.Insert("red", ut::Color<3, ut::byte>(255, 0, 0));
	map.Insert("fire_brick", ut::Color<3, ut::byte>(178, 34, 34));
	map.Insert("dark_red", ut::Color<3, ut::byte>(139, 0, 0));

	// pink colors
	map.Insert("pink", ut::Color<3, ut::byte>(255, 192, 203));
	map.Insert("light_pink", ut::Color<3, ut::byte>(255, 192, 203));
	map.Insert("hot_pink", ut::Color<3, ut::byte>(255, 192, 203));
	map.Insert("deep_pink", ut::Color<3, ut::byte>(255, 20, 147));
	map.Insert("medium_violet_red", ut::Color<3, ut::byte>(199, 21, 133));
	map.Insert("pale_violet_red", ut::Color<3, ut::byte>(199, 21, 133));

	// orange colors
	map.Insert("coral", ut::Color<3, ut::byte>(255, 127, 80));
	map.Insert("tomato", ut::Color<3, ut::byte>(255, 99, 71));
	map.Insert("orange_red", ut::Color<3, ut::byte>(255, 69, 0));
	map.Insert("dark_orange", ut::Color<3, ut::byte>(255, 140, 0));
	map.Insert("orange", ut::Color<3, ut::byte>(255, 165, 0));

	// yellow colors
	map.Insert("gold", ut::Color<3, ut::byte>(255, 215, 0));
	map.Insert("yellow", ut::Color<3, ut::byte>(255, 255, 0));
	map.Insert("light_yellow", ut::Color<3, ut::byte>(255, 255, 224));
	map.Insert("lemon", ut::Color<3, ut::byte>(255, 250, 205));
	map.Insert("light_goldenrod_yellow", ut::Color<3, ut::byte>(250, 250, 210));
	map.Insert("papaya_whip", ut::Color<3, ut::byte>(255, 239, 213));
	map.Insert("moccasin", ut::Color<3, ut::byte>(255, 228, 181));
	map.Insert("peach_puff", ut::Color<3, ut::byte>(255, 218, 185));
	map.Insert("pale_goldenrod", ut::Color<3, ut::byte>(238, 232, 170));
	map.Insert("khaki", ut::Color<3, ut::byte>(240, 230, 140));
	map.Insert("dark_khaki", ut::Color<3, ut::byte>(189, 183, 107));

	// purple colors
	map.Insert("lavender", ut::Color<3, ut::byte>(230, 230, 250));
	map.Insert("thistle", ut::Color<3, ut::byte>(216, 191, 216));
	map.Insert("plum", ut::Color<3, ut::byte>(221, 160, 221));
	map.Insert("violet", ut::Color<3, ut::byte>(238, 130, 238));
	map.Insert("orchid", ut::Color<3, ut::byte>(218, 112, 214));
	map.Insert("fuchsia", ut::Color<3, ut::byte>(255, 0, 255));
	map.Insert("magenta", ut::Color<3, ut::byte>(255, 0, 255));
	map.Insert("medium_orchid", ut::Color<3, ut::byte>(186, 85, 211));
	map.Insert("medium_purple", ut::Color<3, ut::byte>(147, 112, 219));
	map.Insert("rebecca_purple", ut::Color<3, ut::byte>(102, 51, 153));
	map.Insert("blue_violet", ut::Color<3, ut::byte>(138, 43, 226));
	map.Insert("dark_violet", ut::Color<3, ut::byte>(148, 0, 211));
	map.Insert("dark_orchid", ut::Color<3, ut::byte>(153, 50, 204));
	map.Insert("dark_magenta", ut::Color<3, ut::byte>(139, 0, 139));
	map.Insert("purple", ut::Color<3, ut::byte>(128, 0, 128));
	map.Insert("indigo", ut::Color<3, ut::byte>(75, 0, 130));
	map.Insert("slate_blue", ut::Color<3, ut::byte>(106, 90, 205));
	map.Insert("dark_slate_blue", ut::Color<3, ut::byte>(72, 61, 139));
	map.Insert("medium_slate_blue", ut::Color<3, ut::byte>(123, 104, 238));

	// green colors
	map.Insert("green_yellow", ut::Color<3, ut::byte>(173, 255, 47));
	map.Insert("chartreuse", ut::Color<3, ut::byte>(127, 255, 0));
	map.Insert("lawn_green", ut::Color<3, ut::byte>(124, 252, 0));
	map.Insert("lime", ut::Color<3, ut::byte>(0, 255, 0));
	map.Insert("lime_green", ut::Color<3, ut::byte>(50, 205, 50));
	map.Insert("pale_green", ut::Color<3, ut::byte>(152, 251, 152));
	map.Insert("light_green", ut::Color<3, ut::byte>(144, 238, 144));
	map.Insert("medium_spring_green", ut::Color<3, ut::byte>(0, 250, 154));
	map.Insert("spring_green", ut::Color<3, ut::byte>(0, 255, 127));
	map.Insert("medium_sea_green", ut::Color<3, ut::byte>(60, 179, 113));
	map.Insert("sea_green", ut::Color<3, ut::byte>(46, 139, 87));
	map.Insert("forest_green", ut::Color<3, ut::byte>(34, 139, 34));
	map.Insert("green", ut::Color<3, ut::byte>(0, 128, 0));
	map.Insert("dark_green", ut::Color<3, ut::byte>(0, 100, 0));
	map.Insert("yellow_green", ut::Color<3, ut::byte>(154, 205, 50));
	map.Insert("olive_drab", ut::Color<3, ut::byte>(107, 142, 35));
	map.Insert("olive", ut::Color<3, ut::byte>(128, 128, 0));
	map.Insert("dark_olive_green", ut::Color<3, ut::byte>(85, 107, 47));
	map.Insert("medium_aquamarine", ut::Color<3, ut::byte>(102, 205, 170));
	map.Insert("dark_sea_green", ut::Color<3, ut::byte>(143, 188, 139));
	map.Insert("light_sea_green", ut::Color<3, ut::byte>(32, 178, 170));
	map.Insert("dark_cyan", ut::Color<3, ut::byte>(0, 139, 139));
	map.Insert("teal", ut::Color<3, ut::byte>(0, 128, 128));

	// blue colors
	map.Insert("aqua", ut::Color<3, ut::byte>(0, 255, 255));
	map.Insert("cyan", ut::Color<3, ut::byte>(0, 255, 255));
	map.Insert("light_cyan", ut::Color<3, ut::byte>(224, 255, 255));
	map.Insert("pale_turquoise", ut::Color<3, ut::byte>(175, 238, 238));
	map.Insert("aquamarine", ut::Color<3, ut::byte>(127, 255, 212));
	map.Insert("turquoise", ut::Color<3, ut::byte>(64, 224, 208));
	map.Insert("medium_turquoise", ut::Color<3, ut::byte>(72, 209, 204));
	map.Insert("dark_turquoise", ut::Color<3, ut::byte>(0, 206, 209));
	map.Insert("cadet_blue", ut::Color<3, ut::byte>(95, 158, 160));
	map.Insert("steel_blue", ut::Color<3, ut::byte>(70, 130, 180));
	map.Insert("light_steel_blue", ut::Color<3, ut::byte>(176, 196, 222));
	map.Insert("powder_blue", ut::Color<3, ut::byte>(176, 224, 230));
	map.Insert("light_blue", ut::Color<3, ut::byte>(173, 216, 230));
	map.Insert("sky_blue", ut::Color<3, ut::byte>(135, 206, 235));
	map.Insert("light_sky_blue", ut::Color<3, ut::byte>(135, 206, 250));
	map.Insert("deep_sky_blue", ut::Color<3, ut::byte>(0, 191, 255));
	map.Insert("dodger_blue", ut::Color<3, ut::byte>(30, 144, 255));
	map.Insert("cornflower_blue", ut::Color<3, ut::byte>(100, 149, 237));
	map.Insert("royal_blue", ut::Color<3, ut::byte>(65, 105, 225));
	map.Insert("blue", ut::Color<3, ut::byte>(0, 0, 255));
	map.Insert("medium_blue", ut::Color<3, ut::byte>(0, 0, 205));
	map.Insert("dark_blue", ut::Color<3, ut::byte>(0, 0, 139));
	map.Insert("navy", ut::Color<3, ut::byte>(0, 0, 128));
	map.Insert("midnight_blue", ut::Color<3, ut::byte>(25, 25, 112));
	
	// brown colors
	map.Insert("cornsilk", ut::Color<3, ut::byte>(255, 248, 220));
	map.Insert("blanched_almond", ut::Color<3, ut::byte>(255, 235, 205));
	map.Insert("bisque", ut::Color<3, ut::byte>(255, 228, 196));
	map.Insert("navajo_white", ut::Color<3, ut::byte>(255, 222, 173));
	map.Insert("wheat", ut::Color<3, ut::byte>(245, 222, 179));
	map.Insert("burly_wood", ut::Color<3, ut::byte>(222, 184, 135));
	map.Insert("tan", ut::Color<3, ut::byte>(210, 180, 140));
	map.Insert("rosy_brown", ut::Color<3, ut::byte>(188, 143, 143));
	map.Insert("sandy_brown", ut::Color<3, ut::byte>(244, 164, 96));
	map.Insert("goldenrod", ut::Color<3, ut::byte>(218, 165, 32));
	map.Insert("dark_goldenrod", ut::Color<3, ut::byte>(184, 134, 11));
	map.Insert("peru", ut::Color<3, ut::byte>(205, 133, 63));
	map.Insert("chocolate", ut::Color<3, ut::byte>(210, 105, 30));
	map.Insert("saddle_brown", ut::Color<3, ut::byte>(139, 69, 19));
	map.Insert("sienna", ut::Color<3, ut::byte>(160, 82, 45));
	map.Insert("brown", ut::Color<3, ut::byte>(165, 42, 42));
	map.Insert("maroon", ut::Color<3, ut::byte>(128, 0, 0));

	// white colors
	map.Insert("white", ut::Color<3, ut::byte>(255, 255, 255));
	map.Insert("snow", ut::Color<3, ut::byte>(255, 250, 250));
	map.Insert("honey_dew", ut::Color<3, ut::byte>(240, 255, 240));
	map.Insert("mint_cream", ut::Color<3, ut::byte>(245, 255, 250));
	map.Insert("azure", ut::Color<3, ut::byte>(240, 255, 255));
	map.Insert("alice_blue", ut::Color<3, ut::byte>(240, 248, 255));
	map.Insert("ghost_white", ut::Color<3, ut::byte>(248, 248, 255));
	map.Insert("white_smoke", ut::Color<3, ut::byte>(245, 245, 245));
	map.Insert("sea_shell", ut::Color<3, ut::byte>(255, 245, 238));
	map.Insert("beige", ut::Color<3, ut::byte>(245, 245, 220));
	map.Insert("old_lace", ut::Color<3, ut::byte>(253, 245, 230));
	map.Insert("floral_white", ut::Color<3, ut::byte>(255, 250, 240));
	map.Insert("ivory", ut::Color<3, ut::byte>(255, 255, 240));
	map.Insert("antique_white", ut::Color<3, ut::byte>(250, 235, 215));
	map.Insert("linen", ut::Color<3, ut::byte>(250, 240, 230));
	map.Insert("lavender_blush", ut::Color<3, ut::byte>(255, 240, 245));
	map.Insert("misty_rose", ut::Color<3, ut::byte>(255, 228, 225));

	// grey colors
	map.Insert("gainsboro", ut::Color<3, ut::byte>(220, 220, 220));
	map.Insert("light_gray", ut::Color<3, ut::byte>(211, 211, 211));
	map.Insert("silver", ut::Color<3, ut::byte>(192, 192, 192));
	map.Insert("dark_gray", ut::Color<3, ut::byte>(169, 169, 169));
	map.Insert("gray", ut::Color<3, ut::byte>(128, 128, 128));
	map.Insert("dim_gray", ut::Color<3, ut::byte>(105, 105, 105));
	map.Insert("light_slate_gray", ut::Color<3, ut::byte>(119, 136, 153));
	map.Insert("slate_gray", ut::Color<3, ut::byte>(112, 128, 144));
	map.Insert("dark_slate_gray", ut::Color<3, ut::byte>(47, 79, 79));
	map.Insert("black", ut::Color<3, ut::byte>(0, 0, 0));

	return map;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//