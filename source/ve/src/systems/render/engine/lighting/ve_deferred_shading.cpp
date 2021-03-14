//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/units/ve_render_view.h"
#include "systems/render/engine/lighting/ve_deferred_shading.h"
#include "systems/render/engine/policy/ve_render_model_policy.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
DeferredShading::DeferredShading(Toolset& toolset) : tools(toolset)
                                                   , model_gpass_shader(CreateModelGPassShader())
{}

// Creates deferred shading (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @return - a new DeferredShading::ViewData object or error if failed.
ut::Result<DeferredShading::ViewData, ut::Error> DeferredShading::CreateViewData(Target& depth_stencil,
                                                                                 ut::uint32 width,
                                                                                 ut::uint32 height)
{
	Target::Info info;
	info.type = Image::type_2D;
	info.format = skGBufferFormat;
	info.usage = Target::Info::usage_color;
	info.mip_count = 1;
	info.width = width;
	info.height = height;
	info.depth = 1;

	// diffuse
	ut::Result<Target, ut::Error> diffuse = tools.device.CreateTarget(info);
	if (!diffuse)
	{
		return ut::MakeError(diffuse.MoveAlt());
	}

	// normal
	ut::Result<Target, ut::Error> normal = tools.device.CreateTarget(info);
	if (!normal)
	{
		return ut::MakeError(normal.MoveAlt());
	}

	// render pass
	ut::Result<RenderPass, ut::Error> render_pass = CreateGeometryPass(depth_stencil.GetInfo().format);
	if (!render_pass)
	{
		return ut::MakeError(render_pass.MoveAlt());
	}

	// framebuffer
	ut::Array< ut::Ref<Target> > color_targets;
	color_targets.Add(diffuse.Get());
	color_targets.Add(normal.Get());
	ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(render_pass.Get(),
	                                                                                ut::Move(color_targets),
	                                                                                depth_stencil);
	if (!framebuffer)
	{
		return ut::MakeError(framebuffer.MoveAlt());
	}

	// pipeline state
	ut::Result<PipelineState, ut::Error> pipeline = CreateModelGPassPipeline(render_pass.Get(),
	                                                                         width, height);
	if (!pipeline)
	{
		return ut::MakeError(framebuffer.MoveAlt());
	}

	DeferredShading::ViewData data{ diffuse.Move(),
	                                normal.Move(),
	                                render_pass.Move(),
	                                framebuffer.Move(),
	                                pipeline.Move() };

	data.geometry_pass_desc_set.Connect(model_gpass_shader);

	return ut::Move(data);
}

// Creates shaders for rendering geometry to the g-buffer.
BoundShader DeferredShading::CreateModelGPassShader()
{
	Shader::Macros macros;
	Shader::MacroDefinition batch_size;
	batch_size.name = "BATCH_SIZE";
	batch_size.value = ut::Print(ModelBatcher::CalculateBatchSize(tools.device));
	macros.Add(ut::Move(batch_size));

	ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex, "geometry_pass_vs", "VS", "geometry_pass.hlsl", macros);
	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel, "geometry_pass_ps", "PS", "geometry_pass.hlsl", macros);
	return BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow());
}

// Creates a render pass for the g-buffer.
ut::Result<RenderPass, ut::Error> DeferredShading::CreateGeometryPass(pixel::Format depth_stencil_format)
{
	RenderTargetSlot depth_slot(depth_stencil_format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	RenderTargetSlot color_slot(skGBufferFormat, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);

	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot); // diffuse
	color_slots.Add(color_slot); // normal

	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot);
}

