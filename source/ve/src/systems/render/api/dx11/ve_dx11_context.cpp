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

// Sets a constant buffer used by the appropriate shader pipeline stage.
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

// Sets an image used by the appropriate shader pipeline stage.
void PlatformContext::SetImage(ut::uint32 slot, ID3D11ShaderResourceView* srv)
{
	for (ut::uint32 i = 0; i < Shader::skStageCount; i++)
	{
		if (!stage_bound[i])
		{
			continue;
		}

		switch (i)
		{
		case Shader::vertex:   d3d11_context->VSSetShaderResources(slot, 1, &srv); break;
		case Shader::hull:     d3d11_context->HSSetShaderResources(slot, 1, &srv); break;
		case Shader::domain:   d3d11_context->DSSetShaderResources(slot, 1, &srv); break;
		case Shader::geometry: d3d11_context->GSSetShaderResources(slot, 1, &srv); break;
		case Shader::pixel:    d3d11_context->PSSetShaderResources(slot, 1, &srv); break;
		case Shader::compute:  d3d11_context->CSSetShaderResources(slot, 1, &srv); break;
		}
	}
}

// Sets a sampler used by the appropriate shader pipeline stage.
void PlatformContext::SetSampler(ut::uint32 slot, ID3D11SamplerState* sampler_state)
{
	for (ut::uint32 i = 0; i < Shader::skStageCount; i++)
	{
		if (!stage_bound[i])
		{
			continue;
		}

		switch (i)
		{
		case Shader::vertex:   d3d11_context->VSSetSamplers(slot, 1, &sampler_state); break;
		case Shader::hull:     d3d11_context->HSSetSamplers(slot, 1, &sampler_state); break;
		case Shader::domain:   d3d11_context->DSSetSamplers(slot, 1, &sampler_state); break;
		case Shader::geometry: d3d11_context->GSSetSamplers(slot, 1, &sampler_state); break;
		case Shader::pixel:    d3d11_context->PSSetSamplers(slot, 1, &sampler_state); break;
		case Shader::compute:  d3d11_context->CSSetSamplers(slot, 1, &sampler_state); break;
		}
	}
}

