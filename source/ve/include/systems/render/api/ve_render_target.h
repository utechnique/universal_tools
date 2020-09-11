//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_image.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ut::render::Target is an interface to render data to textures.
class Target : public PlatformRenderTarget
{
public:
	// ve::render::Target::Info conveniently stores all essential
	// information about render target.
	struct Info
	{
		enum Usage
		{
			// render target is a view of color image
			usage_color,

			// render target is a view of depth-stencil image
			usage_depth,

			// render target is a view of one of the buffers in a swap 
			// chain and is intended to display final image to user
			usage_present
		};

		Usage usage = usage_color;
	};

	// Constructor.
	Target(PlatformRenderTarget platform_target,
	       Image image,
	       const Info& target_info);

	// Move constructor.
	Target(Target&&) noexcept;

	// Move operator.
	Target& operator =(Target&&) noexcept;

	// Copying is prohibited.
	Target(const Target&) = delete;
	Target& operator =(const Target&) = delete;

	// Returns a const reference to the object with
	// information about this render target.
	const Info& GetInfo() const
	{
		return info;
	}

	// Image associated with render target.
	Image image;

private:
	Info info;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//