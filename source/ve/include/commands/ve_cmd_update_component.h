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
// serialized data. User is expected to inherit another class from the 
// ve::CmdUpdateComponent in order to pass custom data to the @callback.
class CmdUpdateComponent : public Cmd
{
public:
	// Callback function type.
	typedef ut::Optional<ut::Error> Callback(ut::meta::Snapshot&);

	// Constructor.
	//    @param entity_id - id of the entity owning the desired component.
	//    @param component_type - type of the component (must be derived
	//                            from the ve::Component class).
    //    @param callback - the callback that accepts a reference
    //                      to the meta-snapshot of the desired parameter.
	//    @param parameter_name - name of the parameter to be updated.
	CmdUpdateComponent(Entity::Id entity_id,
	                   ut::DynamicType::Handle component_type,
	                   ut::Function<ut::Optional<ut::Error>(ut::meta::Snapshot&)> callback,
	                   ut::String parameter_name) noexcept;

	// Calls ve::Environment::UpdateComponent().
	//    @param environment - reference to the environment
	//                         executing the command.
	//    @return - optional ut::Error if environment failed to execute
	//              the command.
	virtual ut::Optional<ut::Error> Execute(CmdAccessibleEnvironment& environment) override;

	// Connects provided function with signal that is triggered after a call
	// to ve::Environment::UpdateComponent().
	void Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot);

protected:
	// Entity owning the desired component.
	Entity::Id entity_id;

	// Type of the component (must be derived from ve::Component).
	ut::DynamicType::Handle component_type;

	// Callback.
	ut::Function<Callback> callback;

	// Name of the parameter.
	ut::String parameter_name;

	// Signal that is triggered after a call to ve::Environment::UpdateComponent().
	ut::Signal<void(const ut::Optional<ut::Error>&)> signal;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//