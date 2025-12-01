//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_batcher.h"
#include "systems/render/engine/ve_render_hitmask.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Batcher::Batcher(Toolset &toolset) : tools(toolset)
                                   , frame_data(toolset.config.frames_in_flight)
                                   , instance_buffer(CreateInstanceBuffer(toolset.device))
                                   , parallelize_buffer_update(toolset.device.GetInfo().supports_async_rc_mapping)
                                   , batch_size(CalculateBatchSize(toolset.device))
{}

// Registers provided mesh instance unit.
void Batcher::Register(Entity::Id entity_id, ut::Array< ut::UniquePtr<Unit> >& units)
{
	const ut::DynamicType::Handle mesh_inst_handle = ut::GetPolymorphicHandle<MeshInstance>();
	const size_t unit_count = units.Count();
	for (size_t i = 0; i < unit_count; i++)
	{
		if (units[i].Get() == nullptr)
		{
			continue;
		}

		Unit& unit = units[i].GetRef();
		const ut::DynamicType& unit_type = unit.Identify();
		if (unit_type.GetHandle() != mesh_inst_handle)
		{
			continue;
		}

		ut::ScopeLock lock(mutex);

		MeshInstance& instance = static_cast<MeshInstance&>(unit);
		const ut::uint32 subset_count = static_cast<ut::uint32>(instance.mesh->subsets.Count());
		for (ut::uint32 subset_id = 0; subset_id < subset_count; subset_id++)
		{
			draw_calls.Add(MeshInstance::DrawCall{ instance, entity_id, subset_id });
		}
	}
}

// Unregisters all units with the provided entity identifier..
void Batcher::Unregister(Entity::Id entity_id)
{
	ut::ScopeLock lock(mutex);
	const size_t count = draw_calls.Count();
	for (size_t i = count; i-- > 0;)
	{
		if (draw_calls[i].entity_id == entity_id)
		{
			draw_calls.Remove(i);
		}
	}
}

// Updates a batch (material and transform buffer) with specified units.
void Batcher::UpdateBatch(Context& context,
                          MeshInstance::Batch& batch,
                          size_t first_element_id,
                          size_t element_count)
{
    // Linux works better with intermediate buffer while windows
	// is faster writing directly to the mapped memory
#if UT_LINUX
	const size_t transform_size = element_count * sizeof(MeshInstance::TransformBuffer);
	const size_t material_size = element_count * sizeof(MeshInstance::MaterialBuffer);
	ut::Array<ut::byte> cache(transform_size + material_size);

	MeshInstance::TransformBuffer* transform_memory = reinterpret_cast<MeshInstance::TransformBuffer*>(cache.GetAddress());
	MeshInstance::MaterialBuffer* material_memory = reinterpret_cast<MeshInstance::MaterialBuffer*>(transform_memory + element_count);
#else
	ut::Result<void*, ut::Error> transform_map_result = context.MapBuffer(batch.transform, memory::CpuAccess::write);
	ut::Result<void*, ut::Error> material_map_result = context.MapBuffer(batch.material, memory::CpuAccess::write);

	// copy transform and material data
	MeshInstance::TransformBuffer* transform_memory = static_cast<MeshInstance::TransformBuffer*>(transform_map_result.MoveOrThrow());
	MeshInstance::MaterialBuffer* material_memory = static_cast<MeshInstance::MaterialBuffer*>(material_map_result.MoveOrThrow());
#endif

	// copy data
	MeshInstance::DrawCall* dc_start = &draw_calls[first_element_id];
    for (size_t j = 0; j < element_count; j++)
	{
		MeshInstance::DrawCall& dc = dc_start[j];
		MeshInstance& instance = dc.instance;
		const Material& material = instance.mesh->subsets[dc.subset_id].material;

		// transform buffer
		ut::memory::Copy(transform_memory + j, &instance.world_matrix, sizeof(ut::Matrix<4, 4>));

		// material buffer
		MeshInstance::MaterialBuffer& material_buffer = material_memory[j];
		material_buffer.base_color_factor.R() = ut::Pow(material.base_color_factor.R() *
		                                                instance.base_color_factor.R(), 2.2f);
		material_buffer.base_color_factor.G() = ut::Pow(material.base_color_factor.G() *
		                                                instance.base_color_factor.G(), 2.2f);
		material_buffer.base_color_factor.B() = ut::Pow(material.base_color_factor.B() *
		                                                instance.base_color_factor.B(), 2.2f);
		material_buffer.base_color_factor.A() = material.base_color_factor.A() *
		                                        instance.base_color_factor.A();
		material_buffer.roughness_factor = material.roughness_factor *
		                                   instance.roughness_factor;
		material_buffer.metallic_factor = material.metallic_factor *
		                                  instance.metallic_factor;
		material_buffer.emissive_strength = material.emissive_factor.Length() *
		                                    instance.emissive_strength;
		material_buffer.occlusion_strength = material.occlusion_strength *
		                                     instance.occlusion_strength;
		material_buffer.normal_scale = material.normal_scale *
		                               instance.normal_scale;
		material_buffer.alpha_cutoff = material.alpha_cutoff;
	}

	// finish mapping
#if UT_LINUX
	ut::Result<void*, ut::Error> transform_map_result = context.MapBuffer(batch.transform, memory::CpuAccess::write);
	ut::memory::Copy(transform_map_result.MoveOrThrow(), transform_memory, transform_size);
    context.UnmapBuffer(batch.transform);

	ut::Result<void*, ut::Error> material_map_result = context.MapBuffer(batch.material, memory::CpuAccess::write);
	ut::memory::Copy(material_map_result.MoveOrThrow(), material_memory, material_size);
    context.UnmapBuffer(batch.material);
#else
	context.UnmapBuffer(batch.transform);
	context.UnmapBuffer(batch.material);
#endif

	// update entity-id buffer if needed
	if (tools.frame_mgr.GetCurrentFrame().info.needs_entity_id_buffer_update)
	{
		ut::Result<void*, ut::Error> id_buffer_map_result = context.MapBuffer(batch.entity_id, memory::CpuAccess::write);
		MeshInstance::EntityIdBuffer* id_buffer_memory = static_cast<MeshInstance::EntityIdBuffer*>(id_buffer_map_result.MoveOrThrow());
		for (size_t j = 0; j < element_count; j++)
		{
			id_buffer_memory[j].entity_id = HitMask::EncodeEntityId(dc_start[j].entity_id);
		}
		context.UnmapBuffer(batch.entity_id);
	}
	
}

