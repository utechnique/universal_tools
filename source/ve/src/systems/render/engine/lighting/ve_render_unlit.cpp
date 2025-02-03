//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/lighting/ve_render_unlit.h"
#include "systems/render/engine/ve_render_stencil_ref.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
UnlitRenderer::UnlitRenderer(Toolset& toolset) : tools(toolset)
                                               , mesh_inst_shader(CreateMeshInstShaders())
                                               , pass(CreatePass())
                                               , mesh_inst_pipeline(CreateMeshInstPipelinePermutations())
{
	ConnectDescriptors();
}

// Creates unlit shading (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param ldr_buffer - reference to the low dynamic range buffer.
	//    @return - a new UnlitRenderer::ViewData object or error if failed.
ut::Result<UnlitRenderer::ViewData, ut::Error> UnlitRenderer::CreateViewData(Target& depth_stencil)
{
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());

	// secondary command buffers
	ut::Array<CmdBuffer> cmd_buffers;
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		CmdBuffer::Info cmd_buffer_info;
		cmd_buffer_info.usage = CmdBuffer::Usage::dynamic_inside_render_pass;
		cmd_buffer_info.level = CmdBuffer::Level::secondary;

		ut::Result<CmdBuffer, ut::Error> cmd_buffer = tools.device.CreateCmdBuffer(cmd_buffer_info);
		if (!cmd_buffer)
		{
			throw ut::Error(cmd_buffer.MoveAlt());
		}

		cmd_buffers.Add(cmd_buffer.Move());
	}

	UnlitRenderer::ViewData data =
	{
		ut::Move(cmd_buffers),
	};

	return ut::Move(data);
}

// Renders unlit units to a low dynamic range buffer.
void UnlitRenderer::DrawUnlitGeometryLdr(Context& context,
                                         Framebuffer& ldr_framebuffer,
                                         UnlitRenderer::ViewData& data,
                                         Buffer& view_uniform_buffer,
                                         Batcher& batcher)
{
	// get the number of available threads
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	UT_ASSERT(thread_count == mesh_inst_desc_set.Count());

	// get the number of drawcalls
	ut::Array<MeshInstance::DrawCall>& draw_list = batcher.draw_calls;
	const ut::uint32 dc_count = static_cast<ut::uint32>(draw_list.Count());

	// initialize secondary buffers for a parallel work
	secondary_buffer_cache.Reset();
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		secondary_buffer_cache.Add(data.cmd[i]);
		tools.device.ResetCmdBuffer(secondary_buffer_cache.GetLast().Get());
	}

	// begin a render pass
	const Framebuffer::Info& fb_info = ldr_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(pass, ldr_framebuffer, render_area, ut::Color<4>(0), 1.0f, 0, true);

	// parallelize drawcalls
	const ut::uint32 dc_per_thread = dc_count / thread_count; // drawcalls per thread
	ut::uint32 offset = 0; // offset from the first drawcall in the batcher
	for (ut::uint32 thread_id = 0; thread_id < thread_count; thread_id++)
	{
		ut::uint32 count = dc_per_thread + (thread_id == 0 ? (dc_count % thread_count) : 0);

		auto record_commands = [&, thread_id, offset, count](Context& deferred_context)
		{
			RenderUnlitMeshInstancesJob(deferred_context,
			                            data,
			                            view_uniform_buffer,
			                            batcher,
			                            thread_id,
			                            offset,
			                            count);
		};

		auto job = [&, thread_id, record_commands]()
		{
			tools.device.Record(secondary_buffer_cache[thread_id],
			                    record_commands,
			                    pass,
			                    ldr_framebuffer);
		};

		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void()> >(job));

		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// finish render pass
	context.ExecuteSecondaryBuffers(secondary_buffer_cache);
	context.EndRenderPass();
}

