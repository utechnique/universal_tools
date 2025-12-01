//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_query.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformQueryBuffer::PlatformQueryBuffer(ut::Array<ID3D11Query*> query_pointers,
                                         ID3D11Query* disjoint_query_ptr) : disjoint_query(disjoint_query_ptr)
{
	const size_t query_count = query_pointers.Count();

	queries.Resize(query_count);

	for (size_t query_iter = 0; query_iter < query_count; query_iter++)
	{
		queries[query_iter] = ut::ComPtr<ID3D11Query>(query_pointers[query_iter]);
	}
}

// Move constructor.
PlatformQueryBuffer::PlatformQueryBuffer(PlatformQueryBuffer&&) noexcept = default;

// Move operator.
PlatformQueryBuffer& PlatformQueryBuffer::operator =(PlatformQueryBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//