//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_resource.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Forward declarations.
class ResourceManager;
class Map;

//----------------------------------------------------------------------------//
// A Material is a collection of assets and options that can be applied to a
// mesh to control its visual look.
class Material
{
public:
	// Material can be generated from the resource generator prompt.
	struct Generator
	{
		enum Attribute : char
		{
			base_color_map         = 'b',
			normal_map             = 'n',
			metallic_roughness_map = 'm',
			occlusion_map          = 'o',
			emissive_map           = 'e',
			base_color             = 'c',
			alpha                  = 'a',
			transparent            = 't',
			unlit                  = 'u',
		};

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
		static ut::Result<Material, ut::Error> CreateFromPrompt(const ut::String& prompt,
		                                                        ResourceManager& rc_mgr);

		// Creates a checker material.
		//    @param rc_mgr - a reference to the resource manager which is
		//                    needed to correctly acquire resources.
		//    @return - the final material object or ut::Error if failed.
		static ut::Result<Material, ut::Error> CreateChecker(ResourceManager& rc_mgr);

		// Creates a default material.
		//    @param rc_mgr - a reference to the resource manager which is
		//                    needed to correctly acquire resources.
		//    @return - the final material object or ut::Error if failed.
		static ut::Result<Material, ut::Error> CreateDefault(ResourceManager& rc_mgr);

		// The generator prompts for the default material textures.
		static ut::String CreateDefaultBaseColorPrompt();
		static ut::String CreateDefaultNormalPrompt();
		static ut::String CreateDefaultMetallicRoughnessPrompt();
		static ut::String CreateDefaultOcclusionPrompt();
		static ut::String CreateDefaultEmissivePrompt();

		// Below is the list of all possible material types.
		static const char* skTypeSurface; // Solid 2d surface.
	};	

	// Supported transparency modes.
	enum class Alpha
	{
		opaque,
		masked,
		transparent,
	};

	// Map resources.
	RcRef<Map> base_color;
	RcRef<Map> normal;
	RcRef<Map> metallic_roughness;
	RcRef<Map> occlusion;
	RcRef<Map> emissive;

	// Transparency mode.
	Alpha alpha = Alpha::opaque;

	// Indicates if the surface is visible from both sides.
	bool double_sided = false;

	// Indicates if the surface is rendered in unlit (only base color) mode.
	bool unlit = false;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//