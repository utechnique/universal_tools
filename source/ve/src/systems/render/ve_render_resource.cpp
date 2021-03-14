//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_resource.h"
#include "systems/render/engine/ve_render_rc_mgr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
ReferencedResource::ReferencedResource(ResourceManager& rc_mgr,
                                       ut::UniquePtr<Resource> unique_rc,
                                       Resource::Id rc_id,
                                       ut::Optional<ut::String> rc_name) : ptr(ut::Move(unique_rc))
                                                                         , ref_counter(ut::MakeShared<Counter>(rc_mgr, rc_id))
                                                                         , name(ut::Move(rc_name))
{}

// Counter constructor.
ReferencedResource::Counter::Counter(ResourceManager& rc_mgr,
                                     Resource::Id rc_id) : manager(rc_mgr)
                                                         , id(rc_id)
{}

// Decrements reference count and enqueues a deletion if it becomes zero.
void ReferencedResource::Counter::Decrement()
{
	if (count.Decrement() == 0)
	{
		manager.DeleteResource(id);
	}
}

// Increments reference count.
void ReferencedResource::Counter::Increment()
{
	count.Increment();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//