// Helper function to draw a mesh subset.
inline void PerformMeshInstDrawCall(Context& context,
                                    Buffer* index_buffer,
                                    ut::uint32 index_offset,
                                    ut::uint32 index_count,
                                    ut::uint32 instance_count,
                                    ut::uint32 call_id,
                                    ut::uint32 batch_size)
{
	if (instance_count == 0)
	{
		return;
	}

	const ut::uint32 first_instance_id = (call_id + 1 - instance_count) % batch_size;
	if (index_buffer == nullptr)
	{
		context.DrawInstanced(index_count,
		                      instance_count,
		                      index_offset,
		                      first_instance_id);
	}
	else
	{
		context.DrawIndexedInstanced(index_count,
		                      instance_count,
		                      index_offset,
		                      0,
		                      first_instance_id);
	}
}

// Renders specified range of mesh instances.
void UnlitRenderer::RenderUnlitMeshInstancesJob(Context& context,
                                                UnlitRenderer::ViewData& data,
                                                Buffer& view_uniform_buffer,
                                                Batcher& batcher,
                                                ut::uint32 thread_id,
                                                ut::uint32 offset,
                                                ut::uint32 count)
{
	if (count == 0)
	{
		return;
	}

	// get batcher data
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	ut::Array<MeshInstance::DrawCall>& draw_list = batcher.draw_calls;
	Buffer& instance_buffer = batcher.instance_buffer;
	ut::Array<MeshInstance::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	const ut::uint32 batch_size = batcher.GetBatchSize();

	// set common uniforms
	MeshInstRendering::Descriptors& desc_set = mesh_inst_desc_set[thread_id];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);

	// variables tracking if something changes between iterations
	Mesh::VertexFormat prev_vertex_format = Mesh::VertexFormat::count;
	Mesh::PolygonMode prev_polygon_mode = Mesh::PolygonMode::count;
	MeshInstRendering::AlphaMode prev_alpha_mode = MeshInstRendering::AlphaMode::count;
	MeshInstRendering::CullMode prev_cull_mode = MeshInstRendering::CullMode::count;
	MeshInstRendering::StencilMode prev_stencil_mode = MeshInstRendering::StencilMode::count;
	Map* prev_diffuse_ptr = nullptr;
	Buffer* prev_vertex_buffer = nullptr;
	Buffer* prev_index_buffer = nullptr;
	ut::uint32 prev_index_offset = 0;
	ut::uint32 prev_index_count = 0;
	ut::uint32 prev_batch_id = static_cast<ut::uint32>(batches.Count());

	// number of elements to draw at once
	ut::uint32 instance_count = 0;

	// iterate primitive groups and try to merge them in the
	// least possible amount of draw calls
	const ut::uint32 last_element = offset + count - 1;
	for (ut::uint32 i = offset; i <= last_element; i++)
	{
		// extract material
		MeshInstance::DrawCall& dc = draw_list[i];
		Mesh& mesh = dc.instance.mesh.Get();
		Mesh::Subset& subset = mesh.subsets[dc.subset_id];
		Material& material = subset.material;

		// skip lit materials
		if (!material.unlit)
		{
			PerformMeshInstDrawCall(context, prev_index_buffer,
			                        prev_index_offset, prev_index_count,
			                        instance_count, i - 1, batch_size);
			instance_count = 0;
			continue;
		}

		// calculate batch id
		const ut::uint32 batch_id = i / batch_size;
		MeshInstance::Batch& batch = batches[batch_id];

		// material maps
		Map* diffuse_ptr = &material.diffuse.Get();

		// buffers
		Buffer* vertex_buffer = &mesh.vertex_buffer;
		Buffer* index_buffer = mesh.index_buffer ? &mesh.index_buffer.Get() : nullptr;

		// index count
		const ut::uint32 index_offset = subset.index_offset;
		const ut::uint32 index_count = subset.index_count;

		// check pipeline state
		const Mesh::VertexFormat vertex_format = mesh.vertex_format;
		const Mesh::PolygonMode polygon_mode = mesh.polygon_mode;
		MeshInstRendering::AlphaMode alpha_mode = MeshInstRendering::AlphaMode::opaque;
		switch (material.alpha)
		{
		case Material::Alpha::masked: alpha_mode = MeshInstRendering::AlphaMode::alpha_test; break;
		case Material::Alpha::transparent: alpha_mode = MeshInstRendering::AlphaMode::blend; break;
		case Material::Alpha::opaque: alpha_mode = MeshInstRendering::AlphaMode::opaque; break;
		}
		const MeshInstRendering::CullMode cull_mode = material.double_sided ?
		                                              MeshInstRendering::CullMode::none :
		                                              MeshInstRendering::CullMode::back;
		const MeshInstRendering::StencilMode stencil_mode = dc.instance.highlighted ?
		                                                    MeshInstRendering::StencilMode::highlighted :
		                                                    MeshInstRendering::StencilMode::none;
		bool pipeline_changed = prev_vertex_format != vertex_format ||
		                        prev_polygon_mode != polygon_mode ||
		                        prev_cull_mode != cull_mode ||
		                        prev_alpha_mode != alpha_mode ||
		                        prev_stencil_mode != stencil_mode;

		// check if at least one shader resource has changed
		const bool shader_rc_changed = batch_id != prev_batch_id ||
		                               diffuse_ptr != prev_diffuse_ptr;

		// check buffers
		const bool vertex_buffer_changed = prev_vertex_buffer != vertex_buffer;
		const bool index_buffer_changed = prev_index_buffer != index_buffer;
		
		// check index count
		const bool indices_changed = prev_index_offset != index_offset ||
		                             prev_index_count != index_count;

		// check if at least one factor has changed since the previous iteration
		const bool state_changed = pipeline_changed ||
		                           shader_rc_changed ||
		                           vertex_buffer_changed ||
		                           index_buffer_changed ||
		                           indices_changed;

		// draw previous instance group
		if (state_changed && (i != offset))
		{
			PerformMeshInstDrawCall(context, prev_index_buffer,
			                        prev_index_offset, prev_index_count,
			                        instance_count, i - 1, batch_size);
			instance_count = 0;
		}
		instance_count++;

		// set pipeline state
		if (pipeline_changed)
		{
			const size_t pipeline_state_id = MeshInstRendering::PipelineGrid::GetId(static_cast<size_t>(vertex_format),
			                                                                        static_cast<size_t>(alpha_mode),
			                                                                        static_cast<size_t>(cull_mode),
			                                                                        static_cast<size_t>(stencil_mode),
			                                                                        static_cast<size_t>(polygon_mode));
			context.BindPipelineState(mesh_inst_pipeline[pipeline_state_id]);

			prev_vertex_format = vertex_format;
			prev_polygon_mode = polygon_mode;
			prev_alpha_mode = alpha_mode;
			prev_cull_mode = cull_mode;
			prev_stencil_mode = stencil_mode;
		}

		// bind descriptors
		if (shader_rc_changed)
		{
			desc_set.transform_ub.BindUniformBuffer(batch.transform);
			desc_set.material_ub.BindUniformBuffer(batch.material);
			desc_set.diffuse.BindImage(material.diffuse.Get());
			context.BindDescriptorSet(desc_set);

			prev_batch_id = batch_id;
			prev_diffuse_ptr = diffuse_ptr;
		}

		// bind vertex buffer
		if (vertex_buffer_changed)
		{
			context.BindVertexAndInstanceBuffer(*vertex_buffer, 0, instance_buffer, 0);
			prev_vertex_buffer = vertex_buffer;
		}
		
		// bind index buffer
		if (index_buffer_changed)
		{
			if (index_buffer != nullptr)
			{
				context.BindIndexBuffer(*index_buffer, 0, mesh.index_type);
			}
			prev_index_buffer = index_buffer;
		}

		// update indices count and offset
		if (indices_changed)
		{
			prev_index_offset = index_offset;
			prev_index_count = index_count;
		}

		// draw last element
		if (i == last_element)
		{
			PerformMeshInstDrawCall(context, index_buffer,
			                        index_offset, index_count,
			                        instance_count, i, batch_size);
		}
	}
}

