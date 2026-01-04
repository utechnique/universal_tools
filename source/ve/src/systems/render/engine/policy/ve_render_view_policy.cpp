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
                                                , hitmask(toolset)
{}

//----------------------------------------------------------------------------->
// Initializes provided view unit.
void Policy<View>::Initialize(View& view)
{
	ut::time::Counter timer;
	timer.Start();

	// set ssaa preset according to the configuration values
	if (tools.config.supersampling >= 16)
	{
		view.ssaa = postprocess::Ssaa::SampleCount::s16;
	}
	else if (tools.config.supersampling >= 9)
	{
		view.ssaa = postprocess::Ssaa::SampleCount::s9;
	}
	else if (tools.config.supersampling >= 4)
	{
		view.ssaa = postprocess::Ssaa::SampleCount::s4;
	}
	else
	{
		view.ssaa = postprocess::Ssaa::SampleCount::s1;
	}
	const ut::Vector<2, ut::uint32> supersampled_size =
		postprocess::Ssaa::CalculateSupersampledSize(ut::Vector<2, ut::uint32>(view.width, view.height),
		                                             view.ssaa);

	ut::Array<View::FrameData> frames;
	for (ut::uint32 i = 0; i < tools.config.frames_in_flight; i++)
	{
		// environment cubemap
		ut::Result<View::SceneBuffer, ut::Error> env_map = CreateSceneBuffer(tools.config.ibl_size,
		                                                                     tools.config.ibl_size,
		                                                                     true,
		                                                                     lighting_mgr.ibl.GetMipCount());
		if (!env_map)
		{
			throw ut::Error(env_map.MoveAlt());
		}

		// hdr scene targets
		ut::Result<View::SceneBuffer, ut::Error> scene_buffer = CreateSceneBuffer(supersampled_size.X(),
		                                                                          supersampled_size.Y(),
		                                                                          false);
		if (!scene_buffer)
		{
			throw ut::Error(scene_buffer.MoveAlt());
		}

		// image based lighting
		ut::Result<lighting::IBL::ViewData, ut::Error> ibl_data = lighting_mgr.ibl.CreateViewData();
		if (!ibl_data)
		{
			throw ut::Error(ibl_data.MoveAlt());
		}

		// hitmask
		ut::Result<HitMask::ViewData, ut::Error> hitmask_data = hitmask.CreateViewData(scene_buffer->depth_stencil,
		                                                                               view.width, view.height);
		if (!hitmask_data)
		{
			throw ut::Error(hitmask_data.MoveAlt());
		}

		// post-process
		ut::Result<postprocess::ViewData, ut::Error> post_process_data = post_process_mgr.CreateViewData(scene_buffer->depth_stencil,
		                                                                                                 view.width,
		                                                                                                 view.height,
		                                                                                                 view.ssaa,
		                                                                                                 tools.formats.ldr);
		if (!post_process_data)
		{
			throw ut::Error(post_process_data.MoveAlt());
		}

		// add frame
		View::FrameData frame_data{ env_map.Move(),
		                            scene_buffer.Move(),
		                            ibl_data.Move(),
		                            hitmask_data.Move(),
		                            post_process_data.Move(),
		                            ut::Optional<Image&>() };
		if (!frames.Add(ut::Move(frame_data)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	ut::log.Lock() << "Render engine initialized view #" << view.viewport_id
	               << " in "<< timer.GetTime<ut::time::Unit::second>() << "s." << ut::cret;

	View::GpuData gpu_data;
	gpu_data.frames = ut::Move(frames);
	view.data = tools.rc_mgr.AddResource(ut::Move(gpu_data));
}

//----------------------------------------------------------------------------->
// Renders all render units to all render views.
//    @param context - reference to the render context.
//    @param time_step_ms - time step in milliseconds.
void Policy<View>::RenderEnvironment(Context& context,
                                     System::Time time_step_ms)
{
	ut::Array< ut::Ref<View> >& views = selector.Get<View>();

	// extract light units
	Light::Sources lights{ selector.Get<DirectionalLight>(),
	                       selector.Get<PointLight>(),
	                       selector.Get<SpotLight>(),
	                       selector.Get<AmbientLight>() };

	// update lights
	lighting_mgr.UpdateLightUniforms(context, lights);

	// render final image for all view units
	const size_t view_count = views.Count();
	for (size_t i = 0; i < view_count; i++)
	{
		View& view = views[i];
		view.frame_time_ms = time_step_ms;
		view.total_time_ms += view.frame_time_ms;
		RenderView(context, views[i], lights);
	}
}

//----------------------------------------------------------------------------->
// Renders the provided view.
void Policy<View>::RenderView(Context& context, View& view, Light::Sources& lights)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	Policy<MeshInstance>& mesh_instance_policy = policies.Get<MeshInstance>();

	// get current frame
	View::FrameData& frame = view.data->frames[current_frame_id];

	// get view uniform buffer containing view projection for the main view
	Buffer& view_uniform_buffer = frame.scene.view_ub[0];

	// reset final image
	frame.final_img = ut::Optional<Image&>();

	// skip if view is inactive
	if (!view.is_active)
	{
		return;
	}

	// copy hitmask to the cpu
	if(frame.hitmask.submitted)
	{
		hitmask.Read(context, frame.hitmask, view.hitmask);
		frame.hitmask.submitted = false;
	}
	else
	{
		view.hitmask.Reset();
	}

	// render hitmask
	if (view.draw_hitmask)
	{
		hitmask.Draw(context,
		             frame.scene.depth_stencil,
		             frame.hitmask,
		             view_uniform_buffer,
		             mesh_instance_policy.batcher);
	}

	// image based lighting
	ut::Optional<Image&> ibl_cubemap;
	if (tools.config.ibl_enabled && view.light_pass_mode == View::LightPassMode::complete)
	{
		ut::Optional<Profiler::ScopeCounter> ibl_scope_counter =
			tools.profiler.CreateScopeCounter(Profiler::Stat::ibl,
			                                  context,
			                                  view.viewport_id);

		RenderIblCubemap(context, view, lights);

		ibl_cubemap = frame.ibl.filtered_cubemap.GetImage();
	}

	// render the scene to the final buffer
	Image& light_pass_img = RenderLightPass(context,
	                                        view.viewport_id,
	                                        frame.scene,
	                                        lights,
	                                        view.view_matrix,
	                                        view.proj_matrix,
	                                        view.camera_position,
	                                        view.light_pass_mode,
	                                        ibl_cubemap);

	const bool ssaa_enabled = view.ssaa != postprocess::Ssaa::SampleCount::s1;
	const postprocess::SwapManager::Stage stage = ssaa_enabled ?
	                                              postprocess::SwapManager::Stage::supersampled :
	                                              postprocess::SwapManager::Stage::resolved;
	frame.post_process.swap_mgr.SetStage(stage);

	// apply tone mapping
	ut::Optional<postprocess::SwapSlot&> color_buffer =
		post_process_mgr.ApplyEffect<postprocess::Effect::tone_mapping>(context,
		                                                                frame.post_process,
		                                                                light_pass_img,
		                                                                view.post_process,
		                                                                view.total_time_ms);
	UT_ASSERT(color_buffer);

	// here unlit objects can be rendered
	lighting_mgr.unlit.DrawUnlitGeometryLdr(context,
	                                        color_buffer->color_and_ds_framebuffer,
	                                        frame.scene.lighting.unlit,
	                                        frame.scene.view_ub[0],
	                                        mesh_instance_policy.batcher);

	// make color buffer accessible as a resource
	context.SetTargetState(color_buffer->color_target, Target::Info::State::resource);

	// postprocess
	ut::Optional<Profiler::ScopeCounter> postprocess_scope_counter =
		tools.profiler.CreateScopeCounter(Profiler::Stat::postprocess,
		                                  context,
		                                  view.viewport_id);
	frame.final_img = post_process_mgr.ApplyEffects
	                  <postprocess::Effect::stencil_highlighting,
	                   postprocess::Effect::fxaa,
	                   postprocess::Effect::ssaa_resolve,
	                   postprocess::Effect::gradient_dithering>(context,
	                                                            frame.post_process,
	                                                            color_buffer,
	                                                            view.post_process,
	                                                            view.total_time_ms);
}

//----------------------------------------------------------------------------->
// Creates hdr scene targets.
ut::Result<View::SceneBuffer, ut::Error> Policy<View>::CreateSceneBuffer(ut::uint32 width,
                                                                         ut::uint32 height,
                                                                         bool is_cube,
                                                                         ut::uint32 light_buffer_mip_count)
{
	// create view uniform buffer
	const ut::uint32 ub_count = is_cube ? 6 : 1;
	ut::Array<Buffer> view_ub;
	for (ut::uint32 i = 0; i < ub_count; i++)
	{
		Buffer::Info buffer_info;
		buffer_info.type = Buffer::Type::uniform;
		buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
		buffer_info.size = sizeof(View::Uniforms);
		ut::Result<Buffer, ut::Error> uniform_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!uniform_buffer)
		{
			return ut::MakeError(uniform_buffer.MoveAlt());
		}
		view_ub.Add(uniform_buffer.Move());
	}

	// depth stencil
	Target::Info info;
	info.type = is_cube ? Image::Type::cubic : Image::Type::planar;
	info.format = tools.formats.depth_stencil;
	info.usage = Target::Info::Usage::depth;
	info.mip_count = 1;
	info.width = width;
	info.height = height;
	info.depth = 1;
	ut::Result<Target, ut::Error> depth_stencil = tools.device.CreateTarget(info);
	if (!depth_stencil)
	{
		return ut::MakeError(depth_stencil.MoveAlt());
	}

	// lighting
	ut::Result<lighting::ViewData, ut::Error> lighting_data = lighting_mgr.CreateViewData(depth_stencil.Get(),
	                                                                                      width, height,
	                                                                                      is_cube,
	                                                                                      light_buffer_mip_count);
	if (!lighting_data)
	{
		return ut::MakeError(lighting_data.MoveAlt());
	}

	return View::SceneBuffer{ ut::Move(view_ub), depth_stencil.Move(), lighting_data.Move() };
}

//----------------------------------------------------------------------------->
// Renders environment.
Image& Policy<View>::RenderLightPass(Context& context,
                                     ui::Viewport::Id viewport_id,
                                     View::SceneBuffer& scene,
                                     Light::Sources& lights,
                                     const ut::Matrix<4>& view_matrix,
                                     const ut::Matrix<4>& proj_matrix,
                                     const ut::Vector<3>& view_position,
                                     View::LightPassMode light_pass_mode,
                                     ut::Optional<Image&> ibl_cubemap,
                                     Image::Cube::Face face,
                                     ut::Optional<Profiler::StatSubgroup> stat_subgroup)
{
	Policy<MeshInstance>& mesh_instance_policy = policies.Get<MeshInstance>();

	// update view uniform buffer
	UpdateViewUniforms(context, scene, view_matrix, proj_matrix, view_position, face);

	
	{ // bake deferred shading data
		ut::Optional<Profiler::ScopeCounter> gbuffer_bake_scope_counter =
			tools.profiler.CreateScopeCounter(Profiler::Stat::gbuffer_bake,
			                                  context,
			                                  viewport_id,
			                                  stat_subgroup);

		lighting_mgr.deferred_shading.BakeOpaqueGeometry(context,
		                                                 scene.depth_stencil,
		                                                 scene.lighting.deferred_shading,
		                                                 scene.view_ub[static_cast<ut::uint32>(face)],
		                                                 mesh_instance_policy.batcher,
		                                                 face);
	}

	// exit if the view mode is set to show one of the g-buffer targets
	if (light_pass_mode == View::LightPassMode::deferred_base_color)
	{
		context.SetTargetState(scene.lighting.deferred_shading.base_color,
		                       Target::Info::State::resource);
		return scene.lighting.deferred_shading.base_color.GetImage(); // exit
	}
	else if (light_pass_mode == View::LightPassMode::deferred_normal)
	{
		context.SetTargetState(scene.lighting.deferred_shading.normal,
		                       Target::Info::State::resource);
		return scene.lighting.deferred_shading.normal.GetImage(); // exit
	}

	{ // perform deferred shading
		ut::Optional<Profiler::ScopeCounter> def_shading_scope_counter =
			tools.profiler.CreateScopeCounter(Profiler::Stat::deferred_shading,
			                                  context,
			                                  viewport_id,
			                                  stat_subgroup);

		lighting_mgr.deferred_shading.Shade(context,
		                                    scene.lighting.deferred_shading,
		                                    scene.view_ub[static_cast<ut::uint32>(face)],
		                                    lights,
		                                    ibl_cubemap,
		                                    face);
	}

	// use forward renderer to draw all units that
	// can't be rendered in deferred pass
	{
		ut::Optional<Profiler::ScopeCounter> forward_render_scope_counter =
			tools.profiler.CreateScopeCounter(Profiler::Stat::forward_rendering,
			                                  context,
			                                  viewport_id,
			                                  stat_subgroup);

		lighting_mgr.forward_shading.Draw(context,
		                                  scene.lighting.forward_shading,
		                                  scene.view_ub[static_cast<ut::uint32>(face)],
		                                  view_position,
		                                  mesh_instance_policy.batcher,
		                                  lights,
		                                  ibl_cubemap,
		                                  face);
	}

	context.SetTargetState(scene.lighting.light_buffer, Target::Info::State::resource);
	return scene.lighting.light_buffer.GetImage();
}

// Renders IBL cubemap.
void Policy<View>::RenderIblCubemap(Context& context,
                                    View& view,
                                    Light::Sources& lights)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	View::FrameData& frame = view.data->frames[current_frame_id];
	const ut::uint32 faces_to_update = frame.ibl.initialized ? tools.config.ibl_frequency : 6;

	// ibl cubemap uses ibl from the previous frame to compute its own lighting,
	// note that it must be cleared for the first frame to prevent garbage artifacts
	if (!frame.ibl.initialized)
	{
		context.ClearTarget(frame.ibl.filtered_cubemap, ut::Color<4>(0));
	}
	context.SetTargetState(frame.ibl.filtered_cubemap, Target::Info::State::resource);

	// draw ibl faces
	for (ut::uint32 i = frame.ibl.face_id; i < frame.ibl.face_id + faces_to_update; i++)
	{
		RenderLightPass(context,
		                view.viewport_id,
		                frame.environment_map,
		                lights,
		                lighting_mgr.ibl.CreateFaceViewMatrix(static_cast<Image::Cube::Face>(i), view.camera_position),
		                lighting_mgr.ibl.CreateFaceProjectionMatrix(view.znear, view.zfar),
		                view.camera_position,
		                View::LightPassMode::complete,
		                frame.ibl.filtered_cubemap.GetImage(),
		                static_cast<Image::Cube::Face>(i),
		                Profiler::StatSubgroup::ibl);
	}

	// ibl source must have full set of mips
	context.GenerateMips(frame.environment_map.lighting.light_buffer, Target::Info::State::resource);

	// filter the source to retrieve final ibl cubemap
	for (ut::uint32 i = frame.ibl.face_id; i < frame.ibl.face_id + faces_to_update; i++)
	{
		lighting_mgr.ibl.FilterCubemap(context,
		                               frame.ibl,
		                               frame.environment_map.lighting.light_buffer.GetImage(),
		                               static_cast<Image::Cube::Face>(i));
	}
	context.SetTargetState(frame.ibl.filtered_cubemap, Target::Info::State::resource);

	// update face iterator
	frame.ibl.face_id += faces_to_update;
	if (frame.ibl.face_id >= 6)
	{
		frame.ibl.face_id = 0;
		frame.ibl.initialized = true;
	}
}

// Updates view uniform buffer.
void Policy<View>::UpdateViewUniforms(Context& context,
                                      View::SceneBuffer& scene,
                                      const ut::Matrix<4>& view_matrix,
                                      const ut::Matrix<4>& proj_matrix,
                                      const ut::Vector<3>& view_position,
                                      Image::Cube::Face face)
{
	// calculate view-projection matrix
	const ut::Matrix<4, 4> view_proj = view_matrix * proj_matrix;
	const ut::Optional< ut::Matrix<4, 4> > view_proj_inversed = view_proj.Invert();

	// update uniform buffers
	View::Uniforms view_uniforms;
	view_uniforms.view_proj = view_proj;
	if (view_proj_inversed)
	{
		view_uniforms.view_proj_inversed = view_proj_inversed.Get();
		view_uniforms.camera_position.X() = view_position.X();
		view_uniforms.camera_position.Y() = view_position.Y();
		view_uniforms.camera_position.Z() = view_position.Z();
	}
	tools.rc_mgr.UpdateBuffer(context,
	                          scene.view_ub[static_cast<ut::uint32>(face)],
	                          &view_uniforms);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//