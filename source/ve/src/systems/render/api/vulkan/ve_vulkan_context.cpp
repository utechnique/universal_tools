//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_context.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Maximum number of clear values.
const ut::uint32 skMaxClearValues = 16;

//----------------------------------------------------------------------------//
// Constructor.
PlatformContext::PlatformContext(VkDevice device_handle,
                                 VkCommandBuffer cmd_buffer_handle) : device(device_handle)
                                                                    , cmd(cmd_buffer_handle)
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
	UT_ASSERT(color_clear_values.GetNum() <= skMaxClearValues);
	UT_ASSERT(color_clear_values.GetNum() <= render_pass.color_slots.GetNum());
	UT_ASSERT(render_pass.color_slots.GetNum() == framebuffer.color_targets.GetNum());
	UT_ASSERT(render_pass.depth_stencil_slot ? framebuffer.depth_stencil_target : !framebuffer.depth_stencil_target);

	const ut::uint32 clear_val_count = static_cast<ut::uint32>(color_clear_values.GetNum());
	const ut::uint32 color_slot_count = static_cast<ut::uint32>(render_pass.color_slots.GetNum());
	ut::uint32 total_clear_val_count = clear_val_count;

	// color clear values
	VkClearValue vk_clear_values[skMaxClearValues];
	for (ut::uint32 i = 0; i < clear_val_count; i++)
	{
		vk_clear_values[i].color.float32[0] = color_clear_values[i].R();
		vk_clear_values[i].color.float32[1] = color_clear_values[i].G();
		vk_clear_values[i].color.float32[2] = color_clear_values[i].B();
		vk_clear_values[i].color.float32[3] = color_clear_values[i].A();
	}

	// depth and stencil clear value
	if (render_pass.depth_stencil_slot)
	{
		// note that depth-stencil slot goes right after all color slots
		vk_clear_values[color_slot_count].depthStencil.depth = depth_clear_value;
		vk_clear_values[color_slot_count].depthStencil.stencil = stencil_clear_value;
		total_clear_val_count = color_slot_count + 1;
	}

	// initialize render pass begin info
	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = nullptr;
	rp_begin.renderPass = render_pass.render_pass.GetVkHandle();
	rp_begin.framebuffer = framebuffer.framebuffer.GetVkHandle();
	rp_begin.renderArea.offset.x = render_area.offset.X();
	rp_begin.renderArea.offset.y = render_area.offset.Y();
	rp_begin.renderArea.extent.width = render_area.extent.X();
	rp_begin.renderArea.extent.height = render_area.extent.Y();
	rp_begin.clearValueCount = static_cast<uint32_t>(total_clear_val_count);
	rp_begin.pClearValues = vk_clear_values;

	// begin render pass
	vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
}

// End current render pass.
void Context::EndRenderPass()
{
	vkCmdEndRenderPass(cmd);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//