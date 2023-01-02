//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::EntitySystem is a system registering all components. Use it only for the
// global things like UI reflection, etc.
class EntitySystem : public System
{
public:
	typedef ut::Array< ut::Ref<Component> > ComponentSet;
	typedef ut::SparseHashMap<Entity::Id, ComponentSet> EntityMap;

	// Constructor.
	//    @param system_name - name of the system.
	EntitySystem(ut::String system_name);

protected:
	// Registers provided entity. And it will be registered only if
	// it has all needed components.
	//    @param id - identifier of the entity.
	//    @param entity - reference to the entity.
	//    @return - 'true' if entity was registered successfully.
	virtual bool RegisterEntity(Entity::Id id, Entity& entity) override;

	// Unregisters the desired entity by its identifier.
	//    @param id - identifier of the entity.
	virtual void UnregisterEntity(Entity::Id id) override;

	// registered entities
	EntityMap entities;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//