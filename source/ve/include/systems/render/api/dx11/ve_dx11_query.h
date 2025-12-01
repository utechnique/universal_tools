//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "ut.h"
#include <d3d11.h>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// DirectX 11 queries.
class PlatformQueryBuffer
{
	friend class Device;
	friend class Context;
	friend class PlatformContext;
public:
	// Constructor.
	PlatformQueryBuffer(ut::Array<ID3D11Query*> query_pointers,
	                    ID3D11Query* disjoint_query_ptr);

	// Move constructor.
	PlatformQueryBuffer(PlatformQueryBuffer&&) noexcept;

	// Move operator.
	PlatformQueryBuffer& operator =(PlatformQueryBuffer&&) noexcept;

	// Copying is prohibited.
	PlatformQueryBuffer(const PlatformQueryBuffer&) = delete;
	PlatformQueryBuffer& operator =(const PlatformQueryBuffer&) = delete;

private:
	ut::Array<ut::ComPtr<ID3D11Query>> queries;
	ut::ComPtr<ID3D11Query> disjoint_query;
	bool started_disjoint_query = false;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//