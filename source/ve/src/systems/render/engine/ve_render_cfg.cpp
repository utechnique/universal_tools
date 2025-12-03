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
static const ut::uint32 skDefaultSwapchainBufferCount = 3;
static const ut::uint32 skMaxFramesInFlightCount = 3;
//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : vsync(true)
                     , frames_in_flight(skDefaultFramesInFlightCount)
                     , swapchain_buffer_count(skDefaultSwapchainBufferCount)
                     , ibl_enabled(true)
                     , ibl_size(256)
                     , ibl_frequency(1)
                     , show_fps(true)
                     , show_render_stat(false)
{}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(vsync, "vertical_synchronization");
	snapshot.Add(frames_in_flight, "frames_in_flight");
	snapshot.Add(swapchain_buffer_count, "swapchain_buffer_count");
	snapshot.Add(ibl_enabled, "ibl_enabled");
	snapshot.Add(ibl_size, "ibl_size");
	snapshot.Add(ibl_frequency, "ibl_frequency");
	snapshot.Add(show_fps, "show_fps");
	snapshot.Add(show_render_stat, "show_render_stat");
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

	if (swapchain_buffer_count == 0)
	{
		swapchain_buffer_count = skDefaultSwapchainBufferCount;
		ut::log.Lock() << "Warning: swapchain buffer count is zero. " <<
			"Changed to the default value: " << swapchain_buffer_count << ut::cret;
	}
	else if (swapchain_buffer_count < frames_in_flight)
	{
		swapchain_buffer_count = frames_in_flight;
		ut::log.Lock() << "Warning: swapchain buffer count cannot be "
			"lower than the number of frames in flight. "
			"Changed to the value: " << swapchain_buffer_count << ut::cret;
	}

	if (ibl_frequency != 1 && ibl_frequency != 2 && ibl_frequency != 3 && ibl_frequency != 6)
	{
		const ut::uint32 old_frequency = ibl_frequency;
		ibl_frequency = 1;
		ut::log.Lock() << "Warning: IBL frequency is " << old_frequency
		               << ". Valid values are 1, 2, 3, 6. Changed to "
		               << ibl_frequency << "." << ut::cret;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
