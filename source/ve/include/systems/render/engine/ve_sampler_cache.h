//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ResourceManager is a helper class to conveniently operate with
// render resources.
class SamplerCache
{
public:
	// Constructor.
	SamplerCache(Device& device_ref) noexcept;

	// samplers
	Sampler linear_wrap;
	Sampler linear_clamp;
	Sampler linear_mirror;
	Sampler point_wrap;
	Sampler point_clamp;
	Sampler point_mirror;

private:
	// Creates a sampler.
	//    @param device - reference to the render device.
	//    @param filter - min mag mip filter.
	//    @param address_mode - uvw address mode.
	//    @return - sampler object.
	static Sampler CreateSampler(Device& device,
	                             Sampler::Filter filter,
	                             Sampler::AddressMode address_mode);

	Device& device;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
