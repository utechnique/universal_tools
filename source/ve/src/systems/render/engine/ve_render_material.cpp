//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_material.h"
#include "systems/render/engine/ve_render_rc_mgr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Below is the list of all possible material types.
const char* Material::Generator::skTypeSurface = "surface"; // Solid 2d surface.

// Creates a material from the provided generator prompt.
//    @param prompt - a reference to the string containing a prompt.
//    @param rc_mgr - a reference to the resource manager which is
//                    needed to correctly acquire resources.
//    @return - the final material object or ut::Error if failed.
// Acceptable @prompt attributes are:
// 'b': base color map, must be a string acceptable for ve::render::Map
//      generator prompt. Default is '*solid|r:255|g:255|b:255|a:255'.
// 'n': normal map, must be a string acceptable for ve::render::Map
//      generator prompt. Default is '*solid|r:127|g:127|b:255|a:255'.
// 'm': metallic-roughness map, must be a string acceptable for
//      ve::render::Map generator prompt. Default is '"*solid|g:255|b:255"'.
// 'o': occlusion map, must be a string acceptable for ve::render::Map
//      generator prompt. Default is '*solid|r:255|g:255|b:255|a:255'.
// 'e': emissive map, must be a string acceptable for ve::render::Map
//      generator prompt. Default is '*solid|r:0|g:0|b:0|a:255'.
// 'c': base color that is used if 'b' attribute wasn't provided,
//      must be a name of a color, for example 'cyan' or 'orange', see
//      ResourceCreator<Map>::GenColorNameMap() for details.
// 'a': alpha value that is applied only if 'c' parameter was also
//      provided, must be an integer from 0 to 255. Default is 255.
// 't': enables transparency, must be 'yes' or 'no'. Default is 'no'.
// 'u': enables unlit mode, must be 'yes' or 'no'. Default is 'no'.
ut::Result<Material, ut::Error> Material::Generator::CreateFromPrompt(const ut::String& prompt,
                                                                      ResourceManager& rc_mgr)
{
	auto gen_from_prompt = [&rc_mgr](const ut::String& prompt) -> ut::Result<Material, ut::Error>
	{
		// check if the prompt start with a special symbol
		if (!Resource::GeneratorPrompt::Check(prompt))
		{
			return ut::MakeError(ut::error::invalid_arg);
		}

		// parse the prompt into the material type and separate attributes
		Resource::GeneratorPrompt::Attributes generator_attributes;
		ut::Result<ut::String, ut::Error> prompt_parse_result = Resource::GeneratorPrompt::Parse(prompt,
			generator_attributes);
		if (!prompt_parse_result)
		{
			return ut::MakeError(prompt_parse_result.MoveAlt());
		}

		// extract material type
		const ut::String material_type(prompt_parse_result.Move());
		if (material_type != skTypeSurface)
		{
			// only the surface type is supported at this moment
			return ut::MakeError(ut::error::not_supported);
		}

		// material parameters
		ut::Optional<ut::String> base_color_prompt;
		ut::String normal_prompt = CreateDefaultNormalPrompt();
		ut::String metallic_roughness_prompt = CreateDefaultMetallicRoughnessPrompt();
		ut::String occlusion_prompt = CreateDefaultOcclusionPrompt();
		ut::String emissive_prompt = CreateDefaultEmissivePrompt();
		ut::String color;
		ut::byte alpha = 255;
		bool enable_transparency = false;
		bool unlit = false;

		// update parameters using generator prompt attributes
		for (const Resource::GeneratorPrompt::Attribute& attribute : generator_attributes)
		{
			switch (attribute.type)
			{
			case Attribute::base_color_map: base_color_prompt = attribute.value; break;
			case Attribute::normal_map: normal_prompt = attribute.value; break;
			case Attribute::metallic_roughness_map: metallic_roughness_prompt = attribute.value; break;
			case Attribute::occlusion_map: occlusion_prompt = attribute.value; break;
			case Attribute::emissive_map: emissive_prompt = attribute.value; break;
			case Attribute::base_color: color = attribute.value; break;
			case Attribute::alpha: alpha = ut::Scan<ut::byte>(attribute.value); break;
			case Attribute::transparent: enable_transparency = attribute.value == "yes"; break;
			case Attribute::unlit: unlit = attribute.value == "yes"; break;
			}
		}

		// process base color map parameter
		if (!base_color_prompt && color.Length() > 0)
		{
			constexpr bool srgb_enabled = true;
			base_color_prompt = ResourceCreator<Map>::Generator::Create1x1Prompt(color,
			                                                                     alpha,
			                                                                     srgb_enabled);
		}
		else if (!base_color_prompt)
		{
			base_color_prompt = CreateDefaultBaseColorPrompt();
		}

		// acquire base color map
		ut::Result<RcRef<Map>, ut::Error> base_color_map = rc_mgr.Acquire<Map>(base_color_prompt.Get());
		if (!base_color_map)
		{
			return ut::MakeError(base_color_map.MoveAlt());
		}

		// acquire normal map
		ut::Result<RcRef<Map>, ut::Error> normal_map = rc_mgr.Acquire<Map>(normal_prompt);
		if (!normal_map)
		{
			return ut::MakeError(normal_map.MoveAlt());
		}

		// acquire metallic-roughness map
		ut::Result<RcRef<Map>, ut::Error> metallic_roughness_map = rc_mgr.Acquire<Map>(metallic_roughness_prompt);
		if (!metallic_roughness_map)
		{
			return ut::MakeError(metallic_roughness_map.MoveAlt());
		}

		// acquire occlusion map
		ut::Result<RcRef<Map>, ut::Error> occlusion_map = rc_mgr.Acquire<Map>(occlusion_prompt);
		if (!occlusion_map)
		{
			return ut::MakeError(occlusion_map.MoveAlt());
		}

		// acquire emissive map
		ut::Result<RcRef<Map>, ut::Error> emissive_map = rc_mgr.Acquire<Map>(emissive_prompt);
		if (!emissive_map)
		{
			return ut::MakeError(emissive_map.MoveAlt());
		}

		// initialize material
		Material material;
		material.base_color = base_color_map.Move();
		material.normal = normal_map.Move();
		material.metallic_roughness = metallic_roughness_map.Move();
		material.occlusion = occlusion_map.Move();
		material.emissive = emissive_map.Move();
		material.alpha = enable_transparency ? Material::Alpha::transparent :
		                                       Material::Alpha::opaque;
		material.double_sided = false;
		material.unlit = unlit;

		return material;
	};

	ut::Result<Material, ut::Error> result = gen_from_prompt(prompt);
	if (!result)
	{
		ut::log.Lock() << "Failed to load a render resource (material): " << prompt << ut::cret;
		return CreateChecker(rc_mgr);
	}

	return result;
}

