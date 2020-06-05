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
// ve::render::ImageInfo conveniently stores all essential
// information about an image.
class ImageInfo
{
public:
	// Constructor.
	ImageInfo() : format(pixel::unknown)
	            , width(0)
	            , height(0)
	            , depth(0)
	{}

	pixel::Format format;
	ut::uint32 width;
	ut::uint32 height;
	ut::uint32 depth;
};

// ve::render::Texture interface manages texel data, which is structured memory.
class Texture : public PlatformTexture
{
public:
	// context is default-constructible
	Texture(PlatformTexture platform_texture, const ImageInfo& image_info);

	// Move constructor.
	Texture(Texture&&) noexcept;

	// Move operator.
	Texture& operator =(Texture&&) noexcept;

	// Copying is prohibited.
	Texture(const Texture&) = delete;
	Texture& operator =(const Texture&) = delete;

	// Returns a const reference to the object with
	// information about this image.
	const ImageInfo& GetInfo() const
	{
		return info;
	}

private:
	// Format of the texture can't be changed after creation.
	ImageInfo info;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//