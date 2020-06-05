//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_cmd_buffer.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
CmdBuffer::CmdBuffer(PlatformCmdBuffer platform_cmd_buffer,
                     const CmdBufferInfo& cmd_buffer_info) : PlatformCmdBuffer(ut::Move(platform_cmd_buffer))
                                                           , info(cmd_buffer_info)
{}

// Move constructor.
CmdBuffer::CmdBuffer(CmdBuffer&&) noexcept = default;

// Move operator.
CmdBuffer& CmdBuffer::operator =(CmdBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//