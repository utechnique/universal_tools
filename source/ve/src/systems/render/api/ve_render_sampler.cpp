//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_sampler.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Sampler::Sampler(PlatformSampler platform_sampler,
                 const Sampler::Info& sampler_info) : PlatformSampler(ut::Move(platform_sampler))
                                                    , info(sampler_info)
{}

// Move constructor.
Sampler::Sampler(Sampler&&) noexcept = default;

// Move operator.
Sampler& Sampler::operator =(Sampler&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//