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
// ve::render::QueryBuffer encapsulates a set of GPU queries of the same type.
class QueryBuffer : public PlatformQueryBuffer
{
public:
	// List of possible pipeline stages to write a query in.
	enum class PipelineStage
	{
		top_of_pipe,
		bottom_of_pipe
	};

	// List of possible query types.
	enum class Type
	{
		occlusion,
		timestamp
	};

	// ve::render::QueryBuffer::Info conveniently stores all essential
	// information about the managed queries.
	struct Info
	{
		Type type;
		ut::uint32 count;
	};

	// Constructor.
	QueryBuffer(PlatformQueryBuffer platform_query_buffer, const Info& info);

	// Move constructor.
	QueryBuffer(QueryBuffer&&) noexcept;

	// Move operator.
	QueryBuffer& operator =(QueryBuffer&&) noexcept;

	// Copying is prohibited.
	QueryBuffer(const QueryBuffer&) = delete;
	QueryBuffer& operator =(const QueryBuffer&) = delete;

	// Returns a const reference to the object with
	// information about this query buffer.
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