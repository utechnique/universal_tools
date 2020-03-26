//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_pixel_format.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Texture interface manages texel data, which is structured memory.
class Texture : public PlatformTexture
{
public:
	// context is default-constructible
	Texture(PlatformTexture platform_texture, pixel::Format pixel_format);

	// Move constructor.
	Texture(Texture&&) noexcept;

	// Move operator.
	Texture& operator =(Texture&&) noexcept;

	// Copying is prohibited.
	Texture(const Texture&) = delete;
	Texture& operator =(const Texture&) = delete;

	// Returns pixel format of the texture, see ve::render::pixel::Format.
	pixel::Format GetFormat() const;

private:
	// Format of the texture can't be changed after creation.
	pixel::Format format;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//