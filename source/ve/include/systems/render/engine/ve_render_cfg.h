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

	// set 'true' to enable image based lighting
	bool ibl_enabled;

	// width and height of the ibl cubemap in pixels
	ut::uint32 ibl_size;

	// identifies how many IBL faces are rendered in one frame
	ut::uint32 ibl_frequency;

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
