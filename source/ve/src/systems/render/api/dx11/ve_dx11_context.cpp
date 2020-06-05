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

// Begin a new render pass.
//    @param render_pass - reference to the render pass object.
//    @param framebuffer - reference to the framebuffer to be bound.
//    @param render_area - reference to the rectangle representing
//                         rendering area in pixels.
//    @param color_clear_values - array of colors to clear color
//                                render targets with.
//    @param depth_clear_value - value to clear depth buffer with.
//    @param stencil_clear_value - value to clear stencil buffer with.
void Context::BeginRenderPass(RenderPass& render_pass,
	                          Framebuffer& framebuffer,
	                          const ut::Rect<ut::uint32>& render_area,
	                          const ut::Array< ut::Color<4> >& color_clear_values,
	                          float depth_clear_value,
	                          ut::uint32 stencil_clear_value)
{
	// validate arguments
	UT_ASSERT(color_clear_values.GetNum() <= render_pass.color_slots.GetNum());
	UT_ASSERT(render_pass.color_slots.GetNum() == framebuffer.color_targets.GetNum());
	UT_ASSERT(render_pass.depth_stencil_slot ? framebuffer.depth_stencil_target : !framebuffer.depth_stencil_target);

	// unbind all possible render targets from shader bindings
#ifdef DEBUG
	ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	d3d11_context->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
#endif

	// form d3d mrt array
	const size_t color_target_count = framebuffer.color_targets.GetNum();
	const size_t clear_value_count = color_clear_values.GetNum();
	ut::Array<ID3D11RenderTargetView*> rtv(color_target_count);
	for (size_t i = 0; i < color_target_count; i++)
	{
		rtv[i] = framebuffer.color_targets[i]->rtv.Get();

		// clear color target
		if (i < clear_value_count && render_pass.color_slots[i].load_op == RenderTargetSlot::load_clear)
		{
			d3d11_context->ClearRenderTargetView(rtv[i], color_clear_values[i].GetData());
		}
	}

	// form d3d dsv resource
	ID3D11DepthStencilView* dsv = nullptr;
	if (framebuffer.depth_stencil_target)
	{
		dsv = framebuffer.depth_stencil_target.Get().dsv.Get();

		// clear depth and stencil
		if (render_pass.depth_stencil_slot.Get().load_op == RenderTargetSlot::load_clear)
		{
			d3d11_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			                                     depth_clear_value, stencil_clear_value);
		}
	}

	// set viewport
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = static_cast<float>(render_area.offset.X()) / framebuffer.info.width;
	viewport.TopLeftY = static_cast<float>(render_area.offset.Y()) / framebuffer.info.height;
	viewport.Width = static_cast<float>(render_area.extent.X()) / framebuffer.info.width;
	viewport.Height = static_cast<float>(render_area.extent.Y()) / framebuffer.info.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	d3d11_context->RSSetViewports(1, &viewport);

	// set targets
	d3d11_context->OMSetRenderTargets(static_cast<UINT>(color_target_count), rtv.GetAddress(), dsv);
}

// End current render pass.
void Context::EndRenderPass()
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//