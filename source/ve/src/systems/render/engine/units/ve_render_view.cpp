//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_view.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Unit, ve::render::View, "render_view")
UT_REGISTER_TYPE(ve::render::Resource, ve::render::View::GpuData, "view")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& View::Identify() const
{
	return ut::Identify(this);
}

// Registers this view unit into the reflection tree.
//    @param snapshot - reference to the reflection tree.
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