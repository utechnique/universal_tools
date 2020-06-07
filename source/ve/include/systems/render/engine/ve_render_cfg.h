//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_config.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Settings is a class containing rendering options and preferences.
class Settings : public ut::meta::Reflective
{
public:
	// Constructor, default values are set here.
	Settings();

	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	// vertical synchronization (on/off)
	bool vsync;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
