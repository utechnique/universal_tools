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
// ut::render::Target is an interface to render data to an image.
class Target
{
	friend class Device;
	friend class Context;
	friend class Framebuffer;
public:
	// Enumeration of possible ways how a target can be used.
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

	// Enumeration of possible target states.
	enum State
	{
		// image be a render target but can't be bound as a shader resource
		state_target,

		// image can be used as a shader resource but can't be rendered to
		state_resource
	};

	// ve::render::Target::Info conveniently stores all essential
	// information about render target.
	struct Info
	{		
		Image::Type type = Image::type_2D;
		pixel::Format format = pixel::unknown;
		Usage usage = usage_color;
		State state = state_target;
		ut::uint32 mip_count = 1;
		ut::uint32 width = 1;
		ut::uint32 height = 1;
		ut::uint32 depth = 1;
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
		UT_ASSERT(data);
		return data->info;
	}

	// Returns a const reference to the image
	// associated with this render target.
	const Image& GetImage() const
	{
		UT_ASSERT(data);
		return data->image;
	}

	// Returns a reference to the image
	// associated with this render target.
	Image& GetImage()
	{
		UT_ASSERT(data);
		return data->image;
	}

private:
	// Target data is separated logically from the target object to provide
	// efficient linking capabilities. For example framebuffer must have links
	// to it's render targets, but if user will be forced to use some kind
	// of explicit smart pointers - eventually we can get nullptr and some other
	// drawbacks. Current implementation provides convenient use of the target
	// object without smart pointers and reliable linking with other resources.
	struct Data
	{
		// Constructor.
		Data(PlatformRenderTarget base_target,
		     Image target_image,
		     const Info& target_info);

		// Move constructor.
		Data(Data&&) noexcept;

		// Move operator.
		Data& operator =(Data&&) noexcept;

		// Copying is prohibited.
		Data(const Data&) = delete;
		Data& operator =(const Data&) = delete;

		PlatformRenderTarget platform_target;

		// Image associated with render target.
		Image image;

		// Information about this target.
		Info info;
	};

	typedef ut::SharedPtr<Data, ut::thread_safety::off> SharedData;
	
	SharedData data;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//