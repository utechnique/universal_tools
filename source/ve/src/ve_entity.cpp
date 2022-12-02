//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_entity.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Adds provided component to the entity.
//    @param component - unique pointer to the new component.
//    @param overwrite - flag that controls whether component must be
//                       overwritten in case if a component of such
//                       type already exists.
//    @return - optional ut::Error if failed to add @component.
ut::Optional<ut::Error> Entity::AddComponent(ut::UniquePtr<Component> component,
                                             bool overwrite)
{
	// check if component already exists
	const ut::DynamicType::Handle type_handle = component->Identify().GetHandle();
	ut::Optional<ut::Ref<Component>&> find_result = cache.Find(type_handle);
	if (find_result)
	{
		// exit if component already exists and it can't be overwritten
		if (!overwrite)
		{
			return ut::Error(ut::error::already_exists);
		}
		else // otherwise delete current component
		{
			RemoveComponent(type_handle);
		}
	}

	// add new component
	if (!components.Add(ut::Move(component)))
	{
		return ut::Error(ut::error::out_of_memory);
	}

	// add to the cache
	if (cache.Insert(type_handle, components.GetLast().GetRef()))
	{
		return ut::Error(ut::error::already_exists);
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Removes a component of the specified type from the entity.
//    @param component_type - handle to the type of the component
//                            to be deleted.
//    @return - optional ut::Error if failed to remove a component.
ut::Optional<ut::Error> Entity::RemoveComponent(const ut::DynamicType::Handle component_type)
{
	// remove from cache
	cache.Remove(component_type);

	// find and delete component itself
	const size_t component_count = components.Count();
	for (size_t i = 0; i < component_count; i++)
	{
		const ut::DynamicType& dyn_type = components[i]->Identify();
		if (dyn_type.GetHandle() == component_type)
		{
			components.Remove(i);
			return ut::Optional<ut::Error>();
		}
	}

	// nothing was found
	return ut::Error(ut::error::not_found);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//