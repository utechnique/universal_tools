//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_resource.h"
#include "systems/render/engine/resources/ve_render_map.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// A Material is a collection of assets and options that can be applied to a
// mesh to control its visual look.
class Material
{
public:
	enum Alpha
	{
		alpha_opaque,
		alpha_test,
		alpha_transparent,
	};

	RcRef<Map> diffuse;
	RcRef<Map> normal;
	RcRef<Map> material;

	Alpha alpha = alpha_opaque;
	bool double_sided = false;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//