// Returns d3d11 resource associated with provided image.
ID3D11Resource* PlatformContext::GetDX11ImageResource(PlatformImage& image, ut::uint32 type)
{
	switch (type)
	{
	case Image::type_1D: return image.tex1d.Get();
	case Image::type_2D: return image.tex2d.Get();
	case Image::type_cube: return image.tex2d.Get();
	case Image::type_3D: return image.tex3d.Get();
	}
	return nullptr;
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
		if (buffer.info.usage == render::memory::gpu_read_cpu_write)
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

// Maps a memory object associated with provided image
// into application address space. Note that image must be created with
// usage flag to be compatible with this function.
//    @param image - reference to the ve::render::Image object to be mapped.
//    @param mip_level - id of the mip to be mapped.
//    @param array_layer - id of the layer to be mapped.
//    @param access - ut::Access value specifying purpose of the mapping
//                    operation - read, write or both.
ut::Result<Image::MappedResource, ut::Error> Context::MapImage(Image& image,
                                                               ut::Access access,
                                                               ut::uint32 mip_level,
                                                               ut::uint32 array_layer)
{
	Image::MappedResource mapped_rc;
	const Image::Info& info = image.GetInfo();

	D3D11_MAP map_type;
	if (access == ut::access_read)
	{
		UT_ASSERT(info.usage != render::memory::gpu_read_cpu_write);
		map_type = D3D11_MAP_READ;
	}
	else if (access == ut::access_write)
	{
		map_type = D3D11_MAP_WRITE_DISCARD;
	}
	else
	{
		UT_ASSERT(info.usage != render::memory::gpu_read_cpu_write);
		map_type = D3D11_MAP_READ_WRITE;
	}

	ID3D11Resource* rc_ptr = GetDX11ImageResource(image, info.type);
	UT_ASSERT(rc_ptr != nullptr);

	const UINT subrc_id = array_layer * info.mip_count + mip_level;
	
	// only images created with gpu_read_cpu_write flag have linear layout
	// and can be accessed without staging buffer
	if (info.usage == render::memory::gpu_read_cpu_write)
	{
		D3D11_MAPPED_SUBRESOURCE mapped_subrc;
		HRESULT result = d3d11_context->Map(rc_ptr, subrc_id, map_type, 0, &mapped_subrc);
		if (FAILED(result))
		{
			return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to map d3d11 buffer."));
		}

		mapped_rc.data = mapped_subrc.pData;
		mapped_rc.row_pitch = mapped_subrc.RowPitch;
		mapped_rc.depth_pitch = mapped_subrc.DepthPitch;
	}
	else
	{
		return ut::MakeError(ut::error::not_supported);
	}

	return mapped_rc;
}

// Unmaps a previously mapped memory object associated with provided image.
void Context::UnmapImage(Image& image)
{
	const Image::Info& info = image.GetInfo();

	ID3D11Resource* rc_ptr = GetDX11ImageResource(image, info.type);
	UT_ASSERT(rc_ptr != nullptr);
	
	if (info.usage == render::memory::gpu_read_cpu_write)
	{
		d3d11_context->Unmap(rc_ptr, 0);
	}
}

// Copies data between render targets.
//    @param dst - the destination target, must be in transfer_dst state.
//    @param src - the source target, must be in transfer_src state.
void Context::CopyTarget(Target& dst, Target& src)
{
	const Target::Info& src_info = src.GetInfo();
	const Target::Info& dst_info = dst.GetInfo();
	Image& src_image = src.GetImage();
	Image& dst_image = dst.GetImage();

	if (src_info.format != dst_info.format ||
	    src_info.type != dst_info.type)
	{
		throw ut::Error(ut::error::types_not_match,
		                "Unable to copy images with different formats.");
	}

	const UINT array_slices = src_info.type == Image::type_cube ? 6 : 1;
	for (ut::uint32 cubeface = 0; cubeface < array_slices; cubeface++)
	{
		const UINT sub_resource = D3D11CalcSubresource(0, cubeface, 1);
		d3d11_context->CopySubresourceRegion(dst_image.tex2d.Get(), sub_resource, 0, 0, 0,
		                                     src_image.tex2d.Get(), sub_resource, NULL);
	}
}

// Begin a new render pass.
//    @param render_pass - reference to the render pass object.
//    @param framebuffer - reference to the framebuffer to be bound.
//    @param render_area - reference to the rectangle representing
//                         rendering area in pixels.
//    @param clear_color - color to clear render targets with.
//    @param depth_clear_value - value to clear depth buffer with.
//    @param stencil_clear_value - value to clear stencil buffer with.
void Context::BeginRenderPass(RenderPass& render_pass,
                              Framebuffer& framebuffer,
                              const ut::Rect<ut::uint32>& render_area,
                              const ClearColor& clear_color,
                              float depth_clear_value,
                              ut::uint32 stencil_clear_value)
{
	// validate arguments
	if (!clear_color)
	{
		UT_ASSERT(clear_color.GetAlt().GetNum() == render_pass.color_slots.GetNum());
	}
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
	ut::Array<ID3D11RenderTargetView*> rtv(color_target_count);
	for (size_t i = 0; i < color_target_count; i++)
	{
		rtv[i] = framebuffer.color_targets[i]->rtv.Get();

		// clear color target
		if (render_pass.color_slots[i].load_op == RenderTargetSlot::load_clear)
		{
			const float* color_ptr = clear_color ? clear_color->GetData() :
			                                       clear_color.GetAlt()[i].GetData();
			d3d11_context->ClearRenderTargetView(rtv[i], color_ptr);
		}
	}

	// form d3d dsv resource
	ID3D11DepthStencilView* dsv = nullptr;
	if (framebuffer.depth_stencil_target)
	{
		dsv = framebuffer.depth_stencil_target.Get()->dsv.Get();

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
		const ut::Optional<Descriptor::Binding>& binding = descriptor.GetBinding();
		if (!binding)
		{
			continue;
		}

		const size_t slot_count = binding->slots.GetNum();
		for (size_t j = 0; j < slot_count; j++)
		{
			const ut::Optional<Descriptor::Slot>& slot = binding->slots[j];
			if (!slot)
			{
				continue;
			}

			const ut::uint32 binding_id = binding->id + slot->array_id;

			if (binding->type == Shader::Parameter::uniform_buffer)
			{
				SetUniformBuffer(binding_id, slot->uniform_buffer->d3d11_buffer.Get());
			}
			else if (binding->type == Shader::Parameter::image)
			{
				if (slot->cube_face)
				{
					SetImage(binding_id, slot->image->cube_faces[slot->cube_face.Get()].Get());
				}
				else
				{
					SetImage(binding_id, slot->image->srv.Get());
				}
			}
			else if (binding->type == Shader::Parameter::sampler)
			{
				SetSampler(binding_id, slot->sampler->sampler_state.Get());
			}
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

// Binds vertex and instance buffers to the current context.
//    @param vertex_buffer - reference to the vertex buffer to be bound.
//    @param vertex_offset - number of bytes between the first element
//                           of a vertex buffer and the first element
//                           that will be used.
//    @param instance_buffer - reference to the instance buffer to be bound.
//    @param instance_offset - number of bytes between the first element
//                             of an instance buffer and the first element
//                             that will be used.
void Context::BindVertexAndInstanceBuffer(Buffer& vertex_buffer,
                                          size_t vertex_offset,
                                          Buffer& instance_buffer,
                                          size_t instance_offset)
{
	ID3D11Buffer* vbuffer = vertex_buffer.d3d11_buffer.Get();
	ID3D11Buffer* ibuffer = instance_buffer.d3d11_buffer.Get();
	UINT vertex_stride = vertex_buffer.info.stride;
	UINT ui32_vertex_offset = static_cast<UINT>(vertex_offset);
	UINT ui32_instance_offset = static_cast<UINT>(instance_offset);

	ID3D11Buffer* buffers[] = { vbuffer, ibuffer };
	UINT stride[] = { vertex_buffer.info.stride, instance_buffer.info.stride };
	UINT offset[] = { ui32_vertex_offset, ui32_instance_offset };
	d3d11_context->IASetVertexBuffers(0, 2, buffers, stride, offset);
}

// Binds index buffer to the current context.
//    @param buffer - reference to the buffer to be bound.
//    @param offset - number of bytes between the first element
//                    of an index buffer and the first index
//                    that will be used.
//    @param index_type - type of index buffer indices (16 or 32).
void Context::BindIndexBuffer(Buffer& buffer,
                              size_t offset,
                              IndexType index_type)
{
	d3d11_context->IASetIndexBuffer(buffer.d3d11_buffer.Get(),
	                                index_type == index_type_uint32 ?
	                                DXGI_FORMAT_R32_UINT :
	                                DXGI_FORMAT_R16_UINT,
	                                static_cast<UINT>(offset));
}

// Draw non-indexed, non-instanced primitives.
//    @param vertex_count - number of vertices to draw.
//    @param first_vertex_id - index of the first vertex.
void Context::Draw(ut::uint32 vertex_count, ut::uint32 first_vertex_id)
{
	d3d11_context->Draw(vertex_count, first_vertex_id);
}

// Draw non-indexed, instanced primitives.
//    @param vertex_count - number of vertices to draw.
//    @param instance_count - number of instances to draw.
//    @param first_vertex_id - index of the first vertex.
//    @param first_instance_id - a value added to each index before reading
//                               per-instance data from a vertex buffer.
void Context::DrawInstanced(ut::uint32 vertex_count,
                            ut::uint32 instance_count,
                            ut::uint32 first_vertex_id,
                            ut::uint32 first_instance_id)
{
	d3d11_context->DrawInstanced(vertex_count,
	                             instance_count,
	                             first_vertex_id,
	                             first_instance_id);
}

// Draw indexed, non-instanced primitives.
//    @param index_count - number of vertices to draw.
//    @param first_index_id - the base index within the index buffer.
//    @param vertex_offset - the value added to the vertex index before
//                           indexing into the vertex buffer.
void Context::DrawIndexed(ut::uint32 index_count,
                          ut::uint32 first_index_id,
                          ut::int32 vertex_offset)
{
	d3d11_context->DrawIndexed(index_count, first_index_id, vertex_offset);
}

// Draw indexed, instanced primitives.
//    @param index_count - number of vertices to draw.
//    @param instance_count - number of instances to draw.
//    @param first_index_id - the base index within the index buffer.
//    @param vertex_offset - the value added to the vertex index before
//                           indexing into the vertex buffer.
//    @param first_instance_id - a value added to each index before reading
//                               per-instance data from a vertex buffer.
void Context::DrawIndexedInstanced(ut::uint32 index_count,
                                   ut::uint32 instance_count,
                                   ut::uint32 first_index_id,
                                   ut::int32 vertex_offset,
                                   ut::uint32 first_instance_id)
{
	d3d11_context->DrawIndexedInstanced(index_count,
	                                    instance_count,
	                                    first_index_id,
	                                    vertex_offset,
	                                    first_instance_id);
}

// Toggles render target's state.
//    @param targets - reference to the shared target data array.
//    @param state - new state of the target.
void Context::SetTargetState(ut::Array<SharedTargetData>& targets,
                             Target::Info::State state)
{
	const ut::uint32 target_count = static_cast<ut::uint32>(targets.GetNum());
	for (ut::uint32 i = 0; i < target_count; i++)
	{
		targets[i]->info.state = state;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//