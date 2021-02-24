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
                                   , tone_mapper(toolset)
                                   , fxaa(toolset)
{}

// Creates post-process (per-view) data.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @param format - format of the final image.
//    @return - a new postprocess::ViewData object or error if failed.
ut::Result<ViewData, ut::Error> Manager::CreateViewData(ut::uint32 width,
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
	for (ut::uint32 i = 0; i < ViewData::skSwapSlotCount; i++)
	{
		ut::Result<Target, ut::Error> swap_slot = tools.device.CreateTarget(info);
		if (!swap_slot)
		{
			throw ut::Error(swap_slot.MoveAlt());
		}
		swap_targets.Add(swap_slot.Move());
	}

	// render pass
	RenderTargetSlot color_slot(format, RenderTargetSlot::load_dont_care, RenderTargetSlot::store_save, false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	ut::Result<RenderPass, ut::Error> pass = tools.device.CreateRenderPass(ut::Move(color_slots));
	if (!pass)
	{
		return ut::MakeError(pass.MoveAlt());
	}

	// framebuffers
	ut::Array<Framebuffer> swap_framebuffers;
	for (ut::uint32 i = 0; i < ViewData::skSwapSlotCount; i++)
	{
		ut::Array< ut::Ref<Target> > color_targets;
		color_targets.Add(swap_targets[i]);
		ut::Result<Framebuffer, ut::Error> swap_framebuffer = tools.device.CreateFramebuffer(pass.Get(),
		                                                                                     ut::Move(color_targets));
		if (!swap_framebuffer)
		{
			throw ut::Error(swap_framebuffer.MoveAlt());
		}
		swap_framebuffers.Add(swap_framebuffer.Move());
	}

	// tone mapping
	ut::Result<ToneMapper::ViewData, ut::Error> tone_mapping_data = tone_mapper.CreateViewData(pass.Get(),
	                                                                                           width,
	                                                                                           height);
	if (!tone_mapping_data)
	{
		return ut::MakeError(tone_mapping_data.MoveAlt());
	}

	// fxaa
	ut::Result<Fxaa::ViewData, ut::Error> fxaa_data = fxaa.CreateViewData(pass.Get(),
	                                                                      width,
	                                                                      height);
	if (!fxaa_data)
	{
		return ut::MakeError(fxaa_data.MoveAlt());
	}

	// swap slots
	ut::Array<ViewData::SwapSlot> swap_slots;
	for (ut::uint32 i = 0; i < ViewData::skSwapSlotCount; i++)
	{
		swap_slots.Add(ViewData::SwapSlot{ut::Move(swap_targets[i]), ut::Move(swap_framebuffers[i])});
	}

	// success
	return ViewData(ut::Move(swap_slots),
	                pass.Move(),
	                tone_mapping_data.Move(),
	                fxaa_data.Move());
}

// Applies all post-process effects to the provided view.
//    @param context - reference to the rendering context.
//    @param data - reference to the postprocess::ViewData object containing
//                  postprocess-specific resources.
//    @param source - reference to the source image.
//    @return - optional reference to the final image.
ut::Optional<Image&> Manager::ApplyEffects(Context& context,
                                           ViewData& data,
                                           Image& source)
{
	// tone mapping
	ViewData::SwapSlot& srgb_slot = data.Swap();
	tone_mapper.Apply(context, data.tone_mapping, srgb_slot.framebuffer, data.pass, source);
	context.SetTargetState(srgb_slot.target, Target::Info::state_resource);

	// fxaa
	ViewData::SwapSlot& fxaa_slot = data.Swap();
	fxaa.Apply(context, data.fxaa, fxaa_slot.framebuffer, data.pass, srgb_slot.target.GetImage());
	context.SetTargetState(fxaa_slot.target, Target::Info::state_resource);

	return fxaa_slot.target.GetImage();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//