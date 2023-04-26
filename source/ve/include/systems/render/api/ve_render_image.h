//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_pixel_format.h"
#include "systems/render/api/ve_render_memory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Image interface manages texel data, which is structured memory.
class Image : public PlatformImage
{
public:
	// Basic dimensionality of an image.
	enum Type
	{
		type_1D,
		type_2D,
		type_3D,
		type_cube
	};

	// Cubemap traits.
	struct Cube
	{
		enum Face : ut::uint32
		{
			positive_x = 0,
			negative_x = 1,
			positive_y = 2,
			negative_y = 3,
			positive_z = 4,
			negative_z = 5,
		};
	};

	// Provides access to image data.
	struct MappedResource
	{
		void* data;
		size_t row_pitch;
		size_t depth_pitch;
		size_t array_pitch;
	};

	// ve::render::Image::Info conveniently stores all essential
	// information about an image.
	struct Info
	{
		Type type = type_2D;
		pixel::Format format = pixel::unknown;
		memory::Usage usage = render::memory::gpu_immutable;
		bool has_staging_cpu_read_buffer = false;
		ut::uint32 mip_count = 1;
		ut::uint32 width = 1;
		ut::uint32 height = 1;
		ut::uint32 depth = 1;
		ut::Array<ut::byte> data;
	};

	// Constructor.
	Image(PlatformImage platform_img, Info img_info);

	// Move constructor.
	Image(Image&&) noexcept;

	// Move operator.
	Image& operator =(Image&&) noexcept;

	// Copying is prohibited.
	Image(const Image&) = delete;
	Image& operator =(const Image&) = delete;

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