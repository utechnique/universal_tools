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
// A buffer interface accesses a buffer resource, which is unstructured memory.
// Buffers typically store vertex or index data, uniforms and custom data.
class Buffer : public PlatformBuffer
{
	friend class Device;
	friend class Context;
public:
	// Type of the buffer.
	enum Type
	{
		vertex,
		index,
		uniform,
		storage
	};

	// Specifies how buffer will be used.
	enum Usage
	{
		gpu, // can be modified by gpu
		gpu_cpu, // can be modified by gpu and cpu
		immutable // can't be modified after creation
	};

	// ve::render::Buffer::Info conveniently stores all
	// essential information about a buffer.
	struct Info
	{
		Type type;
		Usage usage;

		// size of the buffer in bytes
		size_t size;

		// cpu representation of the gpu data
		ut::Array<ut::byte> data;
	};

	// Constructor.
	Buffer(PlatformBuffer platform_buffer,
	       Buffer::Info buffer_info);

	// Move constructor.
	Buffer(Buffer&&) noexcept;

	// Move operator.
	Buffer& operator =(Buffer&&) noexcept;

	// Copying is prohibited.
	Buffer(const Buffer&) = delete;
	Buffer& operator =(const Buffer&) = delete;

	// Returns a const reference to the object with
	// information about this buffer.
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