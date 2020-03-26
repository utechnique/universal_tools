//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_texture.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Texture::Texture(PlatformTexture platform_texture,
                 pixel::Format pixel_format) : PlatformTexture(ut::Move(platform_texture))
                                             , format(pixel_format)
{}

// Move constructor.
Texture::Texture(Texture&&) noexcept = default;

// Move operator.
Texture& Texture::operator =(Texture&&) noexcept = default;

// Returns pixel format of the texture, see ve::render::pixel::Format.
pixel::Format Texture::GetFormat() const
{
	return format;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//