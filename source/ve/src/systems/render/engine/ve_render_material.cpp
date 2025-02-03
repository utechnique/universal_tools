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
// 'd': diffuse map, must be a string acceptable for ve::render::Map
//      generator prompt. Default is '*solid|r:255|g:255|b:255|a:255'.
// 'n': normal map, must be a string acceptable for ve::render::Map
//      generator prompt. Default is '*solid|r:127|g:127|b:255|a:255'.
// 'm': material map, must be a string acceptable for ve::render::Map
//      generator prompt. Default is '"*solid|r:255|a:255"'.
// 'c': diffuse color that is used if 'd' attribute wasn't provided,
//      must be a name of a color, for example 'cyan' or 'orange', see
//      ResourceCreator<Map>::GenColorNameMap() for details.
// 'a': alpha value that is applied only if 'c' parameter was also
//      provided, must be an integer from 0 to 255. Default is 255.
// 't': enables transparency, must be 'yes' or 'no'. Default is 'no'.
// 'u': enables unlit mode, must be 'yes' or 'no'. Default is 'no'.
ut::Result<Material, ut::Error> Material::Generator::CreateFromPrompt(const ut::String& prompt,
                                                                      ResourceManager& rc_mgr)
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
		return ut::MakeError(ut::error::not_supported);
	}

	// material parameters
	ut::Optional<ut::String> diffuse_prompt;
	ut::String normal_prompt("*solid|r:127|g:127|b:255|a:255");
	ut::String material_prompt("*solid|r:255|a:255");
	ut::String color;
	ut::byte alpha = 255;
	bool enable_transparency = false;
	bool unlit = false;

	// update parameters using generator prompt attributes
	for (const Resource::GeneratorPrompt::Attribute& attribute : generator_attributes)
	{
		switch (attribute.type)
		{
			case 'd': diffuse_prompt = attribute.value; break;
			case 'n': normal_prompt = attribute.value; break;
			case 'm': material_prompt = attribute.value; break;
			case 'c': color = attribute.value; break;
			case 'a': alpha = ut::Scan<ut::byte>(attribute.value); break;
			case 't': enable_transparency = attribute.value == "yes"; break;
			case 'u': unlit = attribute.value == "yes"; break;
		}
	}

	// process diffuse map parameter
	if (!diffuse_prompt && color.Length() > 0)
	{
		ut::String solid_color_diffuse_prompt = Resource::GeneratorPrompt::skStarter;
		solid_color_diffuse_prompt += ResourceCreator<Map>::skTypeSolid;
		solid_color_diffuse_prompt += Resource::GeneratorPrompt::skSeparatorChr0;
		solid_color_diffuse_prompt += ut::String("c") + Resource::GeneratorPrompt::skValueSeparatorChr + color;

		if (alpha != 255)
		{
			solid_color_diffuse_prompt += Resource::GeneratorPrompt::skSeparatorChr0;
			solid_color_diffuse_prompt += ut::String("a") + Resource::GeneratorPrompt::skValueSeparatorChr + ut::Print(alpha);
		}

		diffuse_prompt = solid_color_diffuse_prompt;
	}
	else if (!diffuse_prompt)
	{
		diffuse_prompt = ut::String("*solid|r:255|g:255|b:255|a:255");
	}

	// acquire diffuse map
	ut::Result<RcRef<Map>, ut::Error> diffuse_map = rc_mgr.Acquire<Map>(diffuse_prompt.Get());
	if (!diffuse_map)
	{
		return ut::MakeError(diffuse_map.MoveAlt());
	}

	// acquire normal map
	ut::Result<RcRef<Map>, ut::Error> normal_map = rc_mgr.Acquire<Map>(normal_prompt);
	if (!normal_map)
	{
		return ut::MakeError(normal_map.MoveAlt());
	}

	// acquire material map
	ut::Result<RcRef<Map>, ut::Error> material_map = rc_mgr.Acquire<Map>(material_prompt);
	if (!material_map)
	{
		return ut::MakeError(material_map.MoveAlt());
	}

	// initialize material
	Material material;
	material.diffuse = diffuse_map.Move();
	material.normal = normal_map.Move();
	material.material = material_map.Move();
	material.alpha = enable_transparency ? Material::Alpha::transparent :
	                                       Material::Alpha::opaque;
	material.double_sided = false;
	material.unlit = unlit;

	return material;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
