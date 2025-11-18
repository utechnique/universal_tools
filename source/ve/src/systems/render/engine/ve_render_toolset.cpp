//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Generates an array of macros for quad shader enabling rgb->srgb or
// srgb->rgb conversion.
//    @param rgb_to_srgb - 'true' to convert from rgb to srgb and
//                         'false' to convert from srgb to rgb.
//    @return - an array of macros.
Shader::Macros GenQuadShaderConversionMacros(bool rgb_to_srgb)
{
	Shader::Macros out;
	Shader::MacroDefinition conversion;
	conversion.name = rgb_to_srgb ? "RGB_TO_SRGB" : "SRGB_TO_RGB";
	conversion.value = "1";
	out.Add(ut::Move(conversion));
	return out;
}

//----------------------------------------------------------------------------//
// Constructor.
Toolset::Toolset(Device& dvc_ref) : device(dvc_ref)
#if UT_LINUX
                                  , pool(ut::Min<ut::uint32>(ut::GetNumberOfProcessors(), 4))
#else
                                  , pool(ut::GetNumberOfProcessors())
#endif
                                  , scheduler(pool.CreateScheduler())
                                  , config(LoadCfg())
                                  , rc_mgr(dvc_ref, config)
                                  , img_loader(dvc_ref)
                                  , shader_loader(dvc_ref)
                                  , sampler_cache(dvc_ref)
                                  , frame_mgr(dvc_ref)
                                  , shaders {
                                                shader_loader.Load(Shader::Stage::vertex,
                                                                   "quad_vs",
                                                                   "VS",
                                                                   "quad_vs.hlsl").MoveOrThrow(),
                                                shader_loader.Load(Shader::Stage::pixel,
                                                                   "img_quad_ps",
                                                                   "PS",
                                                                   "img_quad_ps.hlsl").MoveOrThrow(),
                                                shader_loader.Load(Shader::Stage::pixel,
                                                                   "img_quad_rgb2srgb_ps",
                                                                   "PS",
                                                                   "img_quad_ps.hlsl",
                                                                   GenQuadShaderConversionMacros(true)).MoveOrThrow(),
                                                shader_loader.Load(Shader::Stage::pixel,
                                                                   "img_quad_srgb2rgb_ps",
                                                                   "PS",
                                                                   "img_quad_ps.hlsl",
                                                                   GenQuadShaderConversionMacros(false)).MoveOrThrow()
                                            }
{
    // check supported depth-stencil formats
    const Device::Info& device_info = device.GetInfo();
    const bool supports_preferred_depth_format = device_info.supports_2d_render_target_format[static_cast<size_t>(formats.preferred_depth_stencil)];
    const bool supports_alternative_depth_format = device_info.supports_2d_render_target_format[static_cast<size_t>(formats.alternative_depth_stencil)];
    if (!supports_preferred_depth_format && !supports_alternative_depth_format)
    {
        throw ut::Error(ut::error::not_supported, "Depth-stencil format is not supported.");
    }

    // choose supported depth-stencil format
    formats.depth_stencil = supports_preferred_depth_format ?
                            formats.preferred_depth_stencil :
                            formats.alternative_depth_stencil;

    // check supported G-Buffer base color format
    if (!device_info.supports_2d_render_target_format[static_cast<size_t>(formats.gbuffer_base_color)])
    {
        throw ut::Error(ut::error::not_supported, "G-Buffer(base color) format is not supported.");
    }

    // check supported G-Buffer normal format
    if (!device_info.supports_2d_render_target_format[static_cast<size_t>(formats.gbuffer_normal)])
    {
        throw ut::Error(ut::error::not_supported, "G-Buffer(normal) format is not supported.");
    }

    // check supported light-buffer format
    if (!device_info.supports_2d_render_target_format[static_cast<size_t>(formats.light_buffer)])
    {
        throw ut::Error(ut::error::not_supported, "Light buffer format is not supported.");
    }
}

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