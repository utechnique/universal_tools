//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_view.h"
#include "systems/render/engine/lighting/ve_forward_shading.h"
#include "systems/render/engine/policy/ve_render_model_policy.h"
#include "systems/render/engine/ve_render_stencil_ref.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
ForwardShading::ForwardShading(Toolset& toolset,
                               ut::uint32 ibl_mip_count) : tools(toolset)
                                                         , light_shader(CreateModelLightPassShader())
                                                         , ibl_shader(CreateModelIblShader(ibl_mip_count))
{
	ConnectDescriptors();
}

// Creates deferred shading (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param light_buffer - reference to the light buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @param is_cube - 'true' to create as a cubemap.
//    @return - a new ForwardShading::ViewData object or error if failed.
ut::Result<ForwardShading::ViewData, ut::Error> ForwardShading::CreateViewData(Target& depth_stencil,
                                                                               Target& light_buffer,
                                                                               ut::uint32 width,
                                                                               ut::uint32 height,
                                                                               bool is_cube)
{
	const ut::uint32 face_count = is_cube ? 6 : 1;

	// light pass
	ut::Result<RenderPass, ut::Error> lightpass = CreateLightPass(depth_stencil.GetInfo().format,
	                                                              light_buffer.GetInfo().format);
	if (!lightpass)
	{
		return ut::MakeError(lightpass.MoveAlt());
	}

	// light pass framebuffer
	ut::Array<Framebuffer> light_framebuffer;
	for (ut::uint32 face_id = 0; face_id < face_count; face_id++)
	{
		ut::Array<Framebuffer::Attachment> color_targets;
		color_targets.Add(Framebuffer::Attachment(light_buffer, face_id));
		ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(lightpass.Get(),
		                                                                                ut::Move(color_targets),
		                                                                                Framebuffer::Attachment(depth_stencil, face_id));
		if (!framebuffer)
		{
			return ut::MakeError(framebuffer.MoveAlt());
		}
		light_framebuffer.Add(framebuffer.Move());
	}

	// light pass pipeline states
	ut::Array<PipelineState> lightpass_pipeline;
	constexpr size_t light_permutation_count = LightPass::ModelRendering::PipelineGrid::size;
	for (ut::uint32 i = 0; i < light_permutation_count; i++)
	{
		const size_t vertex_format = LightPass::ModelRendering::PipelineGrid::GetCoordinate<LightPass::ModelRendering::vertex_format_column>(i);
		const size_t alpha_test = LightPass::ModelRendering::PipelineGrid::GetCoordinate<LightPass::ModelRendering::alpha_test_column>(i);
		const size_t alpha_mode = LightPass::ModelRendering::PipelineGrid::GetCoordinate<LightPass::ModelRendering::alpha_mode_column>(i);
		const size_t ibl_preset = LightPass::ModelRendering::PipelineGrid::GetCoordinate<LightPass::ModelRendering::ibl_column>(i);
		const size_t source_type = LightPass::ModelRendering::PipelineGrid::GetCoordinate<LightPass::ModelRendering::light_type_column>(i);
		const size_t cull_mode = LightPass::ModelRendering::PipelineGrid::GetCoordinate<LightPass::ModelRendering::cull_mode_column>(i);

		ut::Result<PipelineState, ut::Error> pipeline = CreateModelLightPassPipeline(lightpass.Get(),
		                                                                             width, height,
		                                                                             static_cast<Mesh::VertexFormat>(vertex_format),
		                                                                             static_cast<LightPass::ModelRendering::AlphaTest>(alpha_test),
		                                                                             static_cast<LightPass::ModelRendering::AlphaMode>(alpha_mode),
		                                                                             static_cast<LightPass::ModelRendering::IblPreset>(ibl_preset),
		                                                                             static_cast<Light::SourceType>(source_type),
		                                                                             static_cast<LightPass::ModelRendering::CullMode>(cull_mode));
		if (!pipeline)
		{
			return ut::MakeError(pipeline.MoveAlt());
		}

		if (!lightpass_pipeline.Add(pipeline.Move()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// ibl pass pipeline states
	ut::Array<PipelineState> iblpass_pipeline;
	constexpr size_t ibl_permutation_count = IblPass::ModelRendering::PipelineGrid::size;
	for (ut::uint32 i = 0; i < ibl_permutation_count; i++)
	{
		const size_t vertex_format = IblPass::ModelRendering::PipelineGrid::GetCoordinate<IblPass::ModelRendering::vertex_format_column>(i);
		const size_t alpha_test = IblPass::ModelRendering::PipelineGrid::GetCoordinate<IblPass::ModelRendering::alpha_test_column>(i);
		const size_t alpha_mode = IblPass::ModelRendering::PipelineGrid::GetCoordinate<IblPass::ModelRendering::alpha_mode_column>(i);
		const size_t cull_mode = IblPass::ModelRendering::PipelineGrid::GetCoordinate<IblPass::ModelRendering::cull_mode_column>(i);

		ut::Result<PipelineState, ut::Error> pipeline = CreateModelIblPassPipeline(lightpass.Get(),
		                                                                           width, height,
		                                                                           static_cast<Mesh::VertexFormat>(vertex_format),
		                                                                           static_cast<IblPass::ModelRendering::AlphaTest>(alpha_test),
		                                                                           static_cast<IblPass::ModelRendering::AlphaMode>(alpha_mode),
		                                                                           static_cast<IblPass::ModelRendering::CullMode>(cull_mode));
		if (!pipeline)
		{
			return ut::MakeError(pipeline.MoveAlt());
		}

		if (!iblpass_pipeline.Add(pipeline.Move()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// secondary command buffers
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	const ut::uint32 secondary_buffer_count = thread_count * (is_cube ? 6 : 1);
	ut::Array<CmdBuffer> lightpass_cmd_buffers;
	for (ut::uint32 i = 0; i < secondary_buffer_count; i++)
	{
		CmdBuffer::Info cmd_buffer_info;
		cmd_buffer_info.usage = CmdBuffer::usage_dynamic | CmdBuffer::usage_inside_render_pass;
		cmd_buffer_info.level = CmdBuffer::level_secondary;

		ut::Result<CmdBuffer, ut::Error> cmd_buffer = tools.device.CreateCmdBuffer(cmd_buffer_info);
		if (!cmd_buffer)
		{
			throw ut::Error(cmd_buffer.MoveAlt());
		}

		lightpass_cmd_buffers.Add(cmd_buffer.Move());
	}

	ForwardShading::ViewData data =
	{
		lightpass.Move(),
		ut::Move(light_framebuffer),
		ut::Move(lightpass_pipeline),
		ut::Move(iblpass_pipeline),
		ut::Move(lightpass_cmd_buffers),
	};

	return ut::Move(data);
}

// Renders scene directly to the light buffer.
void ForwardShading::DrawTransparentGeometry(Context& context,
                                             ForwardShading::ViewData& data,
                                             Buffer& view_uniform_buffer,
                                             const ut::Vector<3>& view_position,
                                             ModelBatcher& batcher,
                                             Light::Sources& lights,
                                             ut::Optional<Image&> ibl_cubemap,
                                             Image::Cube::Face cubeface)
{
	// get the number of available threads
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	UT_ASSERT(thread_count == lightpass_model_desc_set.Count());

	// perform z-sorting
	SortTransparentDrawCalls(view_position, batcher.draw_calls);
	const ut::uint32 sorted_dc_count = static_cast<ut::uint32>(z_sorted_dc_indices.Count());

	// initialize secondary buffers for a parallel work
	const ut::uint32 dc_per_thread = sorted_dc_count / thread_count; // drawcalls per thread
	secondary_buffer_cache.Reset();
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		secondary_buffer_cache.Add(data.lightpass_cmd[cubeface * thread_count + i]);
		tools.device.ResetCmdBuffer(secondary_buffer_cache.GetLast().Get());
	}

	// begin a render pass
	Framebuffer& framebuffer = data.light_framebuffer[cubeface];
	const Framebuffer::Info& fb_info = framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.lightpass, framebuffer, render_area, ut::Color<4>(0), 1.0f, 0, true);

	// render all models in different threads
	ut::uint32 offset = 0; // offset from the first drawcall in the batcher
	for (ut::uint32 thread_id = 0; thread_id < thread_count; thread_id++)
	{
		ut::uint32 count = dc_per_thread + (thread_id == 0 ? (sorted_dc_count % thread_count) : 0);
		auto record_commands = [&, thread_id, offset, count](Context& deferred_context)
		{
			RenderTransparentModelJob(deferred_context,
			                          data,
			                          lights,
			                          view_uniform_buffer,
			                          batcher,
			                          ibl_cubemap,
			                          thread_id,
			                          offset,
			                          count);
		};

		auto job = [&, thread_id, record_commands]()
		{
			tools.device.Record(secondary_buffer_cache[thread_id],
			                    record_commands,
			                    data.lightpass,
			                    framebuffer);
		};

		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void()> >(job));

		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// finish render pass
	context.ExecuteSecondaryBuffers(secondary_buffer_cache);
	context.EndRenderPass();
}

// Renders specified range of models.
void ForwardShading::RenderTransparentModelJob(Context& context,
                                               ForwardShading::ViewData& data,
                                               Light::Sources& lights,
                                               Buffer& view_uniform_buffer,
                                               ModelBatcher& batcher,
                                               ut::Optional<Image&> ibl_cubemap,
                                               ut::uint32 thread_id,
                                               ut::uint32 offset,
                                               ut::uint32 count)
{
	if (count == 0)
	{
		return;
	}

	const LightPass::ModelRendering::IblPreset ibl_preset = ibl_cubemap ? LightPass::ModelRendering::ibl_on :
	                                                                      LightPass::ModelRendering::ibl_off;
	const ut::uint32 last_element = offset + count - 1;
	for (ut::uint32 i = offset; i <= last_element; i++)
	{
		// skip opaque models
		const ut::uint32 sorted_id = z_sorted_dc_indices[i].first;
		Model::DrawCall& dc = batcher.draw_calls[sorted_id];
		Mesh::Subset& subset = dc.model.mesh->subsets[dc.subset_id];
		Material& material = subset.material;
		if (material.alpha != Material::alpha_transparent)
		{
			continue;
		}

		constexpr ut::uint32 front_culling = 0;
		constexpr ut::uint32 back_culling = 1;
		constexpr ut::uint32 cull_mode_count = 2;
		ut::uint32 cull_mode = material.double_sided ? front_culling : back_culling;
		for (; cull_mode < cull_mode_count; cull_mode++)
		{
			bool first_pass = true;
			// ibl reflections pass
			if (ibl_cubemap)
			{
				const IblPass::ModelRendering::CullMode ibl_cull_mode = cull_mode == front_culling ?
				                                                        IblPass::ModelRendering::cull_front :
				                                                        IblPass::ModelRendering::cull_back;
				const IblPass::ModelRendering::AlphaMode ibl_alpha_mode = first_pass ?
				                                                          IblPass::ModelRendering::alpha_blend :
				                                                          IblPass::ModelRendering::alpha_add;
				RenderTransparentModelIbl(context,
				                          data,
				                          ibl_cubemap.Get(),
				                          view_uniform_buffer,
				                          batcher,
				                          ibl_cull_mode,
				                          ibl_alpha_mode,
				                          sorted_id,
				                          thread_id);
				first_pass = false;
			}

			// direct light pass
			const LightPass::ModelRendering::CullMode lightpass_cull_mode = cull_mode == front_culling ?
			                                                                LightPass::ModelRendering::cull_front :
			                                                                LightPass::ModelRendering::cull_back;
			const LightPass::ModelRendering::AlphaMode lightpass_alpha_mode = first_pass ?
			                                                                  LightPass::ModelRendering::alpha_blend :
			                                                                  LightPass::ModelRendering::alpha_add;
			RenderTransparentModelLights(context,
			                             data,
			                             lights,
			                             view_uniform_buffer,
			                             batcher,
			                             ibl_preset,
			                             lightpass_cull_mode,
			                             lightpass_alpha_mode,
			                             sorted_id,
			                             thread_id);
		}
	}
}

// Applies direct lighting to the specified model.
void ForwardShading::RenderTransparentModelLights(Context& context,
                                                  ForwardShading::ViewData& data,
                                                  Light::Sources& lights,
                                                  Buffer& view_uniform_buffer,
                                                  ModelBatcher& batcher,
                                                  LightPass::ModelRendering::IblPreset ibl_preset,
                                                  LightPass::ModelRendering::CullMode cull_mode,
                                                  LightPass::ModelRendering::AlphaMode alpha_mode,
                                                  ut::uint32 drawcall_id,
                                                  ut::uint32 thread_id)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	const ut::uint32 batch_size = batcher.GetBatchSize();
	ut::Array<Model::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	Buffer& instance_buffer = batcher.instance_buffer;

	// extract material
	Model::DrawCall& dc = batcher.draw_calls[drawcall_id];
	Mesh& mesh = dc.model.mesh.Get();
	Mesh::Subset& subset = mesh.subsets[dc.subset_id];
	Material& material = subset.material;
	const LightPass::ModelRendering::AlphaTest alpha_test = material.alpha == Material::alpha_masked ?
	                                                        LightPass::ModelRendering::alpha_test_on :
	                                                        LightPass::ModelRendering::alpha_test_off;

	// accumulate light from all sources
	const size_t ambient_count = lights.ambient.Count();
	const size_t directional_count = lights.directional.Count();
	const size_t point_count = lights.point.Count();
	const size_t spot_count = lights.spot.Count();
	const size_t light_count = ambient_count + directional_count + point_count + spot_count;
	for (size_t light_id = 0; light_id < light_count; light_id++)
	{
		// figure out source type
		const Light::SourceType light_type = light_id < ambient_count ? Light::source_ambient :
		                                     light_id < (ambient_count + directional_count) ? Light::source_directional :
		                                     light_id < (ambient_count + directional_count + point_count) ?
		                                     Light::source_point : Light::source_spot;

		// extract light data
		ut::Optional<Buffer&> light_ub;
		if (light_type == Light::source_ambient)
		{
			AmbientLight& light = lights.ambient[light_id];
			AmbientLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::source_directional)
		{
			DirectionalLight& light = lights.directional[light_id - ambient_count];
			DirectionalLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::source_point)
		{
			PointLight& light = lights.point[light_id - ambient_count - directional_count];
			PointLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::source_spot)
		{
			SpotLight& light = lights.spot[light_id - ambient_count - directional_count - point_count];
			SpotLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else
		{
			UT_ASSERT(false);
		}

		// blending mode is used only for the first pass
		if (light_id > 0)
		{
			alpha_mode = LightPass::ModelRendering::alpha_add;
		}

		// calculate batch id
		const ut::uint32 batch_id = drawcall_id / batch_size;
		Model::Batch& batch = batches[batch_id];

		// bind pipeline state
		const Mesh::VertexFormat vertex_format = mesh.vertex_format;
		const size_t pipeline_state_id = LightPass::ModelRendering::PipelineGrid::GetId(vertex_format,
		                                                                                ibl_preset,
		                                                                                light_type,
		                                                                                alpha_test,
		                                                                                alpha_mode,
		                                                                                cull_mode);

		// bind uniforms
		LightPass::ModelRendering::Descriptors& desc_set = lightpass_model_desc_set[thread_id];
		desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
		desc_set.light_ub.BindUniformBuffer(light_ub.Get());
		desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
		desc_set.transform_ub.BindUniformBuffer(batch.transform);
		desc_set.material_ub.BindUniformBuffer(batch.material);
		desc_set.diffuse.BindImage(material.diffuse.Get());
		desc_set.normal.BindImage(material.normal.Get());
		desc_set.material.BindImage(material.material.Get());

		// bind index buffer and draw
		DrawMesh(context,
		         mesh,
		         data.lightpass_pipeline[pipeline_state_id],
		         desc_set,
		         instance_buffer,
		         subset.index_count,
		         subset.index_offset,
		         drawcall_id % batch_size);
	}
}

// Applies direct lighting to the specified model.
void ForwardShading::RenderTransparentModelIbl(Context& context,
                                               ForwardShading::ViewData& data,
                                               Image& ibl_cubemap,
                                               Buffer& view_uniform_buffer,
                                               ModelBatcher& batcher,
                                               IblPass::ModelRendering::CullMode cull_mode,
                                               IblPass::ModelRendering::AlphaMode alpha_mode,
                                               ut::uint32 drawcall_id,
                                               ut::uint32 thread_id)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	const ut::uint32 batch_size = batcher.GetBatchSize();
	ut::Array<Model::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	Buffer& instance_buffer = batcher.instance_buffer;

	// extract material
	Model::DrawCall& dc = batcher.draw_calls[drawcall_id];
	Mesh& mesh = dc.model.mesh.Get();
	Mesh::Subset& subset = mesh.subsets[dc.subset_id];
	Material& material = subset.material;
	const IblPass::ModelRendering::AlphaTest alpha_test = material.alpha == Material::alpha_masked ?
	                                                      IblPass::ModelRendering::alpha_test_on :
	                                                      IblPass::ModelRendering::alpha_test_off;

	// calculate batch id
	const ut::uint32 batch_id = drawcall_id / batch_size;
	Model::Batch& batch = batches[batch_id];

	// get pipeline state
	const Mesh::VertexFormat vertex_format = mesh.vertex_format;
	const size_t pipeline_state_id = IblPass::ModelRendering::PipelineGrid::GetId(vertex_format,
	                                                                              alpha_test,
	                                                                              alpha_mode,
	                                                                              cull_mode);

	// bind uniforms
	IblPass::ModelRendering::Descriptors& desc_set = iblpass_model_desc_set[thread_id];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
	desc_set.transform_ub.BindUniformBuffer(batch.transform);
	desc_set.material_ub.BindUniformBuffer(batch.material);
	desc_set.diffuse.BindImage(material.diffuse.Get());
	desc_set.normal.BindImage(material.normal.Get());
	desc_set.material.BindImage(material.material.Get());
	desc_set.ibl_cubemap.BindImage(ibl_cubemap);

	// draw
	DrawMesh(context,
	         mesh,
	         data.iblpass_pipeline[pipeline_state_id],
	         desc_set,
	         instance_buffer,
	         subset.index_count,
	         subset.index_offset,
	         drawcall_id % batch_size);
}

// Renders provided mesh.
void ForwardShading::DrawMesh(Context& context,
                              Mesh& mesh,
                              PipelineState& pipeline,
                              DescriptorSet& desc_set,
                              Buffer& instance_buffer,
                              ut::uint32 index_count,
                              ut::uint32 index_offset,
                              ut::uint32 instance_offset)
{
	context.BindPipelineState(pipeline);
	context.BindDescriptorSet(desc_set);
	context.BindVertexAndInstanceBuffer(mesh.vertex_buffer, 0, instance_buffer, 0);
	if (mesh.index_buffer)
	{
		context.BindIndexBuffer(mesh.index_buffer.Get(), 0, mesh.index_type);
		context.DrawIndexedInstanced(index_count,
		                             1,
		                             index_offset,
		                             0,
		                             instance_offset);
	}
	else
	{
		context.DrawInstanced(index_count, // index count here means vertex count
		                      1,
		                      index_offset, // index offset here means vertex offset
		                      instance_offset);
	}
}

// Creates a shader for the lighting pass.
ut::Array<BoundShader> ForwardShading::CreateModelLightPassShader()
{
	ut::Array<BoundShader> shaders;
	constexpr size_t light_permutation_count = LightPass::ModelRendering::ShaderGrid::size;
	for (size_t i = 0; i < light_permutation_count; i++)
	{
		const size_t vertex_format = LightPass::ModelRendering::ShaderGrid::GetCoordinate<LightPass::ModelRendering::vertex_format_column>(i);
		const size_t ibl_preset = LightPass::ModelRendering::ShaderGrid::GetCoordinate<LightPass::ModelRendering::ibl_column>(i);
		const size_t source_type = LightPass::ModelRendering::ShaderGrid::GetCoordinate<LightPass::ModelRendering::light_type_column>(i);
		const size_t alpha_test = LightPass::ModelRendering::ShaderGrid::GetCoordinate<LightPass::ModelRendering::alpha_test_column>(i);

		Shader::Macros macros;
		Shader::MacroDefinition macro;
		ut::String shader_name_suffix;

		// enable instancing
		macro.name = "INSTANCING";
		macro.value = "1";
		macros.Add(macro);

		// light pass
		macro.name = "LIGHT_PASS";
		macro.value = "1";
		macros.Add(macro);

		// batch size
		macro.name = "BATCH_SIZE";
		macro.value = ut::Print(ModelBatcher::CalculateBatchSize(tools.device));
		macros.Add(macro);

		// vertex traits
		macros += Mesh::GenerateVertexMacros(static_cast<Mesh::VertexFormat>(vertex_format), true);
		shader_name_suffix += ut::String("_vf") + ut::Print(vertex_format);

		// ibl preset
		const bool ibl_enabled = ibl_preset == LightPass::ModelRendering::ibl_on;
		shader_name_suffix += ibl_enabled ? "_ibl" : "_noibl";
		macro.name = "IBL";
		macro.value = ibl_enabled ? "1" : "0";
		macros.Add(ut::Move(macro));

		// light type
		const char* light_type_str;
		switch (source_type)
		{
		case Light::source_directional:
			light_type_str = "DIRECTIONAL_LIGHT";
			shader_name_suffix += "_directional";
			break;
		case Light::source_point:
			light_type_str = "POINT_LIGHT";
			shader_name_suffix += "_point";
			break;
		case Light::source_spot:
			light_type_str = "SPOT_LIGHT";
			shader_name_suffix += "_spot";
			break;
		case Light::source_ambient:
			light_type_str = "AMBIENT_LIGHT";
			shader_name_suffix += "_ambient";
			break;
		default: throw ut::Error(ut::error::not_implemented);
		}
		macro.name = light_type_str;
		macro.value = "1";
		macros.Add(ut::Move(macro));

		// alpha test
		const bool alpha_test_enabled = alpha_test == LightPass::ModelRendering::alpha_test_on;
		macro.name = "ALPHA_TEST";
		macro.value = alpha_test_enabled ? "1" : "0";
		macros.Add(macro);
		shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

		ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex,
		                                                            ut::String("forward_model_lp_vs") + shader_name_suffix,
		                                                            "VS",
		                                                            "model.hlsl",
		                                                            macros);

		ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
		                                                            ut::String("forward_model_lp_ps") + shader_name_suffix,
		                                                            "PS",
		                                                            "model.hlsl",
		                                                            macros);

		const bool result = shaders.Add(BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow()));
		if (!result)
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return shaders;
}

// Creates a shader for the ibl pass.
ut::Array<BoundShader> ForwardShading::CreateModelIblShader(ut::uint32 ibl_mip_count)
{
	ut::Array<BoundShader> shaders;
	constexpr size_t ibl_permutation_count = IblPass::ModelRendering::ShaderGrid::size;
	for (size_t i = 0; i < ibl_permutation_count; i++)
	{
		const size_t vertex_format = IblPass::ModelRendering::ShaderGrid::GetCoordinate<IblPass::ModelRendering::vertex_format_column>(i);
		const size_t alpha_test = IblPass::ModelRendering::ShaderGrid::GetCoordinate<IblPass::ModelRendering::alpha_test_column>(i);

		Shader::Macros macros;
		Shader::MacroDefinition macro;
		ut::String shader_name_suffix;

		// enable instancing
		macro.name = "INSTANCING";
		macro.value = "1";
		macros.Add(macro);

		// ibl pass
		macro.name = "IBL_PASS";
		macro.value = "1";
		macros.Add(macro);
		macro.name = "IBL";
		macro.value = "1";
		macros.Add(macro);

		// batch size
		macro.name = "BATCH_SIZE";
		macro.value = ut::Print(ModelBatcher::CalculateBatchSize(tools.device));
		macros.Add(macro);

		// mip count
		macro.name = "IBL_MIP_COUNT";
		macro.value = ut::Print(ibl_mip_count);
		macros.Add(ut::Move(macro));

		// vertex traits
		macros += Mesh::GenerateVertexMacros(static_cast<Mesh::VertexFormat>(vertex_format), true);
		shader_name_suffix += ut::String("_vf") + ut::Print(vertex_format);

		// alpha test
		const bool alpha_test_enabled = alpha_test == IblPass::ModelRendering::alpha_test_on;
		macro.name = "ALPHA_TEST";
		macro.value = alpha_test_enabled ? "1" : "0";
		macros.Add(macro);
		shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

		ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex,
		                                                            ut::String("forward_model_ibl_vs") + shader_name_suffix,
		                                                            "VS",
		                                                            "model.hlsl",
		                                                            macros);

		ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
		                                                            ut::String("forward_model_ibl_ps") + shader_name_suffix,
		                                                            "PS",
		                                                            "model.hlsl",
		                                                            macros);

		const bool result = shaders.Add(BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow()));
		if (!result)
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return shaders;
}

