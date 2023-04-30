//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_hitmask.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
HitMask::HitMask(Toolset& toolset) : tools(toolset)
                                   , model_shader(CreateModelShader())
{
	ConnectDescriptors();
}

// Creates hitmask (per-view) data.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
ut::Result<HitMask::ViewData, ut::Error> HitMask::CreateViewData(Target& depth_stencil,
                                                                 ut::uint32 width,
                                                                 ut::uint32 height)
{
	const pixel::Format depth_stencil_format = depth_stencil.GetInfo().format;

	Target::Info info;
	info.type = Image::type_2D;
	info.format = skHitMaskFormat;
	info.usage = Target::Info::usage_color;
	info.has_staging_cpu_read_buffer = true;
	info.mip_count = 1;
	info.width = width;
	info.height = height;
	info.depth = 1;

	// render target
	ut::Result<Target, ut::Error> target = tools.device.CreateTarget(info);
	if (!target)
	{
		return ut::MakeError(target.MoveAlt());
	}

	// render pass
	ut::Result<RenderPass, ut::Error> render_pass = CreateRenderPass(depth_stencil_format);
	if (!render_pass)
	{
		return ut::MakeError(render_pass.MoveAlt());
	}

	// framebuffer
	ut::Array<Framebuffer::Attachment> color_targets;
	color_targets.Add(Framebuffer::Attachment(target.Get()));
	ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(render_pass.Get(),
	                                                                                ut::Move(color_targets),
	                                                                                Framebuffer::Attachment(depth_stencil));
	if (!framebuffer)
	{
		return ut::MakeError(framebuffer.MoveAlt());
	}

	// pipeline
	constexpr ut::uint32 model_pipeline_count = static_cast<ut::uint32>(ModelRendering::PipelineGrid::size);
	ut::Array<PipelineState> model_pipeline;
	for (ut::uint32 i = 0; i < model_pipeline_count; i++)
	{
		const size_t vertex_format = ModelRendering::PipelineGrid::GetCoordinate<ModelRendering::vertex_format_column>(i);
		const size_t alpha_mode = ModelRendering::PipelineGrid::GetCoordinate<ModelRendering::alpha_mode_column>(i);
		const size_t cull_mode = ModelRendering::PipelineGrid::GetCoordinate<ModelRendering::cull_mode_column>(i);

		ut::Result<PipelineState, ut::Error> pipeline = CreateModelPipeline(render_pass.Get(),
		                                                                    width, height,
		                                                                    static_cast<Mesh::VertexFormat>(vertex_format),
		                                                                    static_cast<ModelRendering::AlphaMode>(alpha_mode),
		                                                                    static_cast<ModelRendering::CullMode>(cull_mode));
		if (!pipeline)
		{
			return ut::MakeError(pipeline.MoveAlt());
		}

		if (!model_pipeline.Add(pipeline.Move()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	HitMask::ViewData data =
	{
		target.Move(),
		render_pass.Move(),
		framebuffer.Move(),
		ut::Move(model_pipeline),
		false,
	};

	return ut::Move(data);
}

// Renders the scnene to the provided hitmask.
void HitMask::Draw(Context& context,
                   Target& depth_stencil,
                   HitMask::ViewData& data,
                   Buffer& view_uniform_buffer,
                   ModelBatcher& batcher)
{
	context.SetTargetState(data.target, Target::Info::state_target);

	// begin a render pass
	Framebuffer& framebuffer = data.framebuffer;
	const Framebuffer::Info& fb_info = framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.pass, framebuffer, render_area, ut::Color<4>(1), 1.0f, 0, false);

	// render models
	DrawModelsJob(context,
	              data,
	              view_uniform_buffer,
	              batcher,
	              0,
	              0,
	              static_cast<ut::uint32>(batcher.draw_calls.Count()));

	// finish render pass
	context.EndRenderPass();

	// copy to the staging buffer
	context.SetTargetState(data.target, Target::Info::state_transfer_src);
	context.CopyImageToStagingCpuReadBuffer(data.target.GetImage());

	// mark this frame as submitted to gpu
	data.submitted = true;
}

// Copies gpu data to the provided cpu buffer.
void HitMask::Read(Context& context,
                   HitMask::ViewData& gpu_data,
                   ut::Array<Entity::Id>& cpu_buffer)
{
	Image& hitmask_img = gpu_data.target.GetImage();
	const Image::Info& hitmask_info = hitmask_img.GetInfo();

	ut::Result<Image::MappedResource, ut::Error> mapped_hitmask_rc = context.MapImage(hitmask_img, ut::access_read);
	if (!mapped_hitmask_rc)
	{
		throw ut::Error(ut::error::fail, "Failed to map the hitmask image.");
	}

	const ut::uint32 pixel_size = pixel::GetSize(skHitMaskFormat);
	ut::byte* hitmask_data = static_cast<ut::byte*>(mapped_hitmask_rc->data);
	cpu_buffer.Resize(hitmask_info.width * hitmask_info.height);
	for (ut::uint32 y = 0; y < hitmask_info.height; y++)
	{
		for (ut::uint32 x = 0; x < hitmask_info.width; x++)
		{
			const ut::uint32 id = y * hitmask_info.width + x;
			const ut::Color<4, ut::byte>* hitmask_texel = reinterpret_cast<const ut::Color<4, ut::byte>*>(hitmask_data + x * pixel_size);
			cpu_buffer[id] = DecodeEntityId(*hitmask_texel);
		}
		hitmask_data += mapped_hitmask_rc->row_pitch;
	}

	context.UnmapImage(hitmask_img, ut::access_read);
}

// Encodes the provided entity identifier into the hitmask compatible value.
Model::EntityIdBuffer::Type HitMask::EncodeEntityId(Entity::Id entity_id)
{
	// masking and shifting each 8-bit component from the 32-bit unsigned integer
	const ut::byte r = (entity_id >> 24) & 0xFF;
	const ut::byte g = (entity_id >> 16) & 0xFF;
	const ut::byte b = (entity_id >> 8) & 0xFF;
	const ut::byte a = entity_id & 0xFF;

	// converting each 8-bit component to a float in the range [0, 1]
	return Model::EntityIdBuffer::Type(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

// Decodes the provided hitmask value into the entity identifier.
Entity::Id HitMask::DecodeEntityId(const ut::Vector<4, ut::byte>& hitmask_value)
{
	return (static_cast<ut::uint32>(hitmask_value.R()) << 24) |
	       (static_cast<ut::uint32>(hitmask_value.G()) << 16) |
	       (static_cast<ut::uint32>(hitmask_value.B()) << 8) |
	       static_cast<ut::uint32>(hitmask_value.A());
}

// Helper function to draw a model's subset.
inline void PerformModelDrawCall(Context& context,
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

// Renders specified range of models.
void HitMask::DrawModelsJob(Context& context,
                            HitMask::ViewData& data,
                            Buffer& view_uniform_buffer,
                            ModelBatcher& batcher,
                            ut::uint32 thread_id,
                            ut::uint32 drawcall_index_offset,
                            ut::uint32 drawcall_count)
{
	if (drawcall_count == 0)
	{
		return;
	}
	
	// extract model policy data
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	ut::Array<Model::DrawCall>& draw_list = batcher.draw_calls;
	Buffer& instance_buffer = batcher.instance_buffer;
	ut::Array<Model::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	const ut::uint32 batch_size = batcher.GetBatchSize();

	// set common uniforms
	model_at_off_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	model_at_on_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	model_at_on_desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);

	// variables tracking if something changes between iterations
	Mesh::VertexFormat prev_vertex_format = Mesh::vertex_format_count;
	ModelRendering::AlphaMode prev_alpha_mode = ModelRendering::alpha_mode_count;
	ModelRendering::CullMode prev_cull_mode = ModelRendering::cull_mode_count;
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
	const ut::uint32 last_element = drawcall_index_offset + drawcall_count - 1;
	for (ut::uint32 i = drawcall_index_offset; i <= last_element; i++)
	{
		// extract material
		Model::DrawCall& dc = draw_list[i];
		Mesh& mesh = dc.model.mesh.Get();
		Mesh::Subset& subset = mesh.subsets[dc.subset_id];
		Material& material = subset.material;
		const bool is_transparent = material.alpha == Material::alpha_transparent;
		const bool alpha_test_on = material.alpha == Material::alpha_masked;

		// skip transparent materials
		if (is_transparent)
		{
			PerformModelDrawCall(context, prev_index_buffer,
			                     prev_index_offset, prev_index_count,
			                     instance_count, i - 1, batch_size);
			instance_count = 0;
			continue;
		}

		// calculate batch id
		const ut::uint32 batch_id = i / batch_size;
		Model::Batch& batch = batches[batch_id];

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
		const ModelRendering::AlphaMode alpha_mode = material.alpha == Material::alpha_masked ?
		                                             ModelRendering::alpha_test :
		                                             ModelRendering::alpha_opaque;
		const ModelRendering::CullMode cull_mode = material.double_sided ?
		                                           ModelRendering::cull_none :
		                                           ModelRendering::cull_back;
		const bool alpha_mode_changed = prev_alpha_mode != alpha_mode;
		bool pipeline_changed = prev_vertex_format != vertex_format ||
		                        prev_cull_mode != cull_mode ||
		                        alpha_mode_changed;

		// check if at least one shader resource has changed
		const bool batch_id_changed = batch_id != prev_batch_id;
		const bool diffuse_map_changed = diffuse_ptr != prev_diffuse_ptr;
		const bool shader_rc_changed = batch_id_changed || alpha_mode_changed ||
		                               (alpha_test_on && diffuse_map_changed);

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
		if (state_changed && (i != drawcall_index_offset))
		{
			PerformModelDrawCall(context, prev_index_buffer,
			                     prev_index_offset, prev_index_count,
			                     instance_count, i - 1, batch_size);
			instance_count = 0;
		}
		instance_count++;

		// set pipeline state
		if (pipeline_changed)
		{
			const size_t pipeline_state_id = ModelRendering::PipelineGrid::GetId(vertex_format,
			                                                                     alpha_mode,
			                                                                     cull_mode);
			context.BindPipelineState(data.model_pipeline[pipeline_state_id]);

			prev_vertex_format = vertex_format;
			prev_alpha_mode = alpha_mode;
			prev_cull_mode = cull_mode;
		}

		// bind descriptors
		if (shader_rc_changed)
		{			
			if (alpha_test_on)
			{
				model_at_on_desc_set.transform_ub.BindUniformBuffer(batch.transform);
				model_at_on_desc_set.hitmask_id_ub.BindUniformBuffer(batch.entity_id);
				model_at_on_desc_set.diffuse.BindImage(material.diffuse.Get());
				context.BindDescriptorSet(model_at_on_desc_set);
			}
			else
			{
				model_at_off_desc_set.transform_ub.BindUniformBuffer(batch.transform);
				model_at_off_desc_set.hitmask_id_ub.BindUniformBuffer(batch.entity_id);
				context.BindDescriptorSet(model_at_off_desc_set);
			}

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
			PerformModelDrawCall(context, index_buffer,
			                     index_offset, index_count,
			                     instance_count, i, batch_size);
		}
	}
}

// Creates shaders for model rendering.
ut::Array<BoundShader> HitMask::CreateModelShader()
{
	ut::Array<BoundShader> shaders;
	constexpr size_t light_permutation_count = ModelRendering::ShaderGrid::size;
	for (size_t i = 0; i < light_permutation_count; i++)
	{
		const size_t vertex_format = ModelRendering::ShaderGrid::GetCoordinate<ModelRendering::vertex_format_column>(i);
		const size_t alpha_mode = ModelRendering::ShaderGrid::GetCoordinate<ModelRendering::alpha_mode_column>(i);

		Shader::Macros macros;
		Shader::MacroDefinition macro;
		ut::String shader_name_suffix;

		// enable instancing
		macro.name = "INSTANCING";
		macro.value = "1";
		macros.Add(macro);

		// hitmask pass
		macro.name = "HITMASK_PASS";
		macro.value = "1";
		macros.Add(macro);

		// batch size
		macro.name = "BATCH_SIZE";
		macro.value = ut::Print(ModelBatcher::CalculateBatchSize(tools.device));
		macros.Add(macro);

		// alpha test
		const bool alpha_test_enabled = alpha_mode == ModelRendering::alpha_test;
		macro.name = "ALPHA_TEST";
		macro.value = alpha_test_enabled ? "1" : "0";
		macros.Add(macro);
		shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

		// vertex traits
		macros += Mesh::GenerateVertexMacros(static_cast<Mesh::VertexFormat>(vertex_format), true);
		shader_name_suffix += ut::String("_vf") + ut::Print(vertex_format);

		ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex,
		                                                            ut::String("hitmask_model_vs") + shader_name_suffix,
		                                                            "VS",
		                                                            "model.hlsl",
		                                                            macros);

		ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
		                                                            ut::String("hitmask_model_ps") + shader_name_suffix,
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

// Creates the hitmask render pass.
ut::Result<RenderPass, ut::Error> HitMask::CreateRenderPass(pixel::Format depth_stencil_format)
{
	RenderTargetSlot depth_slot(depth_stencil_format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	RenderTargetSlot color_slot(skHitMaskFormat, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);

	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot); // hitmask itself

	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot);
}

// Creates a pipeline state to render geometry to the hitmask.
ut::Result<PipelineState, ut::Error> HitMask::CreateModelPipeline(RenderPass& render_pass,
                                                                  ut::uint32 width,
                                                                  ut::uint32 height,
                                                                  Mesh::VertexFormat vertex_format,
                                                                  ModelRendering::AlphaMode alpha_mode,
                                                                  ModelRendering::CullMode cull_mode)
{
	const size_t shader_id = ModelRendering::ShaderGrid::GetId(vertex_format, alpha_mode);
	BoundShader& shader = model_shader[shader_id];

	PipelineState::Info info;
	info.stages[Shader::vertex] = shader.stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = shader.stages[Shader::pixel].Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format, true);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = true;
	info.depth_stencil_state.depth_compare_op = compare::less;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::replace;
	info.depth_stencil_state.back.compare_mask = 0x0;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xff;
	info.depth_stencil_state.stencil_reference = 0x0;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = cull_mode == ModelRendering::cull_back ?
	                                                  RasterizationState::back_culling :
	                                                  RasterizationState::no_culling;
	info.rasterization_state.line_width = 1.0f;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(ut::Move(info), render_pass);
}

// Connects all descriptor sets to the corresponding shaders.
void HitMask::ConnectDescriptors()
{
	const size_t at_off_shader_id = ModelRendering::ShaderGrid::GetId(Mesh::vertex_pos3_texcoord2_normal3_tangent3_float,
	                                                                  ModelRendering::alpha_opaque);
	const size_t at_on_shader_id = ModelRendering::ShaderGrid::GetId(Mesh::vertex_pos3_texcoord2_normal3_tangent3_float,
	                                                                 ModelRendering::alpha_test);
	model_at_off_desc_set.Connect(model_shader[at_off_shader_id]);
	model_at_on_desc_set.Connect(model_shader[at_on_shader_id]);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//