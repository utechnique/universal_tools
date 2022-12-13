//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commands/ve_cmd_update_component.h"
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
//    @param entity_id - id of the entity owning the desired component.
//    @param component_type - type of the component (must be derived
//                            from the ve::Component class).
//    @param callback - the callback that accepts a reference
//                      to the meta-snapshot of the desired parameter.
//    @param parameter_name - name of the parameter to be updated.
CmdUpdateComponent::CmdUpdateComponent(Entity::Id in_entity_id,
                                       ut::DynamicType::Handle in_component_type,
                                       ut::Function<CmdUpdateComponent::Callback> in_callback,
                                       ut::String in_parameter_name) noexcept : entity_id(in_entity_id)
                                                                              , component_type(in_component_type)
                                                                              , callback(ut::Move(in_callback))
                                                                              , parameter_name(ut::Move(in_parameter_name))
{}

// Calls ve::Environment::UpdateComponent().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdUpdateComponent::Execute(CmdAccessibleEnvironment& environment)
{
	ut::Optional<ut::Error> result =  environment.UpdateComponent(entity_id,
	                                                              component_type,
	                                                              callback,
	                                                              parameter_name);
	signal(result);
	return result;
}
// Connects provided function with signal that is triggered after a call
// to ve::Environment::UpdateComponent().
void CmdUpdateComponent::Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot)
{
	signal.Connect(ut::Move(slot));
}


//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//