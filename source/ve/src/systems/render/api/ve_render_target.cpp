//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_target.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Render target.
Target::Target(PlatformRenderTarget target,
               Image image,
               const Target::Info& target_info) : SharedTargetData(ut::MakeUnsafeShared<TargetData>(ut::Move(target),
                                                                                                    ut::Move(image),
                                                                                                    target_info))
{}

Target::Target(Target&&) noexcept = default;
Target& Target::operator =(Target&&) noexcept = default;

//----------------------------------------------------------------------------->
// Target Data.
TargetData::TargetData(PlatformRenderTarget base_target,
                       Image target_image,
                       const Info& target_info) : PlatformRenderTarget(ut::Move(base_target))
                                                , image(ut::Move(target_image))
                                                , info(target_info)
{}

TargetData::TargetData(TargetData&&) noexcept = default;
TargetData& TargetData::operator =(TargetData&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//