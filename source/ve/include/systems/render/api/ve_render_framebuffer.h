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
// ve::render::FramebufferInfo conveniently stores all essential
// information about a framebuffer.
class FramebufferInfo
{
public:
	FramebufferInfo(ut::uint32 in_width = 0,
	                ut::uint32 in_height = 0) : width(in_width)
	                                          , height(in_height)
	{}

	ut::uint32 width;
	ut::uint32 height;
};

// ve::render::Framebuffer represents a set of render targets that can
// be bound to the graphics pipeline. It can have one depth-stencil
// target (optionally) and multiple color targets.
class Framebuffer : public PlatformFramebuffer
{
	friend class Context;
	friend class Device;

public:
	// Constructor.
	Framebuffer(PlatformFramebuffer platform_framebuffer,
	            const FramebufferInfo& framebuffer_info,
	            ut::Array< ut::Ref<Target> > in_color_targets,
	            ut::Optional<Target&> in_depth_stencil_target = ut::Optional<Target&>());

	// Move constructor.
	Framebuffer(Framebuffer&&) noexcept;

	// Move operator.
	Framebuffer& operator =(Framebuffer&&) noexcept;

	// Copying is prohibited.
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator =(const Framebuffer&) = delete;

	// Returns a const reference to the object with information about this framebuffer.
	const FramebufferInfo& GetInfo() const
	{
		return info;
	}

private:
	ut::Array< ut::Ref<Target> > color_targets;
	ut::Optional<Target&> depth_stencil_target;
	FramebufferInfo info;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//