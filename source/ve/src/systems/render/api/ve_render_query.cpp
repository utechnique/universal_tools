//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_query.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
QueryBuffer::QueryBuffer(PlatformQueryBuffer platform_query_buffer,
                         const QueryBuffer::Info& info) : PlatformQueryBuffer(ut::Move(platform_query_buffer))
                                                        , info(info)
{}

// Move constructor.
QueryBuffer::QueryBuffer(QueryBuffer&&) noexcept = default;

// Move operator.
QueryBuffer& QueryBuffer::operator =(QueryBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//