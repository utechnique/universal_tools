//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_target.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Target::Target(PlatformRenderTarget platform_target,
               Texture texture) : PlatformRenderTarget(ut::Move(platform_target))
                                , buffer(ut::Move(texture))
{}

// Move constructor.
Target::Target(Target&&) noexcept = default;

// Move operator.
Target& Target::operator =(Target&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//