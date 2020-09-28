//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_sampler_cache.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
SamplerCache::SamplerCache(Device& dvc_ref) noexcept : device(dvc_ref)
                                                     , linear_wrap(CreateSampler(dvc_ref,
                                                                                 Sampler::filter_linear,
                                                                                 Sampler::address_wrap))
                                                     , linear_clamp(CreateSampler(dvc_ref,
                                                                                  Sampler::filter_linear,
                                                                                  Sampler::address_clamp))
                                                     , linear_mirror(CreateSampler(dvc_ref,
                                                                                   Sampler::filter_linear,
                                                                                   Sampler::address_mirror))
                                                     , point_wrap(CreateSampler(dvc_ref,
                                                                                Sampler::filter_nearest,
                                                                                Sampler::address_wrap))
                                                     , point_clamp(CreateSampler(dvc_ref,
                                                                                 Sampler::filter_nearest,
                                                                                 Sampler::address_clamp))
                                                     , point_mirror(CreateSampler(dvc_ref,
                                                                                  Sampler::filter_nearest,
                                                                                  Sampler::address_mirror))
{}

//----------------------------------------------------------------------------->
// Creates a sampler.
//    @param device - reference to the render device.
//    @param filter - min mag mip filter.
//    @param address_mode - uvw address mode.
//    @return - sampler object.
Sampler SamplerCache::CreateSampler(Device& device,
                                    Sampler::Filter filter,
                                    Sampler::AddressMode address_mode)
{
	Sampler::Info sampler_info;
	sampler_info.mag_filter = filter;
	sampler_info.min_filter = filter;
	sampler_info.mip_filter = filter;
	sampler_info.address_u = address_mode;
	sampler_info.address_v = address_mode;
	sampler_info.address_w = address_mode;
	sampler_info.compare_op = compare::always;
	sampler_info.border_color = ut::Color<4, float>(0.0f);
	sampler_info.anisotropy_enable = false;
	sampler_info.max_anisotropy = 1.0f;
	sampler_info.mip_lod_bias = 0.0f;
	sampler_info.min_lod = 0.0f;
	sampler_info.max_lod = 4096.0f;
	ut::Result<Sampler, ut::Error> sampler_result = device.CreateSampler(sampler_info);
	return sampler_result.MoveOrThrow();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//