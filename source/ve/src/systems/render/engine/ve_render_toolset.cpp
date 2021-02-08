//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Toolset::Toolset(Device& dvc_ref) noexcept : device(dvc_ref)
                                           , config(LoadCfg())
                                           , rc_mgr(dvc_ref, config)
                                           , img_loader(dvc_ref)
                                           , shader_loader(dvc_ref)
                                           , sampler_cache(dvc_ref)
                                           , frame_mgr(dvc_ref)
                                           , fullscreen_quad(rc_mgr.CreateRect(ut::Vector<2>(0), ut::Vector<2>(1)).MoveOrThrow())
                                           , cube(rc_mgr.CreateBox(ut::Vector<3>(0), ut::Vector<3>(1)).MoveOrThrow())
                                           , img_black(rc_mgr.CreateImage(1, 1, ut::Color<4, ut::byte>(0, 0, 0, 255)).MoveOrThrow())
                                           , img_white(rc_mgr.CreateImage(1, 1, ut::Color<4, ut::byte>(255, 255, 255, 255)).MoveOrThrow())
                                           , img_red(rc_mgr.CreateImage(1, 1, ut::Color<4, ut::byte>(255, 0, 0, 255)).MoveOrThrow())
                                           , img_green(rc_mgr.CreateImage(1, 1, ut::Color<4, ut::byte>(0, 255, 0, 255)).MoveOrThrow())
                                           , img_blue(rc_mgr.CreateImage(1, 1, ut::Color<4, ut::byte>(0, 0, 255, 255)).MoveOrThrow())
                                           , img_normal(rc_mgr.CreateImage(1, 1, ut::Color<4, ut::byte>(127, 127, 255, 255)).MoveOrThrow())
{}

//----------------------------------------------------------------------------->
// Returns a configuration object. Tries to load it from file, and creates
// a default one if loading failed.
Config<Settings> Toolset::LoadCfg()
{
	Config<Settings> config;
	ut::Optional<ut::Error> load_cfg_error = config.Load();
	if (load_cfg_error)
	{
		const ut::error::Code error_code = load_cfg_error->GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "Render config file is absent. Using default configuration..." << ut::cret;
			config.Save();
		}
		else
		{
			ut::log << "Fatal error while loading render config file." << ut::cret;
			throw load_cfg_error.Move();
		}
	}

	return config;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//