// Creates a render pass for the shading techniques.
ut::Result<RenderPass, ut::Error> ForwardShading::CreateLightPass(pixel::Format depth_stencil_format,
                                                                  pixel::Format light_buffer_format)
{
	RenderTargetSlot depth_slot(depth_stencil_format, RenderTargetSlot::load_extract, RenderTargetSlot::store_dont_care, false);
	RenderTargetSlot color_slot(light_buffer_format, RenderTargetSlot::load_extract, RenderTargetSlot::store_save, false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot);
}

// Creates a pipeline state to apply lighting.
ut::Result<PipelineState, ut::Error> ForwardShading::CreateModelLightPassPipeline(RenderPass& lightpass,
                                                                                  ut::uint32 width,
                                                                                  ut::uint32 height,
                                                                                  Mesh::VertexFormat vertex_format,
                                                                                  LightPass::ModelRendering::AlphaTest alpha_test,
                                                                                  LightPass::ModelRendering::AlphaMode alpha_mode,
                                                                                  LightPass::ModelRendering::IblPreset ibl_preset,
                                                                                  Light::SourceType source_type,
                                                                                  LightPass::ModelRendering::CullMode cull_mode)
{
	PipelineState::Info info;
	const size_t shader_id = LightPass::ModelRendering::ShaderGrid::GetId(vertex_format,
	                                                                      ibl_preset,
	                                                                      source_type,
	                                                                      alpha_test);
	UT_ASSERT(light_shader[shader_id].stages[Shader::vertex]);
	UT_ASSERT(light_shader[shader_id].stages[Shader::pixel]);

	info.stages[Shader::vertex] = light_shader[shader_id].stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = light_shader[shader_id].stages[Shader::pixel].Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format, true);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::replace;
	info.depth_stencil_state.back.compare_mask = 0xffffffff;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xffffffff;
	info.depth_stencil_state.stencil_reference = 0x0;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = cull_mode == LightPass::ModelRendering::cull_back ?
	                                                  RasterizationState::back_culling :
	                                                  RasterizationState::no_culling;
	if (alpha_mode == LightPass::ModelRendering::alpha_blend)
	{
		Blending blending(true,
		                  Blending::src_alpha,
		                  Blending::inverted_src_alpha,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == LightPass::ModelRendering::alpha_add)
	{
		Blending blending(true,
		                  Blending::src_alpha,
		                  Blending::one,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	
	return tools.device.CreatePipelineState(ut::Move(info), lightpass);
}

// Creates a pipeline state to apply ibl reflections.
ut::Result<PipelineState, ut::Error> ForwardShading::CreateModelIblPassPipeline(RenderPass& lightpass,
                                                                                ut::uint32 width,
                                                                                ut::uint32 height,
                                                                                Mesh::VertexFormat vertex_format,
                                                                                IblPass::ModelRendering::AlphaTest alpha_test,
                                                                                IblPass::ModelRendering::AlphaMode alpha_mode,
                                                                                IblPass::ModelRendering::CullMode cull_mode)
{
	PipelineState::Info info;
	const size_t shader_id = IblPass::ModelRendering::ShaderGrid::GetId(vertex_format, alpha_test);
	UT_ASSERT(ibl_shader[shader_id].stages[Shader::vertex]);
	UT_ASSERT(ibl_shader[shader_id].stages[Shader::pixel]);

	info.stages[Shader::vertex] = ibl_shader[shader_id].stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = ibl_shader[shader_id].stages[Shader::pixel].Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format, true);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = false;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = cull_mode == IblPass::ModelRendering::cull_back ?
	                                                  RasterizationState::back_culling :
	                                                  RasterizationState::no_culling;
	if (alpha_mode == IblPass::ModelRendering::alpha_blend)
	{
		Blending blending(true,
		                  Blending::one,
		                  Blending::inverted_src_alpha,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == IblPass::ModelRendering::alpha_add)
	{
		Blending blending(true,
		                  Blending::one,
		                  Blending::one,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	
	return tools.device.CreatePipelineState(ut::Move(info), lightpass);
}

// Connects all descriptor sets to the corresponding shaders.
void ForwardShading::ConnectDescriptors()
{
	UT_ASSERT(light_shader.Count() != 0);
	UT_ASSERT(ibl_shader.Count() != 0);
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	lightpass_model_desc_set.Resize(thread_count);
	iblpass_model_desc_set.Resize(thread_count);
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		lightpass_model_desc_set[i].Connect(light_shader.GetFirst());
		iblpass_model_desc_set[i].Connect(ibl_shader.GetFirst());
	}
}

// Performes Z-sotring for transparent objects and stores the result in
// @z_sorted_dc_indices array.
void ForwardShading::SortTransparentDrawCalls(const ut::Vector<3>& view_position,
                                              ut::Array<Model::DrawCall>& draw_list)
{
	z_sorted_dc_indices.Reset();
	const ut::uint32 dc_count = static_cast<ut::uint32>(draw_list.Count());
	for (ut::uint32 dc_index = 0; dc_index < dc_count; dc_index++)
	{
		// skip opaque draw calls
		const Model::DrawCall& dc = draw_list[dc_index];
		const Mesh::Subset& subset = dc.model.mesh->subsets[dc.subset_id];
		const Material& material = subset.material;
		if (material.alpha != Material::alpha_transparent)
		{
			continue;
		}

		// calculate distance
		const ut::Vector<3> direction = dc.model.world_transform.translation - view_position;
		const float distance = direction.Length();
		ut::Pair<ut::uint32, float> element_to_sort(dc_index, distance);

		// insert the view using the 'bubble' principle to retain sorting
		bool inserted = false;
		const size_t sorted_dc_count = z_sorted_dc_indices.Count();
		for (size_t sorted_dc_index = 0; sorted_dc_index < sorted_dc_count; sorted_dc_index++)
		{
			const ut::Pair<ut::uint32, float>& sorted_id = z_sorted_dc_indices[sorted_dc_index];
			if (distance >= sorted_id.second)
			{
				z_sorted_dc_indices.Insert(sorted_dc_index, ut::Move(element_to_sort));
				inserted = true;
				break;
			}
		}

		if (!inserted)
		{
			z_sorted_dc_indices.Add(ut::Move(element_to_sort));
		}
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//