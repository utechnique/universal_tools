//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commands/ve_cmd_add_component.h"
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
//    @param entity_id - identifier of the entity to add the component.
//    @param component - the unique pointer to the component to be added.
CmdAddComponent::CmdAddComponent(Entity::Id in_entity_id,
                                 ut::UniquePtr<Component> in_component) noexcept : entity_id(in_entity_id)
                                                                                 , component(ut::Move(in_component))
{}

// Calls ve::Environment::AddComponent().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdAddComponent::Execute(Environment& environment)
{
	ut::Optional<ut::Error> error = environment.AddComponent(entity_id, ut::Move(component));
	signal(error);
	return ut::Optional<ut::Error>();
}

// Connects provided function with signal that is triggered after a call
// to ve::Environment::AddComponent().
void CmdAddComponent::Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot)
{
	signal.Connect(ut::Move(slot));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//