// Creates all shader permutations for a mesh instance.
ut::Array<BoundShader> UnlitRenderer::CreateMeshInstShaders()
{
	ut::Array<BoundShader> shaders;
	constexpr size_t light_permutation_count = MeshInstRendering::ShaderGrid::size;
	for (size_t i = 0; i < light_permutation_count; i++)
	{
		const size_t vertex_format = MeshInstRendering::ShaderGrid::GetCoordinate<MeshInstRendering::vertex_format_column>(i);
		const size_t alpha_mode = MeshInstRendering::ShaderGrid::GetCoordinate<MeshInstRendering::alpha_mode_column>(i);

		Shader::Macros macros;
		Shader::MacroDefinition macro;
		ut::String shader_name_suffix;

		// enable instancing
		macro.name = "INSTANCING";
		macro.value = "1";
		macros.Add(macro);

		// unlit pass
		macro.name = "UNLIT_PASS";
		macro.value = "1";
		macros.Add(macro);

		// batch size
		macro.name = "BATCH_SIZE";
		macro.value = ut::Print(Batcher::CalculateBatchSize(tools.device));
		macros.Add(macro);

		// vertex traits
		macros += Mesh::GenerateVertexMacros(static_cast<Mesh::VertexFormat>(vertex_format), Mesh::Instancing::on);
		shader_name_suffix += ut::String("_vf") + ut::Print(vertex_format);

		// alpha mode
		const bool alpha_test_enabled = alpha_mode == static_cast<size_t>(MeshInstRendering::AlphaMode::alpha_test);
		macro.name = "ALPHA_TEST";
		macro.value = alpha_test_enabled ? "1" : "0";
		macros.Add(macro);
		if (alpha_test_enabled)
		{
			shader_name_suffix += "_at";
		}
		else if (alpha_mode == static_cast<size_t>(MeshInstRendering::MeshInstRendering::AlphaMode::blend))
		{
			shader_name_suffix += "_alpha_blend";
		}

		ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::Stage::vertex,
		                                                            ut::String("unlit_mesh_vs") + shader_name_suffix,
		                                                            "VS",
		                                                            "mesh.hlsl",
		                                                            macros);

		ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel,
		                                                            ut::String("unlit_mesh_ps") + shader_name_suffix,
		                                                            "PS",
		                                                            "mesh.hlsl",
		                                                            macros);

		const bool result = shaders.Add(BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow()));
		if (!result)
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return shaders;
}

