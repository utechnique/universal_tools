//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_context.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformContext::PlatformContext(ID3D11DeviceContext* context_ptr) : d3d11_context(context_ptr)
{}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&&) noexcept = default;

// Move operator.
PlatformContext& PlatformContext::operator =(PlatformContext&& other) noexcept = default;

//----------------------------------------------------------------------------//
// Constructor.
Context::Context(PlatformContext platform_context) : PlatformContext(ut::Move(platform_context))
{}

// Set all the elements in a render target to one value.
void Context::ClearTarget(Target& target, float* color)
{
	if (target.rtv)
	{
		d3d11_context->ClearRenderTargetView(target.rtv.Get(), color);
	}
	else if (target.dsv)
	{
		d3d11_context->ClearDepthStencilView(target.dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, *color, 0);
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//