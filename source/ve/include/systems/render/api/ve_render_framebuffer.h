//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_target.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Framebuffer represents a set of render targets that can
// be bound to the graphics pipeline. It can have one depth-stencil
// target (optionally) and multiple color targets.
class Framebuffer : public PlatformFramebuffer
{
	friend class Context;
	friend class Device;

public:
	// ve::render::Framebuffer::Info conveniently stores all essential
	// information about a framebuffer.
	struct Info
	{
		ut::uint32 width = 0;
		ut::uint32 height = 0;
	};

	// Represents a target bound to the pipeline, one can select
	// an array slice and mip id that will be used as a render target.
	struct Attachment
	{
		Attachment(Target& in_target,
		           ut::uint32 in_array_slice = 0,
		           ut::uint32 in_mip_id = 0);

		SharedTargetData target;
		ut::uint32 array_slice;
		ut::uint32 mip;
	};

	// Constructor.
	Framebuffer(PlatformFramebuffer platform_framebuffer,
	            const Info& framebuffer_info,
	            ut::Array<Attachment> in_color_targets,
	            ut::Optional<Attachment> in_depth_stencil_target);

	// Move constructor.
	Framebuffer(Framebuffer&&) noexcept;

	// Move operator.
	Framebuffer& operator =(Framebuffer&&) noexcept;

	// Copying is prohibited.
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator =(const Framebuffer&) = delete;

	// Returns a const reference to the object with information about this framebuffer.
	const Info& GetInfo() const
	{
		return info;
	}

private:
	Info info;
	ut::Array<Attachment> color_attachments;
	ut::Optional<Attachment> depth_stencil_attachment;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//