//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commands/ve_cmd_add_entity.h"
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
CmdAddEntity::CmdAddEntity(ut::Array< ut::UniquePtr<ve::Component> > in_components) noexcept : components(ut::Move(in_components))
{}

// Connects provided function with signal that is triggered after a call
// to ve::Environment::AddEntity().
void CmdAddEntity::Connect(ut::Function<void(const AddResult&)> slot)
{
	signal.Connect(ut::Move(slot));
}

// Calls ve::Environment::AddEntity().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdAddEntity::Execute(CmdAccessibleEnvironment& environment)
{
	ut::Result<Entity::Id, ut::Error> result = environment.AddEntity();
	if (result)
	{
		const size_t component_count = components.Count();
		for (size_t i = 0; i < component_count; i++)
		{
			ut::Optional<ut::Error> add_err = environment.AddComponent(result.Get(), ut::Move(components[i]));
			if (add_err)
			{
				result = ut::MakeError(add_err.Move());
				break;
			}
		}
	}
	signal(result);
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//