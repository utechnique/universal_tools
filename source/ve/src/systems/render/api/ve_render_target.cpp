//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_target.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Target::Target(PlatformRenderTarget target,
               Image image,
               const Target::Info& target_info) : data(ut::MakeUnsafeShared<Target::Data>(ut::Move(target),
                                                                                          ut::Move(image),
                                                                                          target_info))
{}

Target::Data::Data(PlatformRenderTarget base_target,
                   Image target_image,
                   const Info& target_info) : platform_target(ut::Move(base_target))
                                            , image(ut::Move(target_image))
                                            , info(target_info)
{}

// Move constructor.
Target::Target(Target&&) noexcept = default;
Target::Data::Data(Data&&) noexcept = default;

// Move operator.
Target& Target::operator =(Target&&) noexcept = default;
Target::Data& Target::Data::operator =(Data&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//