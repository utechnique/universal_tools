//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_entity_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
//    @param system_name - name of the system.
EntitySystem::EntitySystem(ut::String system_name) : System(ut::Move(system_name))
{}

// Registers provided entity. And it will be registered only if
// it has all needed components.
//    @param id - identifier of the entity.
//    @param entity - reference to the entity.
//    @return - 'true' if entity was registered successfully.
bool EntitySystem::RegisterEntity(Entity::Id id, Entity& entity)
{
	ComponentSet set;

	const size_t component_count = entity.GetComponentCount();
	for (size_t i = 0; i < component_count; i++)
	{
		if (!set.Add(entity.GetComponentByIndex(i)))
		{
			return false;
		}
	}

	entities.Insert(id, ut::Move(set));

	// success
	return true;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//