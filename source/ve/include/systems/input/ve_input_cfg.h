//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_config.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Input key bindings.
class Bindings : public ut::meta::Reflective
{
public:
	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::String move_forward = "W",
	           move_backward = "S",
	           move_left = "A",
	           move_right = "D";
	ut::String select_entity = "LMB";
	ut::String observation_mode = "RMB";
	ut::String observation_source_x = "MouseX",
	           observation_source_y = "MouseY";
	ut::String zoom_source = "MouseWheel";
	ut::String multiplication_modifier = "LCtrl";
	ut::String delete_entity = "Delete";
};

//----------------------------------------------------------------------------//
// ve::input::Settings is a class containing input options and preferences.
class Settings : public ut::meta::Reflective
{
public:
	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	Bindings bindings;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
