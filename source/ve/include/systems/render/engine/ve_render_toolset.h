//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "systems/render/resources/ve_render_mesh.h"
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
	Toolset(Device& device_ref) noexcept;

	// render device
	Device& device;

	// parallelizes cpu work
	typedef ut::Scheduler<void, ut::DefaultCombiner<void>,
	                      ut::pool_sync::cond_var> DefaultScheduler;
	ut::ThreadPool<void, ut::pool_sync::cond_var> pool;
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
