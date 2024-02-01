//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_post_process_mgr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Constructor.
Manager::Manager(Toolset& toolset) : tools(toolset)
                                   , gaussian_blur(toolset)
                                   , tone_mapper(toolset)
                                   , stencil_highlight(toolset, gaussian_blur)
                                   , fxaa(toolset)
{}

// Creates post-process (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @param format - format of the final image.
//    @return - a new postprocess::ViewData object or error if failed.
ut::Result<ViewData, ut::Error> Manager::CreateViewData(Target& depth_stencil,
                                                        ut::uint32 width,
                                                        ut::uint32 height,
                                                        pixel::Format format)
{
	// inremediate buffers
	Target::Info info;
	info.type = Image::type_2D;
	info.format = format;
	info.mip_count = 1;
	info.width = width;
	info.height = height;
	info.depth = 1;
	info.usage = Target::Info::usage_color;
	ut::Array<Target> swap_targets;
	for (ut::uint32 i = 0; i < SwapManager::skSlotCount; i++)
	{
		ut::Result<Target, ut::Error> swap_slot = tools.device.CreateTarget(info);
		if (!swap_slot)
		{
			throw ut::Error(swap_slot.MoveAlt());
		}
		swap_targets.Add(swap_slot.Move());
	}

	// color-only render pass
	RenderTargetSlot color_slot(format, RenderTargetSlot::load_dont_care, RenderTargetSlot::store_save, false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	ut::Result<RenderPass, ut::Error> color_only_pass = tools.device.CreateRenderPass(ut::Move(color_slots));
	if (!color_only_pass)
	{
		return ut::MakeError(color_only_pass.MoveAlt());
	}

	// color and depth-stencil render pass
	color_slots.Add(color_slot);
	const pixel::Format depth_stencil_format = depth_stencil.GetInfo().format;
	RenderTargetSlot ds_slot(depth_stencil_format, RenderTargetSlot::load_extract, RenderTargetSlot::store_save, false);
	ut::Result<RenderPass, ut::Error> color_and_ds_pass = tools.device.CreateRenderPass(ut::Move(color_slots), ds_slot);
	if (!color_and_ds_pass)
	{
		return ut::MakeError(color_and_ds_pass.MoveAlt());
	}

	// clear color and depth-stencil render pass
	RenderTargetSlot clean_color_slot(format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	color_slots.Add(clean_color_slot);
	ut::Result<RenderPass, ut::Error> clear_color_and_ds_pass = tools.device.CreateRenderPass(ut::Move(color_slots), ds_slot);
	if (!clear_color_and_ds_pass)
	{
		return ut::MakeError(clear_color_and_ds_pass.MoveAlt());
	}

	// framebuffers
	ut::Array<Framebuffer> color_only_framebuffers;
	ut::Array<Framebuffer> color_and_ds_framebuffers;
	for (ut::uint32 i = 0; i < SwapManager::skSlotCount; i++)
	{
		ut::Array<Framebuffer::Attachment> color_targets;

		// color-only
		color_targets.Add(swap_targets[i]);
		ut::Result<Framebuffer, ut::Error> color_only_framebuffer = tools.device.CreateFramebuffer(color_only_pass.Get(),
		                                                                                           ut::Move(color_targets));
		if (!color_only_framebuffer)
		{
			throw ut::Error(color_only_framebuffer.MoveAlt());
		}
		color_only_framebuffers.Add(color_only_framebuffer.Move());

		// color and depth-stencil
		color_targets.Add(swap_targets[i]);
		ut::Result<Framebuffer, ut::Error> color_and_ds_framebuffer = tools.device.CreateFramebuffer(color_and_ds_pass.Get(),
		                                                                                             ut::Move(color_targets),
		                                                                                             Framebuffer::Attachment(depth_stencil));
		if (!color_and_ds_framebuffer)
		{
			throw ut::Error(color_and_ds_framebuffer.MoveAlt());
		}
		color_and_ds_framebuffers.Add(color_and_ds_framebuffer.Move());
	}

	// tone mapping
	ut::Result<ToneMapper::ViewData, ut::Error> tone_mapping_data = tone_mapper.CreateViewData(color_only_pass.Get(),
	                                                                                           width, height);
	if (!tone_mapping_data)
	{
		return ut::MakeError(tone_mapping_data.MoveAlt());
	}

	// stencil highlight
	ut::Result<StencilHighlight::ViewData, ut::Error> stencil_highlight_data = stencil_highlight.CreateViewData(color_only_pass.Get(),
	                                                                                                            color_and_ds_pass.Get(),
	                                                                                                            clear_color_and_ds_pass.Get(),
	                                                                                                            width, height);
	if (!stencil_highlight_data)
	{
		return ut::MakeError(stencil_highlight_data.MoveAlt());
	}

	// fxaa
	ut::Result<Fxaa::ViewData, ut::Error> fxaa_data = fxaa.CreateViewData(color_only_pass.Get(),
	                                                                      width,
	                                                                      height);
	if (!fxaa_data)
	{
		return ut::MakeError(fxaa_data.MoveAlt());
	}

	// swap slots
	ut::Array<SwapSlot> swap_slots;
	for (ut::uint32 i = 0; i < SwapManager::skSlotCount; i++)
	{
		swap_slots.Add(SwapSlot{ false,
		                         ut::Move(swap_targets[i]),
		                         ut::Move(color_only_framebuffers[i]),
		                         ut::Move(color_and_ds_framebuffers[i]) });
	}

	// success
	return ViewData(ut::Move(swap_slots),
	                color_only_pass.Move(),
	                color_and_ds_pass.Move(),
	                clear_color_and_ds_pass.Move(),
	                tone_mapping_data.Move(),
	                stencil_highlight_data.Move(),
	                fxaa_data.Move());
}

// Applies all post-process effects to the provided view.
//    @param context - reference to the rendering context.
//    @param data - reference to the postprocess::ViewData object containing
//                  postprocess-specific resources.
//    @param source - reference to the source image.
//    @param time_ms - total accumulated time in milliseconds.
//    @return - optional reference to the final image.
ut::Optional<Image&> Manager::ApplyEffects(Context& context,
                                           ViewData& data,
                                           Image& source,
                                           double time_ms)
{
	// tone mapping
	SwapSlot& srgb_slot = tone_mapper.Apply(data.swap_mgr,
	                                        context,
	                                        data.tone_mapping,
	                                        data.color_only_pass,
	                                        source);
	context.SetTargetState(srgb_slot.color_target, Target::Info::state_resource);

	// stencil highlight
	SwapSlot& highlight_slot = stencil_highlight.Apply(data.swap_mgr,
	                                                   context,
	                                                   data.stencil_highlight,
	                                                   data.color_only_pass,
	                                                   data.color_and_ds_pass,
	                                                   data.clear_color_and_ds_pass,
	                                                   srgb_slot.color_target.GetImage(),
	                                                   static_cast<float>(time_ms));
	context.SetTargetState(highlight_slot.color_target, Target::Info::state_resource);
	srgb_slot.busy = false;

	// fxaa
	SwapSlot& fxaa_slot = fxaa.Apply(data.swap_mgr,
	                                 context,
	                                 data.fxaa,
	                                 data.color_only_pass,
	                                 highlight_slot.color_target.GetImage());
	context.SetTargetState(fxaa_slot.color_target, Target::Info::state_resource);
	highlight_slot.busy = false;
	fxaa_slot.busy = false;

	return fxaa_slot.color_target.GetImage();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//