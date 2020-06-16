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
               Image target_img,
               const RenderTargetInfo& target_info) : PlatformRenderTarget(ut::Move(platform_target))
                                                    , image(ut::Move(target_img))
                                                    , info(target_info)
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