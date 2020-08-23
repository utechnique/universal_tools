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

	// specifies how many frames can be concurrently processed by
	// the engine in a moment
	ut::uint32 frames_in_flight;

private:
	// Validates data after loading.
	void Validate();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