// Creates a checker material.
//    @param rc_mgr - a reference to the resource manager which is
//                    needed to correctly acquire resources.
//    @return - the final material object or ut::Error if failed.
ut::Result<Material, ut::Error> Material::Generator::CreateChecker(ResourceManager& rc_mgr)
{
	Material material;

	material.base_color = rc_mgr.img_checker;
	material.normal = rc_mgr.img_normal;
	material.metallic_roughness = rc_mgr.img_green;
	material.occlusion = rc_mgr.img_white;
	material.emissive = rc_mgr.img_black;

	return material;
}

// Creates a default material.
//    @param rc_mgr - a reference to the resource manager which is
//                    needed to correctly acquire resources.
//    @return - the final material object or ut::Error if failed.
ut::Result<Material, ut::Error> Material::Generator::CreateDefault(ResourceManager& rc_mgr)
{
	Material material;

	material.base_color = rc_mgr.img_white;
	material.normal = rc_mgr.img_normal;
	material.metallic_roughness = rc_mgr.img_green;
	material.occlusion = rc_mgr.img_white;
	material.emissive = rc_mgr.img_black;

	return material;
}

// The generator prompt for a base color map.
ut::String Material::Generator::CreateDefaultBaseColorPrompt()
{
	const ut::Color<4, ut::byte> white(255, 255, 255, 255);
	return ResourceCreator<Map>::Generator::Create1x1Prompt(white);
}

// The generator prompt for a normal map.
ut::String Material::Generator::CreateDefaultNormalPrompt()
{
	const ut::Color<4, ut::byte> normal(127, 127, 255, 255);
	return ResourceCreator<Map>::Generator::Create1x1Prompt(normal);
}

// The generator prompt for a metallic-roughness map.
ut::String Material::Generator::CreateDefaultMetallicRoughnessPrompt()
{
	const ut::byte roughness_default = 255;
	const ut::byte metal_default = 0;
	const ut::Color<4, ut::byte> metallic_roughness(0,
	                                                roughness_default,
	                                                metal_default,
	                                                255);
	return ResourceCreator<Map>::Generator::Create1x1Prompt(metallic_roughness);
}

// The generator prompt for an occlusion map.
ut::String Material::Generator::CreateDefaultOcclusionPrompt()
{
	const ut::Color<4, ut::byte> white(255, 255, 255, 255);
	return ResourceCreator<Map>::Generator::Create1x1Prompt(white);
}

// The generator prompt for a emissive map.
ut::String Material::Generator::CreateDefaultEmissivePrompt()
{
	const ut::Color<4, ut::byte> black(0, 0, 0, 255);
	return ResourceCreator<Map>::Generator::Create1x1Prompt(black);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
