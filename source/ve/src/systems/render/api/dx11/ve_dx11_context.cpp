//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_context.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformContext::PlatformContext(ID3D11DeviceContext* context_ptr) : d3d11_context(context_ptr)
{
	for (ut::uint32 i = 0; i < 6; i++)
	{
		stage_bound[i] = false;
	}
}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&&) noexcept = default;

// Move operator.
PlatformContext& PlatformContext::operator =(PlatformContext&& other) noexcept = default;

// Sets the constant buffers used by the appropriate shader pipeline stage.
void PlatformContext::SetUniformBuffer(ut::uint32 slot, ID3D11Buffer* buffer)
{
	for (ut::uint32 i = 0; i < Shader::skStageCount; i++)
	{
		if (!stage_bound[i])
		{
			continue;
		}

		switch (i)
		{
		case Shader::vertex:   d3d11_context->VSSetConstantBuffers(slot, 1, &buffer); break;
		case Shader::hull:     d3d11_context->HSSetConstantBuffers(slot, 1, &buffer); break;
		case Shader::domain:   d3d11_context->DSSetConstantBuffers(slot, 1, &buffer); break;
		case Shader::geometry: d3d11_context->GSSetConstantBuffers(slot, 1, &buffer); break;
		case Shader::pixel:    d3d11_context->PSSetConstantBuffers(slot, 1, &buffer); break;
		case Shader::compute:  d3d11_context->CSSetConstantBuffers(slot, 1, &buffer); break;
		}
	}
}

//----------------------------------------------------------------------------//
// Constructor.
Context::Context(PlatformContext platform_context) : PlatformContext(ut::Move(platform_context))
{}

// Maps a memory object associated with provided buffer
// into application address space. Note that buffer must be created with
// ve::render::Buffer::gpu_cpu flag to be compatible with this function.
//    @param buffer - reference to the ve::render::Buffer object to be mapped.
//    @param access - ut::Access value specifying purpose of the mapping
//                    operation - read, write or both.
//    @return - pointer to the mapped area or error if failed.
ut::Result<void*, ut::Error> Context::MapBuffer(Buffer& buffer, ut::Access access)
{
	D3D11_MAP map_type;
	if (access == ut::access_full)
	{
		map_type = D3D11_MAP_READ_WRITE;
	}
	else if (access == ut::access_read)
	{
		map_type = D3D11_MAP_READ;
	}
	else if (access == ut::access_write)
	{
		if (buffer.info.usage == render::memory::gpu_cpu)
		{
			if (buffer.info.type == Buffer::uniform)
			{
				map_type = D3D11_MAP_WRITE_DISCARD;
			}
			else
			{
				map_type = D3D11_MAP_WRITE_NO_OVERWRITE;
			}
		}
		else
		{
			map_type = D3D11_MAP_WRITE;
		}
	}

	D3D11_MAPPED_SUBRESOURCE mapped_subrc;
	HRESULT result = d3d11_context->Map(buffer.d3d11_buffer.Get(), 0, map_type, 0, &mapped_subrc);
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to map d3d11 buffer."));
	}

	return mapped_subrc.pData;
}

// Unmaps a previously mapped memory object associated with provided buffer.
void Context::UnmapBuffer(Buffer& buffer)
{
	d3d11_context->Unmap(buffer.d3d11_buffer.Get(), 0);
}

// Begin a new render pass.
//    @param render_pass - reference to the render pass object.
//    @param framebuffer - reference to the framebuffer to be bound.
//    @param render_area - reference to the rectangle representing
//                         rendering area in pixels.
//    @param color_clear_values - array of colors to clear color
//                                render targets with.
//    @param depth_clear_value - value to clear depth buffer with.
//    @param stencil_clear_value - value to clear stencil buffer with.
void Context::BeginRenderPass(RenderPass& render_pass,
	                          Framebuffer& framebuffer,
	                          const ut::Rect<ut::uint32>& render_area,
	                          const ut::Array< ut::Color<4> >& color_clear_values,
	                          float depth_clear_value,
	                          ut::uint32 stencil_clear_value)
{
	// validate arguments
	UT_ASSERT(color_clear_values.GetNum() <= render_pass.color_slots.GetNum());
	UT_ASSERT(render_pass.color_slots.GetNum() == framebuffer.color_targets.GetNum());
	UT_ASSERT(render_pass.depth_stencil_slot ? framebuffer.depth_stencil_target : !framebuffer.depth_stencil_target);

	// unbind all possible render targets from shader bindings
#ifdef DEBUG
	ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
	d3d11_context->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
	d3d11_context->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, srvs);
#endif

	// form d3d mrt array
	const size_t color_target_count = framebuffer.color_targets.GetNum();
	const size_t clear_value_count = color_clear_values.GetNum();
	ut::Array<ID3D11RenderTargetView*> rtv(color_target_count);
	for (size_t i = 0; i < color_target_count; i++)
	{
		rtv[i] = framebuffer.color_targets[i]->rtv.Get();

		// clear color target
		if (i < clear_value_count && render_pass.color_slots[i].load_op == RenderTargetSlot::load_clear)
		{
			d3d11_context->ClearRenderTargetView(rtv[i], color_clear_values[i].GetData());
		}
	}

	// form d3d dsv resource
	ID3D11DepthStencilView* dsv = nullptr;
	if (framebuffer.depth_stencil_target)
	{
		dsv = framebuffer.depth_stencil_target->dsv.Get();

		// clear depth and stencil
		if (render_pass.depth_stencil_slot->load_op == RenderTargetSlot::load_clear)
		{
			d3d11_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			                                     depth_clear_value, stencil_clear_value);
		}
	}

	// set targets
	d3d11_context->OMSetRenderTargets(static_cast<UINT>(color_target_count), rtv.GetAddress(), dsv);
}

