//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_resource.h"
#include "systems/render/ve_render_api.h"
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
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//