// Creates unlit render pass.
RenderPass UnlitRenderer::CreatePass()
{
	RenderTargetSlot depth_slot(tools.formats.depth_stencil,
	                            RenderTargetSlot::LoadOperation::extract,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	RenderTargetSlot color_slot(tools.formats.ldr,
	                            RenderTargetSlot::LoadOperation::extract,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot).MoveOrThrow();
}

// Creates a pipeline state for a mesh instance.
ut::Result<PipelineState, ut::Error> UnlitRenderer::CreateMeshInstPipeline(Mesh::VertexFormat vertex_format,
                                                                           Mesh::PolygonMode polygon_mode,
                                                                           MeshInstRendering::AlphaMode alpha_mode,
                                                                           MeshInstRendering::CullMode cull_mode,
                                                                           MeshInstRendering::StencilMode stencil_mode)
{
	PipelineState::Info info;
	const size_t shader_id = MeshInstRendering::ShaderGrid::GetId(static_cast<size_t>(vertex_format),
	                                                              static_cast<size_t>(alpha_mode));
	UT_ASSERT(mesh_inst_shader[shader_id].GetShader(Shader::Stage::vertex));
	UT_ASSERT(mesh_inst_shader[shader_id].GetShader(Shader::Stage::pixel));

	const ut::uint32 stencil_mask = stencil_mode == MeshInstRendering::StencilMode::highlighted ?
	                                                static_cast<ut::uint32>(StencilReference::highlight) : 0x0;

	info.SetShader(Shader::Stage::vertex,
	               mesh_inst_shader[shader_id].GetShader(Shader::Stage::vertex));
	info.SetShader(Shader::Stage::pixel,
	               mesh_inst_shader[shader_id].GetShader(Shader::Stage::pixel));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format,
	                                                polygon_mode,
	                                                Mesh::Instancing::on);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = true;
	info.depth_stencil_state.depth_compare_op = compare::Operation::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.compare_mask = 0xffffffff;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xffffffff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = Mesh::GetRasterizerPolygonMode(polygon_mode);
	info.rasterization_state.cull_mode = cull_mode == MeshInstRendering::CullMode::back ?
	                                                  RasterizationState::CullMode::back :
	                                                  RasterizationState::CullMode::off;
	if (alpha_mode == MeshInstRendering::AlphaMode::blend)
	{
		Blending blending(true,
		                  Blending::Factor::src_alpha,
		                  Blending::Factor::inverted_src_alpha,
		                  Blending::Operation::add,
		                  Blending::Factor::one,
		                  Blending::Factor::one,
		                  Blending::Operation::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else
	{
		info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	}
	
	return tools.device.CreatePipelineState(ut::Move(info), pass);
}

// Creates all possible lighting pipeline state permutations for a mesh instance.
ut::Array<PipelineState> UnlitRenderer::CreateMeshInstPipelinePermutations()
{
	ut::Array<PipelineState> pipeline_states;

	constexpr ut::uint32 mesh_inst_pipeline_count = static_cast<ut::uint32>(MeshInstRendering::PipelineGrid::size);
	for (ut::uint32 i = 0; i < mesh_inst_pipeline_count; i++)
	{
		const size_t vertex_format = MeshInstRendering::PipelineGrid::GetCoordinate<MeshInstRendering::vertex_format_column>(i);
		const size_t alpha_mode = MeshInstRendering::PipelineGrid::GetCoordinate<MeshInstRendering::alpha_mode_column>(i);
		const size_t cull_mode = MeshInstRendering::PipelineGrid::GetCoordinate<MeshInstRendering::cull_mode_column>(i);
		const size_t stencil_mode = MeshInstRendering::PipelineGrid::GetCoordinate<MeshInstRendering::stencil_mode_column>(i);
		const size_t polygon_mode = MeshInstRendering::PipelineGrid::GetCoordinate<MeshInstRendering::polygon_mode_column>(i);

		ut::Result<PipelineState, ut::Error> pipeline = CreateMeshInstPipeline(static_cast<Mesh::VertexFormat>(vertex_format),
		                                                                       static_cast<Mesh::PolygonMode>(polygon_mode),
		                                                                       static_cast<MeshInstRendering::AlphaMode>(alpha_mode),
		                                                                       static_cast<MeshInstRendering::CullMode>(cull_mode),
		                                                                       static_cast<MeshInstRendering::StencilMode>(stencil_mode));
		if (!pipeline)
		{
			throw ut::Error(pipeline.MoveAlt());
		}

		if (!pipeline_states.Add(pipeline.Move()))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return pipeline_states;
}

// Connects all descriptor sets to the corresponding shaders.
void UnlitRenderer::ConnectDescriptors()
{
	UT_ASSERT(mesh_inst_shader.Count() != 0);
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	mesh_inst_desc_set.Resize(thread_count);
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		mesh_inst_desc_set[i].Connect(mesh_inst_shader.GetFirst());
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//