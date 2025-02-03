//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_cmp.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Sampler encapsulates sampling information for a texture.
class Sampler : public PlatformSampler
{
public:
	// Filtering options during texture sampling.
	enum class Filter
	{
		nearest,
		linear
	};

	// Identifies a technique for resolving texture coordinates that are
	// outside of the boundaries of a texture.
	enum class AddressMode
	{
		wrap,
		mirror,
		clamp,
		border
	};

	// ve::render::Sampler::Info conveniently stores all essential
	// information about a sampler.
	struct Info
	{
		Filter mag_filter = Filter::linear;
		Filter min_filter = Filter::linear;
		Filter mip_filter = Filter::linear;
		AddressMode address_u = AddressMode::wrap;
		AddressMode address_v = AddressMode::wrap;
		AddressMode address_w = AddressMode::wrap;
		ut::Optional<compare::Operation> compare_op;
		ut::Color<4, float> border_color = ut::Color<4, float>(0.0f);
		bool anisotropy_enable = false;
		float max_anisotropy = 1.0f;
		float mip_lod_bias = 0.0f;
		float min_lod = 0.0f;
		float max_lod = 4096.0f;
	};

	// Constructor.
	Sampler(PlatformSampler platform_sampler, const Info& sampler_info);

	// Move constructor.
	Sampler(Sampler&&) noexcept;

	// Move operator.
	Sampler& operator =(Sampler&&) noexcept;

	// Copying is prohibited.
	Sampler(const Sampler&) = delete;
	Sampler& operator =(const Sampler&) = delete;

	// Returns a const reference to the object with
	// information about this image.
	const Info& GetInfo() const
	{
		return info;
	}

private:
	Info info;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//