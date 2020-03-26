//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_texture.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ut::render::Target is an interface to render data to textures.
class Target : public PlatformRenderTarget
{
public:
	// Constructor.
	Target(PlatformRenderTarget platform_target, Texture texture);

	// Move constructor.
	Target(Target&&) noexcept;

	// Move operator.
	Target& operator =(Target&&) noexcept;

	// Copying is prohibited.
	Target(const Target&) = delete;
	Target& operator =(const Target&) = delete;

	// Texture buffer associated with render target.
	Texture buffer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//