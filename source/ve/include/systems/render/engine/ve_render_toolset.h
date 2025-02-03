//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "resources/ve_render_mesh.h"
#include "ve_render_cfg.h"
#include "ve_render_frame.h"
#include "ve_render_rc_mgr.h"
#include "ve_render_image_loader.h"
#include "ve_shader_loader.h"
#include "ve_sampler_cache.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Toolset is a set of simple independent tools to make rendering
// easier. This class is some kind of a swiss knife for bigger render modules.
class Toolset
{
public:
	// Constructor.
	Toolset(Device& device_ref);

	// render device
	Device& device;

	// parallelizes cpu work
	typedef ut::Scheduler<void, ut::DefaultCombiner<void> > DefaultScheduler;
	ut::ThreadPool<void> pool;
	DefaultScheduler scheduler;

	// tools
	Config<Settings> config;
	ResourceManager rc_mgr;
	ImageLoader img_loader;
	ShaderLoader shader_loader;
	SamplerCache sampler_cache;
	FrameManager frame_mgr;

	// common shaders
	struct Shaders
	{
		Shader quad_vs;
		Shader img_quad_ps;
		Shader img_quad_rgb2srgb_ps;
		Shader img_quad_srgb2rgb_ps;
	} shaders;

	// common formats
	struct Formats
	{
		pixel::Format depth_stencil;
		pixel::Format preferred_depth_stencil = pixel::Format::d24_unorm_s8_uint;
		pixel::Format alternative_depth_stencil = pixel::Format::d32_float_s8_uint;
		pixel::Format light_buffer = pixel::Format::r16g16b16a16_float;
		pixel::Format ibl = pixel::Format::r16g16b16a16_float;
		pixel::Format gbuffer = pixel::Format::r16g16b16a16_float;
		pixel::Format hitmask = pixel::Format::r8g8b8a8_unorm;
		pixel::Format ldr = pixel::Format::r8g8b8a8_unorm;
	} formats;

private:
	// Returns a configuration object. Tries to load it from file, and creates
	// a default one if loading failed.
	static Config<Settings> LoadCfg();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
