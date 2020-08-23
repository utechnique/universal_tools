//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_cfg.h"
#include "ve_default.h"
//----------------------------------------------------------------------------//
template<> const char* ve::Config<ve::render::Settings>::skName = "render";
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
static const ut::uint32 skDefaultFramesInFlightCount = 2;
static const ut::uint32 skMaxFramesInFlightCount = 3;
//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : vsync(true)
                     , frames_in_flight(skDefaultFramesInFlightCount)
{}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(vsync, "vertical_synchronization");
	snapshot.Add(frames_in_flight, "frames_in_flight");
	snapshot.SetPostLoadCallback(ut::MemberFunction<Settings, void()>(this, &Settings::Validate));
}

// Validates data after loading.
void Settings::Validate()
{
	if (frames_in_flight == 0)
	{
		frames_in_flight = skDefaultFramesInFlightCount;
		ut::log.Lock() << "Warning: frames in flight count is zero. " <<
		                  "Changed to the default value: " << frames_in_flight << ut::cret;
	}
	else if (frames_in_flight > skMaxFramesInFlightCount)
	{
		frames_in_flight = skMaxFramesInFlightCount;
		ut::log.Lock() << "Warning: frames in flight count is too big. " <<
		                  "Changed to the maximum value: " << frames_in_flight << ut::cret;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
