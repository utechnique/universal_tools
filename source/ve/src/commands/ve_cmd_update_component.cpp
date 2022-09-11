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
//    @param serialized_data - serialized data in JSON format.
//    @param parameter_name - name of the parameter to be updated, this
//                            parameter is optional; if it is empty - the
//                            whole component will be updated.
CmdUpdateComponent::CmdUpdateComponent(Entity::Id in_entity_id,
                                       ut::DynamicType::Handle in_component_type,
                                       ut::JsonDoc in_serialized_data,
                                       ut::Optional<ut::String> in_parameter_name) noexcept : entity_id(in_entity_id)
                                                                                            , component_type(in_component_type)
                                                                                            , data(ut::Move(in_serialized_data))
                                                                                            , parameter_name(ut::Move(in_parameter_name))
{}

// Calls ve::Environment::UpdateComponent().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdUpdateComponent::Execute(CmdAccessibleEnvironment& environment)
{
	return environment.UpdateComponent(entity_id,
	                                   component_type,
	                                   data,
	                                   parameter_name);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//