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
                                   , color_only_pass(CreateColorOnlyRenderPass())
                                   , color_and_ds_pass(CreateColorAndDepthStencilRenderPass())
                                   , clear_color_and_ds_pass(CreateClearColorAndDepthStencilRenderPass())
                                   , gaussian_blur(toolset)
                                   , downsampling(toolset, color_only_pass)
                                   , tone_mapper(toolset, color_only_pass)
                                   , stencil_highlight(toolset,
                                                       gaussian_blur,
                                                       color_only_pass,
                                                       color_and_ds_pass,
                                                       clear_color_and_ds_pass)
                                   , dithering(toolset, color_only_pass)
                                   , fxaa(toolset, color_only_pass)
                                   , ssaa(toolset, downsampling)
                                   
{}

// Creates post-process (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param width - width of the view in pixels, without supersampling.
//    @param height - height of the view in pixels, without supersampling.
//    @param ssaa_samples - the number of ssaa samples per final pixel.
//    @param format - format of the final image.
//    @return - a new postprocess::ViewData object or error if failed.
ut::Result<ViewData, ut::Error> Manager::CreateViewData(Target& depth_stencil,
                                                        ut::uint32 width,
                                                        ut::uint32 height,
                                                        Ssaa::SampleCount ssaa_samples,
                                                        pixel::Format format)
{
	// color-only render pass
	RenderTargetSlot color_slot(format,
	                            RenderTargetSlot::LoadOperation::dont_care,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
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
	RenderTargetSlot ds_slot(depth_stencil_format,
	                         RenderTargetSlot::LoadOperation::extract,
	                         RenderTargetSlot::StoreOperation::save,
	                         false);
	ut::Result<RenderPass, ut::Error> color_and_ds_pass = tools.device.CreateRenderPass(ut::Move(color_slots), ds_slot);
	if (!color_and_ds_pass)
	{
		return ut::MakeError(color_and_ds_pass.MoveAlt());
	}

	// clear color and depth-stencil render pass
	RenderTargetSlot clean_color_slot(format,
	                                  RenderTargetSlot::LoadOperation::clear,
	                                  RenderTargetSlot::StoreOperation::save,
	                                  false);
	color_slots.Add(clean_color_slot);
	ut::Result<RenderPass, ut::Error> clear_color_and_ds_pass = tools.device.CreateRenderPass(ut::Move(color_slots), ds_slot);
	if (!clear_color_and_ds_pass)
	{
		return ut::MakeError(clear_color_and_ds_pass.MoveAlt());
	}

	// create swap slots for supersampled and resolved stages
	ut::Array<SwapSlots> swap_slots;
	for (ut::uint32 stage_iter = 0; stage_iter < SwapManager::skStageCount; stage_iter++)
	{
		const bool supersampled_stage =
			static_cast<SwapManager::Stage>(stage_iter) == SwapManager::Stage::supersampled;

		// skip supersampled stage if ssaa is not used
		if (supersampled_stage && ssaa_samples == Ssaa::SampleCount::s1)
		{
			swap_slots.Add(SwapSlots());
			continue;
		}

		// calculate slot width and height for the current stage
		ut::Vector<2, ut::uint32> slot_size = ut::Vector<2, ut::uint32>(width, height);
		if (supersampled_stage)
		{
			slot_size = Ssaa::CalculateSupersampledSize(slot_size, ssaa_samples);
		}

		// initialize slots
		SwapSlots stage_slots;
		for (ut::uint32 slot_iter = 0; slot_iter < SwapManager::skSlotCount; slot_iter++)
		{
			// slot render target
			Target::Info info;
			info.type = Image::Type::planar;
			info.format = format;
			info.mip_count = 1;
			info.width = slot_size.X();
			info.height = slot_size.Y();
			info.depth = 1;
			info.usage = Target::Info::Usage::color;
			ut::Result<Target, ut::Error> swap_slot_target = tools.device.CreateTarget(info);
			if (!swap_slot_target)
			{
				return ut::MakeError(swap_slot_target.MoveAlt());
			}

			// color-only framebuffer
			ut::Array<Framebuffer::Attachment> color_targets;
			color_targets.Add(swap_slot_target.Get());
			ut::Result<Framebuffer, ut::Error> color_only_framebuffer = tools.device.CreateFramebuffer(color_only_pass.Get(),
			                                                                                           ut::Move(color_targets));
			if (!color_only_framebuffer)
			{
				return ut::MakeError(color_only_framebuffer.MoveAlt());
			}

			// color and depth-stencil framebuffer
			color_targets.Add(swap_slot_target.Get());
			ut::Result<Framebuffer, ut::Error> color_and_ds_framebuffer = tools.device.CreateFramebuffer(color_and_ds_pass.Get(),
			                                                                                             ut::Move(color_targets),
			                                                                                             Framebuffer::Attachment(depth_stencil));
			if (!color_and_ds_framebuffer)
			{
				return ut::MakeError(color_and_ds_framebuffer.MoveAlt());
			}

			// add the new slot to the stage buffer
			stage_slots.Add(SwapSlot{ false,
			                          swap_slot_target.Move(),
			                          color_only_framebuffer.Move(),
			                          color_and_ds_framebuffer.Move() });
		}
		
		swap_slots.Add(ut::Move(stage_slots));
	}

	// tone mapping
	ut::Result<ToneMapper::ViewData, ut::Error> tone_mapping_data = tone_mapper.CreateViewData();
	if (!tone_mapping_data)
	{
		return ut::MakeError(tone_mapping_data.MoveAlt());
	}

	// stencil highlight
	ut::Result<StencilHighlight::ViewData, ut::Error> stencil_highlight_data = stencil_highlight.CreateViewData();
	if (!stencil_highlight_data)
	{
		return ut::MakeError(stencil_highlight_data.MoveAlt());
	}

	// dithering
	ut::Result<Dithering::ViewData, ut::Error> dithering_data = dithering.CreateViewData();
	if (!dithering_data)
	{
		return ut::MakeError(dithering_data.MoveAlt());
	}

	// fxaa
	ut::Result<Fxaa::ViewData, ut::Error> fxaa_data = fxaa.CreateViewData();
	if (!fxaa_data)
	{
		return ut::MakeError(fxaa_data.MoveAlt());
	}

	// ssaa
	ut::Result<Ssaa::ViewData, ut::Error> ssaa_data = ssaa.CreateViewData(width, height,
	                                                                      ssaa_samples);
	if (!ssaa_data)
	{
		return ut::MakeError(ssaa_data.MoveAlt());
	}

	// success
	return ViewData(ut::Move(swap_slots),
	                tone_mapping_data.Move(),
	                stencil_highlight_data.Move(),
	                dithering_data.Move(),
	                fxaa_data.Move(),
	                ssaa_data.Move());
}

// Applies tone mapping effect.
template<>
ut::Optional<SwapSlot&> Manager::ApplyEffect<Effect::tone_mapping>(Context& context,
                                                                   ViewData& data,
                                                                   Image& source,
                                                                   const Parameters& parameters,
                                                                   double time_ms)
{
	return tone_mapper.Apply(data.swap_mgr,
	                         context,
	                         data.tone_mapping,
	                         source,
	                         parameters.tone_mapping);
}

// Resolves SSAA buffer.
template<>
ut::Optional<SwapSlot&> Manager::ApplyEffect<Effect::ssaa_resolve>(Context& context,
                                                                   ViewData& data,
                                                                   Image& source,
                                                                   const Parameters& parameters,
                                                                   double time_ms)
{
	return ssaa.Resolve(data.swap_mgr,
	                    context,
	                    data.ssaa,
	                    source);
}

// Applies gradient dithering effect.
template<>
ut::Optional<SwapSlot&> Manager::ApplyEffect<Effect::gradient_dithering>(Context& context,
                                                                         ViewData& data,
                                                                         Image& source,
                                                                         const Parameters& parameters,
                                                                         double time_ms)
{
	return dithering.ApplyGradientDithering(data.swap_mgr,
	                                        context,
	                                        data.dithering,
	                                        source,
	                                        parameters.dithering,
	                                        static_cast<float>(time_ms));
}

// Applies stencil highlighting effect.
template<>
ut::Optional<SwapSlot&> Manager::ApplyEffect<Effect::stencil_highlighting>(Context& context,
                                                                           ViewData& data,
                                                                           Image& source,
                                                                           const Parameters& parameters,
                                                                           double time_ms)
{
	return stencil_highlight.Apply(data.swap_mgr,
	                               context,
	                               data.stencil_highlight,
	                               source,
	                               parameters.stencil_highlight,
	                               static_cast<float>(time_ms));
}

// Applies FXAA effect.
template<>
ut::Optional<SwapSlot&> Manager::ApplyEffect<Effect::fxaa>(Context& context,
                                                           ViewData& data,
                                                           Image& source,
                                                           const Parameters& parameters,
                                                           double time_ms)
{
	return fxaa.Apply(data.swap_mgr,
	                  context,
	                  data.fxaa,
	                  color_only_pass,
	                  source,
	                  parameters.fxaa);
}

// Creates a render pass with only one attached color slot.
RenderPass Manager::CreateColorOnlyRenderPass()
{
	RenderTargetSlot color_slot(tools.formats.ldr,
	                            RenderTargetSlot::LoadOperation::dont_care,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots)).MoveOrThrow();
}

// Creates a render pass with one color slot and one depth-stencil slot.
RenderPass Manager::CreateColorAndDepthStencilRenderPass()
{
	RenderTargetSlot color_slot(tools.formats.ldr,
	                            RenderTargetSlot::LoadOperation::dont_care,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	RenderTargetSlot ds_slot(tools.formats.depth_stencil,
	                            RenderTargetSlot::LoadOperation::extract,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), ds_slot).MoveOrThrow();
}

// Creates a render pass with one color slot and one depth-stencil slot.
// Color slot is cleared when the renderpass begins.
RenderPass Manager::CreateClearColorAndDepthStencilRenderPass()
{
	RenderTargetSlot clean_color_slot(tools.formats.ldr,
	                                  RenderTargetSlot::LoadOperation::clear,
		                              RenderTargetSlot::StoreOperation::save,
	                                  false);
	RenderTargetSlot ds_slot(tools.formats.depth_stencil,
	                         RenderTargetSlot::LoadOperation::extract,
	                         RenderTargetSlot::StoreOperation::save,
	                         false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(clean_color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), ds_slot).MoveOrThrow();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//