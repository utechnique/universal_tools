//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/units/ve_render_view.h"
#include "systems/render/engine/post_process/ve_post_process_parameters.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// All post-processing effect types.
enum class Effect
{
	tone_mapping,
	stencil_highlighting,
	fxaa
};

// Post process manager encapsulates post process techniques and applies
// screen-space effects.
class Manager
{
public:
	// Constructor.
	Manager(Toolset& toolset);

	// Creates post-process (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @param format - format of the final image.
	//    @return - a new postprocess::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                               ut::uint32 width,
	                                               ut::uint32 height,
	                                               pixel::Format format);

	// Applies desired post-process effects to the provided view.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the postprocess::ViewData object containing
	//                  postprocess-specific resources.
	//    @param source - reference to the source image.
	//    @param time_ms - total accumulated time in milliseconds.
	//    @param parameters - reference to the postprocess::Parameter object
	//                        containing parameters for all post-processing
	//                        effects.
	//    @return - optional reference to the final image.
	template<Effect ... effects>
	ut::Optional<Image&> ApplyEffects(Context& context,
	                                  ViewData& data,
	                                  ut::Optional<SwapSlot&> slot,
	                                  const Parameters& parameters,
	                                  double time_ms)
	{
		ut::Optional<SwapSlot&> final_slot = ApplyMultipleEffects<Effect,
		                                                          effects...>(context,
		                                                                      data,
		                                                                      slot,
		                                                                      parameters,
		                                                                      time_ms);
		if (final_slot)
		{
			context.SetTargetState(final_slot->color_target, Target::Info::state_resource);
			final_slot->busy = false;
			return final_slot->color_target.GetImage();
		}

		return slot->color_target.GetImage();
	}

	// Applies desired postprocess effect to the provided view.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the postprocess::ViewData object containing
	//                  postprocess-specific resources.
	//    @param source - reference to the source image.
	//    @param time_ms - total accumulated time in milliseconds.
	//    @param parameters - reference to the postprocess::Parameter object
	//                        containing parameters for all post-processing
	//                        effects.
	//    @return - optional reference to the next postprocess slot.
	template<Effect effect>
	ut::Optional<SwapSlot&> ApplyEffect(Context& context,
	                                    ViewData& data,
	                                    Image& source,
	                                    const Parameters& parameters,
	                                    double time_ms);

private:
	// Helper function to perform multiple postprocess effects, one by one.
	template<typename EffectType, EffectType current_effect, EffectType ... next_effects>
	ut::Optional<SwapSlot&> ApplyMultipleEffects(Context& context,
	                                             ViewData& data,
	                                             ut::Optional<SwapSlot&> slot,
	                                             const Parameters& parameters,
	                                             double time_ms)
	{
		ut::Optional<SwapSlot&> next_slot = ApplyEffect<current_effect>(context,
		                                                                data,
		                                                                slot->color_target.GetImage(),
		                                                                parameters,
		                                                                time_ms);
		if (next_slot)
		{
			context.SetTargetState(next_slot->color_target, Target::Info::state_resource);
			if (slot)
			{
				slot->busy = false;
			}
			slot = next_slot;
		}

		return ApplyMultipleEffects<EffectType,
		                            next_effects...>(context,
		                                             data,
		                                             slot,
		                                             parameters,
		                                             time_ms);
	}

	// Final postprocess effect.
	template<typename EffectType>
	ut::Optional<SwapSlot&> ApplyMultipleEffects(Context& context,
	                                             ViewData& data,
	                                             ut::Optional<SwapSlot&> slot,
	                                             const Parameters& parameters,
	                                             double time_ms)
	{
		return slot;
	}

	Toolset& tools;
	GaussianBlur gaussian_blur;
	ToneMapper tone_mapper;
	StencilHighlight stencil_highlight;
	Fxaa fxaa;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
