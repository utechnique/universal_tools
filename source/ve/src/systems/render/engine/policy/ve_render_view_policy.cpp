//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/policy/ve_render_view_policy.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Policy<View>::Policy(Toolset &toolset,
                     UnitSelector& unit_selector,
                     Policies& engine_policies) : tools(toolset)
                                                , selector(unit_selector)
                                                , policies(engine_policies)
                                                , lighting_mgr(toolset)
                                                , post_process_mgr(toolset)
{}

//----------------------------------------------------------------------------->
// Initializes provided view unit.
void Policy<View>::Initialize(View& view)
{
	ut::Array<View::FrameData> frames;
	for (ut::uint32 i = 0; i < tools.config.frames_in_flight; i++)
	{
		// create view uniform buffer
		Buffer::Info buffer_info;
		buffer_info.type = Buffer::uniform;
		buffer_info.usage = render::memory::gpu_read_cpu_write;
		buffer_info.size = sizeof(View::Uniforms);
		ut::Result<Buffer, ut::Error> view_ub = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!view_ub)
		{
			throw ut::Error(view_ub.MoveAlt());
		}

		// depth stencil
		Target::Info info;
		info.type = Image::type_2D;
		info.format = skDepthFormat;
		info.usage = Target::Info::usage_depth;
		info.mip_count = 1;
		info.width = view.width;
		info.height = view.height;
		info.depth = 1;
		ut::Result<Target, ut::Error> depth_stencil = tools.device.CreateTarget(info);
		if (!depth_stencil)
		{
			throw ut::Error(depth_stencil.MoveAlt());
		}

		// lighting
		ut::Result<lighting::ViewData, ut::Error> lighting_data = lighting_mgr.CreateViewData(depth_stencil.Get(),
		                                                                                      view.width,
		                                                                                      view.height);
		if (!lighting_data)
		{
			throw ut::Error(lighting_data.MoveAlt());
		}

		// post-process
		ut::Result<postprocess::ViewData, ut::Error> post_process_data = post_process_mgr.CreateViewData(view.width,
		                                                                                                 view.height,
		                                                                                                 view.format);
		if (!post_process_data)
		{
			throw ut::Error(post_process_data.MoveAlt());
		}

		// add frame
		View::FrameData frame_data(view_ub.Move(),
		                           depth_stencil.Move(),
		                           lighting_data.Move(),
		                           post_process_data.Move());
		if (!frames.Add(ut::Move(frame_data)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	View::GpuData gpu_data(ut::Move(frames));
	view.data = tools.rc_mgr.AddResource(ut::Move(gpu_data));
}

//----------------------------------------------------------------------------->
// Renders all render units to all render views.
void Policy<View>::RenderEnvironment(Context& context)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	Policy<Model>& model_policy = policies.Get<Model>();
	ut::Array< ut::Ref<View> >& views = selector.Get<View>();

	const size_t view_count = views.GetNum();
	for (size_t i = 0; i < view_count; i++)
	{
		View& view = views[i];

		// get current frame
		View::FrameData& frame = view.data->frames[current_frame_id];

		// reset final image
		frame.final_img = ut::Optional<Image&>();

		// skip if view is inactive
		if (!view.is_active)
		{
			continue;
		}

		// calculate view-projection matrix
		const ut::Matrix<4, 4> view_proj = view.view_matrix * view.proj_matrix;

		// update view uniform buffer
		View::Uniforms view_uniforms;
		view_uniforms.view_proj = view_proj;
		tools.rc_mgr.UpdateBuffer(context, frame.uniform_buffer, &view_uniforms);

		// bake deferred shading data
		lighting_mgr.deferred_shading.BakeModels(context,
		                                         frame.lighting.deferred_shading,
		                                         frame.uniform_buffer,
		                                         model_policy.batcher);
		context.SetTargetState(frame.lighting.deferred_shading.framebuffer, Target::Info::state_resource);


		if (view.mode == View::mode_diffuse)
		{
			frame.final_img = frame.lighting.deferred_shading.diffuse.GetImage();
			continue;
		}
		else if (view.mode == View::mode_normal)
		{
			frame.final_img = frame.lighting.deferred_shading.normal.GetImage();
			continue;
		}

		// postprocess
		frame.final_img = post_process_mgr.ApplyEffects(context,
		                                                frame.post_process,
		                                                frame.lighting.deferred_shading.diffuse.GetImage());
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//