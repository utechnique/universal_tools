//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_cmd_buffer.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformCmdBuffer::PlatformCmdBuffer(ID3D11CommandList* in_cmd_list,
                                     ID3D11DeviceContext* in_deferred_context) : cmd_list(in_cmd_list)
                                                                               , deferred_context(in_deferred_context)
{}

// Move constructor.
PlatformCmdBuffer::PlatformCmdBuffer(PlatformCmdBuffer&&) noexcept = default;

// Move operator.
PlatformCmdBuffer& PlatformCmdBuffer::operator =(PlatformCmdBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//