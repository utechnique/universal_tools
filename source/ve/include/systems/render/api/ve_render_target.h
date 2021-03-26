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
// Target data is separated logically from the target object to provide
// efficient linking capabilities. For example framebuffer must have links
// to it's render targets, but if user will be forced to use some kind
// of explicit smart pointers - eventually we can get nullptr and some other
// drawbacks. Current implementation provides convenient use of the target
// object without smart pointers and reliable linking with other resources.
struct TargetData : public PlatformRenderTarget
{
	// Conveniently stores all essential information about this render target.
	struct Info
	{
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
			state_resource,

			// image can be used as a transfer source
			state_transfer_src,

			// image can be used as a transfer destination
			state_transfer_dst
		};

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
	TargetData(PlatformRenderTarget base_target,
		       Image target_image,
		       const Info& target_info);

	// Move constructor.
	TargetData(TargetData&&) noexcept;

	// Move operator.
	TargetData& operator =(TargetData&&) noexcept;

	// Copying is prohibited.
	TargetData(const TargetData&) = delete;
	TargetData& operator =(const TargetData&) = delete;

	// Image associated with this target.
	Image image;

	// Information about this target.
	Info info;
};
typedef ut::SharedPtr<TargetData, ut::thread_safety::off> SharedTargetData;

//----------------------------------------------------------------------------//
// ut::render::Target is an interface to render data to an image.
class Target : private SharedTargetData
{
	friend class Device;
	friend class Context;
	friend class Framebuffer;
public:
	typedef TargetData::Info Info;

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
		return Get()->info;
	}

	// Returns a const reference to the image
	// associated with this render target.
	const Image& GetImage() const
	{
		return Get()->image;
	}

	// Returns a reference to the image
	// associated with this render target.
	Image& GetImage()
	{
		return Get()->image;
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//