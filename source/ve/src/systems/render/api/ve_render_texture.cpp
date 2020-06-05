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
                 const ImageInfo& image_info) : PlatformTexture(ut::Move(platform_texture))
                                              , info(image_info)
{}

// Move constructor.
Texture::Texture(Texture&&) noexcept = default;

// Move operator.
Texture& Texture::operator =(Texture&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//