// Creates a pipeline state to render geometry to the g-buffer.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateModelGPassPipeline(RenderPass& render_pass,
                                                                               ut::uint32 width,
                                                                               ut::uint32 height)
{
	PipelineState::Info info;
	info.stages[Shader::vertex] = model_gpass_shader.stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = model_gpass_shader.stages[Shader::pixel].Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = tools.rc_mgr.cube->input_assembly_instancing;
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = true;
	info.depth_stencil_state.depth_compare_op = compare::less;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.rasterization_state.line_width = 1.0f;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // diffuse
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // normal
	return tools.device.CreatePipelineState(ut::Move(info), render_pass);
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

// Renders model units to the g-buffer.
void DeferredShading::BakeModels(Context& context,
                                 DeferredShading::ViewData& data,
                                 Buffer& view_uniform_buffer,
                                 ModelBatcher& batcher)
{
	//return;
	
	// extract model policy data
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	ut::Array<Model::DrawCall>& draw_list = batcher.draw_calls;
	Buffer& instance_buffer = batcher.instance_buffer;
	ut::Array<Model::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	const ut::uint32 batch_size = batcher.GetBatchSize();

	// begin a render pass
	const Framebuffer::Info& fb_info = data.framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.pass, data.framebuffer, render_area, ut::Color<4>(0.11f, 0.11f, 0.11f, 1.0f), 1.0f);
	context.BindPipelineState(data.model_pipeline);

	// set common uniforms
	data.geometry_pass_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	data.geometry_pass_desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);

	// variables tracking if something changes between iterations
	Map* prev_diffuse_ptr = nullptr;
	Map* prev_normal_ptr = nullptr;
	Map* prev_material_ptr = nullptr;
	Buffer* prev_vertex_buffer = nullptr;
	Buffer* prev_index_buffer = nullptr;
	ut::uint32 prev_index_offset = 0;
	ut::uint32 prev_index_count = 0;
	ut::uint32 prev_batch_id = static_cast<ut::uint32>(batches.GetNum());

	// number of elements to draw at once
	ut::uint32 instance_count = 0;

	// iterate primitive groups and try to merge them in the
	// least possible amount of draw calls
	const ut::uint32 count = static_cast<ut::uint32>(draw_list.GetNum());
	for (ut::uint32 i = 0; i < count; i++)
	{
		// calculate batch id
		const ut::uint32 batch_id = i / batch_size;
		Model::Batch& batch = batches[batch_id];

		// extract material
		Model::DrawCall& dc = draw_list[i];
		Mesh& mesh = dc.model.mesh.Get();
		Mesh::Subset& subset = mesh.subsets[dc.subset_id];
		Material& material = subset.material;

		// material maps
		Map* diffuse_ptr = &material.diffuse.Get();
		Map* normal_ptr = &material.normal.Get();
		Map* material_ptr = &material.material.Get();

		// buffers
		Buffer* vertex_buffer = &mesh.vertex_buffer;
		Buffer* index_buffer = mesh.index_buffer ? &mesh.index_buffer.Get() : nullptr;

		// index count
		const ut::uint32 index_offset = subset.index_offset;
		const ut::uint32 index_count = subset.index_count;

		// check if at least one shader resource has changed
		const bool shader_rc_changed = batch_id != prev_batch_id ||
		                               diffuse_ptr != prev_diffuse_ptr ||
		                               normal_ptr != prev_normal_ptr ||
		                               material_ptr != prev_material_ptr;

		// check buffers
		const bool vertex_buffer_changed = prev_vertex_buffer != vertex_buffer;
		const bool index_buffer_changed = prev_index_buffer != index_buffer;
		
		// check index count
		const bool indices_changed = prev_index_offset != index_offset ||
		                             prev_index_count != index_count;

		// check if at least one factor has changed since the previous iteration
		const bool state_changed = shader_rc_changed ||
		                           vertex_buffer_changed ||
		                           index_buffer_changed ||
		                           indices_changed;

		// draw previous instance group
		if (state_changed && i != 0)
		{
			PerformModelDrawCall(context, index_buffer,
			                     index_offset, index_count,
			                     instance_count, i, batch_size);
			instance_count = 0;
		}
		else
		{
			instance_count++;
		}

		// bind descriptors
		if (shader_rc_changed)
		{
			data.geometry_pass_desc_set.transform_ub.BindUniformBuffer(batch.transform);
			data.geometry_pass_desc_set.material_ub.BindUniformBuffer(batch.material);
			data.geometry_pass_desc_set.diffuse.BindImage(material.diffuse.Get());
			data.geometry_pass_desc_set.normal.BindImage(material.normal.Get());
			data.geometry_pass_desc_set.material.BindImage(material.material.Get());
			context.BindDescriptorSet(data.geometry_pass_desc_set);

			prev_batch_id = batch_id;
			prev_diffuse_ptr = diffuse_ptr;
			prev_normal_ptr = normal_ptr;
			prev_material_ptr = material_ptr;
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
		if ( i == count - 1)
		{
			PerformModelDrawCall(context, index_buffer,
			                     index_offset, index_count,
			                     instance_count, i, batch_size);
		}
	}

	context.EndRenderPass();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//