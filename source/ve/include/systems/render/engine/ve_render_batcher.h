//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_render_toolset.h"
#include "units/ve_render_mesh_instance.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Batcher class is used to minimize the number of
// draw calls and cpu->gpu data transfer time.
class Batcher
{
public:
	// Per-frame gpu resources.
	struct FrameData
	{
		ut::Array<MeshInstance::Batch> batches;
	};

	// Constructor.
	Batcher(Toolset &toolset_ref);

	// Registers provided mesh instance unit.
	void Register(Entity::Id entity_id, ut::Array< ut::UniquePtr<Unit> >& units);

	// Unregisters all units with the provided entity identifier..
	void Unregister(Entity::Id entity_id);

	// Updates a batch (material and transform buffer) with specified units.
	void UpdateBatch(Context& context,
	                 MeshInstance::Batch& batch,
	                 size_t first_element_id,
	                 size_t element_count);

	// Updates desired batch.
	void UpdateBatchById(Context& context, size_t batch_id);

	// Updates all batches.
	void UpdateBuffers(Context& context);

	// Returns the number of elements in one batch.
	ut::uint32 GetBatchSize() const;

	// Calculates number of individual elements in one batch.
	static ut::uint32 CalculateBatchSize(Device& device);

	// Per-frame data.
	ut::Array<FrameData> frame_data;

	// vertex buffer with instance identifiers
	Buffer instance_buffer;

	// 'drawcall' here means indivisible part of a mesh
	ut::Array<MeshInstance::DrawCall> draw_calls;

private:
	// Creates a vertex buffer with instance id for the batch.
	static Buffer CreateInstanceBuffer(Device& device);

	// common engine tools
	Toolset &tools;

	// number of individual elements in one batch
	ut::uint32 batch_size;

	// true if buffers can be updated from non-main threads
	bool parallelize_buffer_update;

	// protects Register() and Unregister() methods
	ut::Mutex mutex;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
