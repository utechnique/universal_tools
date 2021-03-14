//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_model_batcher.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
ModelBatcher::ModelBatcher(Toolset &toolset) : tools(toolset)
                                             , frame_data(toolset.config.frames_in_flight)
                                             , instance_buffer(CreateInstanceBuffer(toolset.device))
                                             , parallelize_buffer_update(toolset.device.GetInfo().supports_async_rc_mapping)
                                             , batch_size(CalculateBatchSize(toolset.device))
{}

// Registers provided model unit.
void ModelBatcher::Register(Entity::Id entity_id, ut::Array< ut::UniquePtr<Unit> >& units)
{
	const ut::DynamicType::Handle model_handle = ut::GetPolymorphicHandle<Model>();
	const size_t unit_count = units.GetNum();
	for (size_t i = 0; i < unit_count; i++)
	{
		Unit& unit = units[i].GetRef();
		const ut::DynamicType& unit_type = unit.Identify();
		if (unit_type.GetHandle() != model_handle)
		{
			continue;
		}

		ut::ScopeLock lock(mutex);

		Model& model = static_cast<Model&>(unit);
		const ut::uint32 subset_count = static_cast<ut::uint32>(model.mesh->subsets.GetNum());
		for (ut::uint32 subset_id = 0; subset_id < subset_count; subset_id++)
		{
			draw_calls.Add(Model::DrawCall{ model, entity_id, subset_id });
		}
	}
}

// Unregisters all units with the provided entity identifier..
void ModelBatcher::Unregister(Entity::Id entity_id)
{
	ut::ScopeLock lock(mutex);
	const size_t count = draw_calls.GetNum();
	for (size_t i = count; i-- > 0;)
	{
		if (draw_calls[i].entity_id == entity_id)
		{
			draw_calls.Remove(i);
		}
	}
}

// Initializes a provided model unit.
void ModelBatcher::Initialize(Model& model)
{
	// load mesh
	ut::Result<RcRef<Mesh>, ut::Error> mesh = tools.rc_mgr.Find<Mesh>(model.name);
	if (!mesh)
	{
		throw ut::Error(ut::error::not_found);
	}
	model.mesh = mesh.Move();
}

// Updates a batch (material and transform buffer) with specified units.
void ModelBatcher::UpdateBatch(Context& context,
                               Model::Batch& batch,
                               size_t first_element_id,
                               size_t element_count)
{
	// map
	ut::Result<void*, ut::Error> transform_map_result = context.MapBuffer(batch.transform, ut::access_write);
	if (!transform_map_result)
	{
		throw ut::Error(transform_map_result.MoveAlt());
	}
	ut::Result<void*, ut::Error> material_map_result = context.MapBuffer(batch.material, ut::access_write);
	if (!material_map_result)
	{
		throw ut::Error(material_map_result.MoveAlt());
	}

	// copy transform and material data
	Model::TransformBuffer* transform_memory = static_cast<Model::TransformBuffer*>(transform_map_result.Get());
	Model::MaterialBuffer* material_memory = static_cast<Model::MaterialBuffer*>(material_map_result.Get());
	Model::DrawCall* dc_start = &draw_calls[first_element_id];
	for (size_t j = 0; j < element_count; j++)
	{
		Model::DrawCall& dc = dc_start[j];
		Model& model = dc.model;
		ut::memory::Copy(transform_memory + j, &model.world_trasform, sizeof(ut::Matrix<4, 4>));

		Model::MaterialBuffer& material = material_memory[j];
		ut::memory::Copy(&material.diffuse_add, &model.diffuse_add, sizeof(ut::Color<3>));
		ut::memory::Copy(&material.diffuse_mul, &model.diffuse_mul, sizeof(ut::Color<3>));
		ut::memory::Copy(&material.material_add, &model.material_add, sizeof(ut::Color<3>));
		ut::memory::Copy(&material.material_mul, &model.material_mul, sizeof(ut::Color<3>));
	}

	// unmap
	context.UnmapBuffer(batch.transform);
	context.UnmapBuffer(batch.material);
}

// Updates desired batch.
void ModelBatcher::UpdateBatchById(Context& context, size_t batch_id)
{
	// get references to the desired buffers
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	FrameData& frame = frame_data[current_frame_id];

	// calculate how many units contains this buffer
	const size_t dc_count = draw_calls.GetNum();
	size_t elements_to_update = batch_size;
	if ((batch_id == frame.batches.GetNum() - 1) && (dc_count % batch_size != 0))
	{
		elements_to_update = dc_count % batch_size;
	}

	const size_t first_element_id = batch_id * batch_size;
	UpdateBatch(context, frame.batches[batch_id], first_element_id, elements_to_update);
}

// Updates all batches.
void ModelBatcher::UpdateBuffers(Context& context)
{
	// calculate how many buffers are needed to cover all model units
	const size_t dc_count = draw_calls.GetNum();
	const size_t buffer_need = dc_count / batch_size +
	                           ((dc_count % batch_size != 0) ? 1 : 0);

	// allocate new buffers
	FrameData& current_frame = frame_data[tools.frame_mgr.GetCurrentFrameId()];
	const size_t current_batch_count = current_frame.batches.GetNum();
	for (size_t i = current_batch_count; i < buffer_need; i++)
	{
		// transform
		Buffer::Info buffer_info;
		buffer_info.type = Buffer::uniform;
		buffer_info.usage = render::memory::gpu_read_cpu_write;
		buffer_info.size = batch_size * sizeof(Model::TransformBuffer);
		ut::Result<Buffer, ut::Error> transform_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!transform_buffer)
		{
			throw ut::Error(transform_buffer.MoveAlt());
		}

		// material
		buffer_info.size = batch_size * sizeof(Model::MaterialBuffer);
		ut::Result<Buffer, ut::Error> material_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!material_buffer)
		{
			throw ut::Error(material_buffer.MoveAlt());
		}

		// add batch
		current_frame.batches.Add(Model::Batch{transform_buffer.Move(), material_buffer.Move()});
	}

	// update buffers
	for (size_t i = 0; i < buffer_need; i++)
	{
		if (parallelize_buffer_update)
		{
			void(ModelBatcher::*callback)(Context&, size_t) = &ModelBatcher::UpdateBatchById;
			auto f = ut::MemberFunction<ModelBatcher, void(Context&, size_t)>(this, callback);
			tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void(Context&, size_t)> >(f, context, i));
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
ut::uint32 ModelBatcher::GetBatchSize() const
{
	return batch_size;
}

// Calculates number of individual elements in one batch.
ut::uint32 ModelBatcher::CalculateBatchSize(Device& device)
{
	const size_t ub_max_size = device.GetInfo().max_uniform_buffer_size;
	const ut::uint32 buffer_bound_size = ut::Max<ut::uint32>(sizeof(Model::TransformBuffer),
	                                                         sizeof(Model::MaterialBuffer));
	return static_cast<ut::uint32>(ub_max_size) / buffer_bound_size;
}

// Creates a vertex buffer with instance id for the batch.
Buffer ModelBatcher::CreateInstanceBuffer(Device& device)
{
	ut::uint32 element_count = CalculateBatchSize(device);

	Buffer::Info buffer_info;
	buffer_info.type = Buffer::vertex;
	buffer_info.stride = pixel::GetSize(Mesh::skInstanceIdFormat);
	buffer_info.usage = render::memory::gpu_immutable;
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