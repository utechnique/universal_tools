//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_frame.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Frame::Frame(CmdBuffer in_cmd_buffer,
             Buffer in_display_ub) : cmd_buffer(ut::Move(in_cmd_buffer))
                                   , display_ub(ut::Move(in_display_ub))
{}

// Move constructor.
Frame::Frame(Frame&&) noexcept = default;

// Move operator.
Frame& Frame::operator =(Frame&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//