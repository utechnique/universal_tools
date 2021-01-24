//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/input/ve_input_cfg.h"
#include "ve_default.h"
//----------------------------------------------------------------------------//
template<> const char* ve::Config<ve::input::Settings>::skName = "input";
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Bindings::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(move_forward, "move_forward");
	snapshot.Add(move_backward, "move_backward");
	snapshot.Add(move_left, "move_left");
	snapshot.Add(move_right, "move_right");
	snapshot.Add(observation_mode, "observation_mode");
	snapshot.Add(observation_source_x, "observation_source_x");
	snapshot.Add(observation_source_y, "observation_source_y");
}

//----------------------------------------------------------------------------//
// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(bindings, "bindings");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