// Updates desired batch.
void Batcher::UpdateBatchById(Context& context, size_t batch_id)
{
	// get references to the desired buffers
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	FrameData& frame = frame_data[current_frame_id];

	// calculate how many units contains this buffer
	const size_t dc_count = draw_calls.Count();
	size_t elements_to_update = batch_size;
	if ((batch_id == frame.batches.Count() - 1) && (dc_count % batch_size != 0))
	{
		elements_to_update = dc_count % batch_size;
	}

	const size_t first_element_id = batch_id * batch_size;
	UpdateBatch(context, frame.batches[batch_id], first_element_id, elements_to_update);
}

// Updates all batches.
void Batcher::UpdateBuffers(Context& context)
{
	ut::Optional<Profiler::ScopeCounter> scope_counter =
		tools.profiler.CreateScopeCounter(Profiler::Stat::batching);

	// calculate how many buffers are needed to cover all mesh instance units
	const size_t dc_count = draw_calls.Count();
	const size_t buffer_need = dc_count / batch_size +
	                           ((dc_count % batch_size != 0) ? 1 : 0);

	// allocate new buffers
	FrameData& current_frame = frame_data[tools.frame_mgr.GetCurrentFrameId()];
	const size_t current_batch_count = current_frame.batches.Count();
	for (size_t i = current_batch_count; i < buffer_need; i++)
	{
		// transform
		Buffer::Info buffer_info;
		buffer_info.type = Buffer::Type::uniform;
		buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
		buffer_info.size = batch_size * sizeof(MeshInstance::TransformBuffer);
		ut::Result<Buffer, ut::Error> transform_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!transform_buffer)
		{
			throw ut::Error(transform_buffer.MoveAlt());
		}

		// material
		buffer_info.size = batch_size * sizeof(MeshInstance::MaterialBuffer);
		ut::Result<Buffer, ut::Error> material_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!material_buffer)
		{
			throw ut::Error(material_buffer.MoveAlt());
		}

		// entity-id
		buffer_info.size = batch_size * sizeof(MeshInstance::EntityIdBuffer);
		ut::Result<Buffer, ut::Error> entity_id_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!entity_id_buffer)
		{
			throw ut::Error(entity_id_buffer.MoveAlt());
		}

		// add batch
		current_frame.batches.Add(MeshInstance::Batch{ transform_buffer.Move(),
		                                               material_buffer.Move(),
		                                               entity_id_buffer.Move() });
	}

	// update buffers
	for (size_t i = 0; i < buffer_need; i++)
	{
		if (parallelize_buffer_update)
		{
			auto job = [&, i]()
			{
				UpdateBatchById(context, i);
			};

			tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void()> >(job));
		}
		else
		{
			UpdateBatchById(context, i);
		}
	}

	// wait for threads
	if (parallelize_buffer_update)
	{
		tools.scheduler.WaitForCompletion();
	}
}

// Returns the number of elements in one batch.
ut::uint32 Batcher::GetBatchSize() const
{
	return batch_size;
}

// Calculates number of individual elements in one batch.
ut::uint32 Batcher::CalculateBatchSize(Device& device)
{
	const size_t ub_max_size = ut::Min<size_t>(65536, device.GetInfo().max_uniform_buffer_size);
	const ut::uint32 buffer_bound_size = ut::Max<ut::uint32>(ut::Max<ut::uint32>(sizeof(MeshInstance::TransformBuffer),
	                                                                             sizeof(MeshInstance::MaterialBuffer)),
	                                                                             sizeof(MeshInstance::EntityIdBuffer));
	return static_cast<ut::uint32>(ub_max_size) / buffer_bound_size;
}

// Creates a vertex buffer with instance id for the batch.
Buffer Batcher::CreateInstanceBuffer(Device& device)
{
	ut::uint32 element_count = CalculateBatchSize(device);

	Buffer::Info buffer_info;
	buffer_info.type = Buffer::Type::vertex;
	buffer_info.stride = pixel::GetSize(Mesh::skInstanceIdFormat);
	buffer_info.usage = render::memory::Usage::gpu_immutable;
	buffer_info.size = element_count * buffer_info.stride;

	buffer_info.data.Resize(buffer_info.size);
	for (ut::uint32 i = 0; i < element_count; i++)
	{
		*reinterpret_cast<ut::uint32*>(&buffer_info.data[i * sizeof(ut::uint32)]) = i;
	}

	ut::Result<Buffer, ut::Error> buffer = device.CreateBuffer(ut::Move(buffer_info));
	if (!buffer)
	{
		throw ut::Error(buffer.MoveAlt());
	}

	return buffer.Move();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//