//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_cmd.h"
#include "ve_entity.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::CmdUpdateComponent command updates desired component with provided
// serialized data.
class CmdUpdateComponent : public Cmd
{
public:
	// Constructor.
	//    @param entity_id - id of the entity owning the desired component.
	//    @param component_type - type of the component (must be derived
	//                            from the ve::Component class).
	//    @param serialized_data - serialized data in JSON format.
	//    @param parameter_name - name of the parameter to be updated, this
	//                            parameter is optional; if it is empty - the
	//                            whole component will be updated.
	CmdUpdateComponent(Entity::Id entity_id,
	                   ut::DynamicType::Handle component_type,
	                   ut::JsonDoc serialized_data,
	                   ut::Optional<ut::String> parameter_name) noexcept;

	// Calls ve::Environment::UpdateComponent().
	//    @param environment - reference to the environment
	//                         executing the command.
	//    @return - optional ut::Error if environment failed to execute
	//              the command.
	ut::Optional<ut::Error> Execute(Environment& environment) override;

private:
	// Entity owning the desired component.
	Entity::Id entity_id;

	// Type of the component (must be derived from ve::Component).
	ut::DynamicType::Handle component_type;

	// Serialized data in JSON format.
	ut::JsonDoc data;

	// Name of the parameter. This member is optional. If it is empty - the
	// whole component will be updated.
	ut::Optional<ut::String> parameter_name;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//