// End current render pass.
void Context::EndRenderPass()
{}

// Binds provided pipeline state to the current context.
//    @param pipeline_state - reference to the pipeline state.
void Context::BindPipelineState(PipelineState& state)
{
	// input layout
	d3d11_context->IASetInputLayout(state.input_layout.Get());

	// primitive topology
	d3d11_context->IASetPrimitiveTopology(ConvertPrimitiveTopologyToDX11(state.info.input_assembly_state.topology));

	// set viewports
	const UINT viewport_count = static_cast<UINT>(state.info.viewports.GetNum());
	ut::Array<D3D11_VIEWPORT> viewports(viewport_count);
	ut::Array<D3D11_RECT> scissors;
	for (size_t i = 0; i < viewport_count; i++)
	{
		const Viewport& viewport = state.info.viewports[i];
		viewports[i].TopLeftX = viewport.x;
		viewports[i].TopLeftY = viewport.y;
		viewports[i].Width = viewport.width;
		viewports[i].Height = viewport.height;
		viewports[i].MinDepth = viewport.min_depth;
		viewports[i].MaxDepth = viewport.max_depth;

		if (viewport.scissor)
		{
			D3D11_RECT scissor;
			scissor.left = viewport.scissor->offset.X();
			scissor.top = viewport.scissor->offset.Y();
			scissor.right = scissor.left + viewport.scissor->extent.X();
			scissor.bottom = scissor.top + viewport.scissor->extent.Y();
			scissors.Add(scissor);
		}
	}
	d3d11_context->RSSetViewports(viewport_count, viewports.GetAddress());
	d3d11_context->RSSetScissorRects(static_cast<UINT>(scissors.GetNum()), scissors.GetAddress());

	// raster state
	d3d11_context->RSSetState(state.rasterizer_state.Get());

	// depth stencil state
	d3d11_context->OMSetDepthStencilState(state.depthstencil_state.Get(), state.info.depth_stencil_state.stencil_reference);

	// blend state
	d3d11_context->OMSetBlendState(state.blend_state.Get(), 0, 0xffffffff);

	// check what shader stages are bound to the pipeline
	for (ut::uint32 i = 0; i < Shader::skStageCount; i++)
	{
		stage_bound[i] = state.info.stages[i];
	}

	// set shaders
	d3d11_context->VSSetShader(stage_bound[Shader::vertex]   ? state.info.stages[Shader::vertex]->vs.Get()   : nullptr, nullptr, 0);
	d3d11_context->GSSetShader(stage_bound[Shader::geometry] ? state.info.stages[Shader::geometry]->gs.Get() : nullptr, nullptr, 0);
	d3d11_context->HSSetShader(stage_bound[Shader::hull]     ? state.info.stages[Shader::hull]->hs.Get()     : nullptr, nullptr, 0);
	d3d11_context->DSSetShader(stage_bound[Shader::domain]   ? state.info.stages[Shader::domain]->ds.Get()   : nullptr, nullptr, 0);
	d3d11_context->PSSetShader(stage_bound[Shader::pixel]    ? state.info.stages[Shader::pixel]->ps.Get()    : nullptr, nullptr, 0);
	d3d11_context->CSSetShader(stage_bound[Shader::compute]  ? state.info.stages[Shader::compute]->cs.Get()  : nullptr, nullptr, 0);
}

// Binds provided descriptor set to the current pipeline.
// Note that BindPipelineState() function must be called before.
//    @param pipeline_state - reference to the pipeline state.
void Context::BindDescriptorSet(DescriptorSet& descriptor_set)
{
	const size_t descriptor_count = descriptor_set.GetDescriptorCount();
	for (size_t i = 0; i < descriptor_count; i++)
	{
		const Descriptor& descriptor = descriptor_set.GetDescriptor(i);
		const ut::Optional<Descriptor::Binding> binding = descriptor.GetBinding();
		if (!binding || !binding->slot)
		{
			continue;
		}

		if (binding->type == Shader::Parameter::uniform_buffer)
		{
			SetUniformBuffer(binding->id, binding->slot->uniform_buffer->d3d11_buffer.Get());
		}
	}
}

// Binds vertex buffer to the current context.
//    @param buffer - reference to the buffer to be bound.
//    @param offset - number of bytes between the first element
//                    of a vertex buffer and the first element
//                    that will be used.
void Context::BindVertexBuffer(Buffer& buffer, size_t offset)
{
	ID3D11Buffer* vertex_buffer = buffer.d3d11_buffer.Get();
	UINT stride = buffer.info.stride;
	UINT ui32_offset = static_cast<UINT>(offset);
	d3d11_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &ui32_offset);
}

// Draw non-indexed, non-instanced primitives.
//    @param vertex_count - number of vertices to draw.
//    @param first_vertex_id - index of the first vertex.
void Context::Draw(ut::uint32 vertex_count, ut::uint32 first_vertex_id)
{
	d3d11_context->Draw(vertex_count, first_vertex_id);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//