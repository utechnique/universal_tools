//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/units/ve_render_view.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Unit, ve::render::View, "render_view")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
const ut::DynamicType& View::Identify() const
{
	return ut::Identify(this);
}

void View::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(view_matrix, "view");
	snapshot.Add(proj_matrix, "projection");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//