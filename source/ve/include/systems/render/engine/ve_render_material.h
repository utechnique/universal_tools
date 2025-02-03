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
		static ut::Result<Material, ut::Error> CreateFromPrompt(const ut::String& prompt,
		                                                        ResourceManager& rc_mgr);

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
	RcRef<Map> diffuse;
	RcRef<Map> normal;
	RcRef<Map> material;

	// Transparency mode.
	Alpha alpha = Alpha::opaque;

	// Indicates if the surface is visible from both sides.
	bool double_sided = false;

	// Indicates if the surface is rendered in unlit (only diffuse color) mode.
	bool unlit = false;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//