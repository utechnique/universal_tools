//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commands/ve_cmd_delete_component.h"
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
CmdDeleteComponent::CmdDeleteComponent(Entity::Id id,
                                       ut::DynamicType::Handle type_handle) noexcept : entity_id(id)
                                                                                     , component_type(type_handle)
{}

// Calls ve::Environment::DeleteEntity().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdDeleteComponent::Execute(Environment& environment)
{
	signal(environment.DeleteComponent(entity_id, component_type));
	return ut::Optional<ut::Error>();
}

// Connects provided function with signal that is triggered after a call
// to ve::Environment::AddEntity().
void CmdDeleteComponent::Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot)
{
	signal.Connect(ut::Move(slot));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//