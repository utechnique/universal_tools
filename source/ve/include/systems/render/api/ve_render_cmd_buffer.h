//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::CmdBuffer encapsulates a list of graphics commands for rendering.
// One can record buffer calling render::Device::Record() and execute recorded
// commands with render::Device::Submit().
class CmdBuffer : public PlatformCmdBuffer
{
	friend class Context;
	friend class Device;

public:
	// Enumeration of flags specifying if the command buffer is primary or
	// secondary command buffer.
	enum Level
	{
		level_primary,
		level_secondary
	};

	// Enumeration of possible bits specifying usage of the command buffer.
	enum Usage
	{
		// Normal usage.
		usage_normal = 0x00000000,

		// Specifies that this command buffer will be reset after the call to
		// Device::WaitCmdBuffer() function.
		usage_dynamic = 0x00000001,

		// Makes sense only for secondary buffers. Specifies that a secondary
		// command buffer is considered to be entirely inside a render pass.
		// If this is a primary command buffer, then this bit is ignored.
		usage_inside_render_pass = 0x00000002
	};

	// ve::render::CmdBuffer::Info conveniently stores all essential
	// command buffer information.
	class Info
	{
	public:
		// Constructor.
		Info() : level(level_primary)
		       , usage(usage_normal)
		{}

		Level level;
		ut::uint32 usage;
	};

	// Constructor.
	CmdBuffer(PlatformCmdBuffer platform_cmd_buffer,
	          const Info& cmd_buffer_info);

	// Move constructor.
	CmdBuffer(CmdBuffer&&) noexcept;

	// Move operator.
	CmdBuffer& operator =(CmdBuffer&&) noexcept;

	// Copying is prohibited.
	CmdBuffer(const CmdBuffer&) = delete;
	CmdBuffer& operator =(const CmdBuffer&) = delete;

	// Returns a const reference to the object with information about this command buffer.
	const Info& GetInfo() const
	{
		return info;
	}

private:
	Info info;
	bool pending;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//