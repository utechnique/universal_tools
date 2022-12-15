//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_system.h"
#include "components/ve_transform_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
RenderSystem::RenderSystem(ut::SharedPtr<Device::Thread> in_render_thread,
                           ut::SharedPtr<ui::Frontend::Thread> in_ui_thread) : Base("render")
                                                                             , render_thread(ut::Move(in_render_thread))
                                                                             , ui_thread(ut::Move(in_ui_thread))
{
	UT_ASSERT(ui_thread);
	UT_ASSERT(render_thread);

	// create viewport manager
	ViewportManager vp_mgr(in_render_thread);

	// start counter to know how much time does it take to
	// launch the render engine.
	ut::time::Counter timer;
	timer.Start();

	// initialize render engine
	render_thread->Enqueue([&](Device& device) { engine = ut::MakeUnique<Engine>(device, ut::Move(vp_mgr)); });

	// ask render thread to create display, but the task must be scheduled from the ui thread
	// to synchronize them both
	ui_thread->Enqueue([&](ui::Frontend& frontend) { engine->OpenViewports(frontend); });

	ut::log.Lock() << "Render engine is ready in "
	               << timer.GetTime<ut::time::seconds>()
	               << "s." << ut::cret;
}

// Destructor. Engine is destructed in the render thread.
RenderSystem::~RenderSystem()
{
	render_thread->Enqueue([&](Device& device) { device.WaitIdle(); engine.Delete(); });
}

// Draws all renderable components.
//    @return - array of commands to be executed by owning environment,
//              or ut::Error if system encountered fatal error.
System::Result RenderSystem::Update()
{
	// update units and calculate world transform matrices
	InitializeUnits();

	// draw scene in render thread
	render_thread->Enqueue([&](Device& device) { engine->ProcessNextFrame(); });

	return CmdArray();
}

// Initializes a portion of units.
void RenderSystem::InitializeUnitsJob(size_t first_entity_id, size_t entity_count)
{
	const size_t last_entity_id = first_entity_id + entity_count;
	for (size_t i = first_entity_id; i < last_entity_id; i++)
	{
		RenderComponent& render_component = entities[i].Get<RenderComponent>();
		TransformComponent& transform_component = entities[i].Get<TransformComponent>();

		// an entity must be re-registered if it has obtained new units
		// or any of the old units has been removed
		bool changed = false;

		// check if the number of units has changed since the previous frame
		const size_t unit_count = render_component.units.Count();
		if (unit_count != render_component.cache.Count())
		{
			changed = true;
		}

		// iterate units to calculate transform matrix and initialize
		// appropriate render resources
		for (size_t j = 0; j < unit_count; j++)
		{
			Unit* unit = render_component.units[j].Get();

			// check if unit pointer has changed since the previous frame
			if (!changed && unit != render_component.cache[j])
			{
				changed = true;
			}

			if (unit == nullptr)
			{
				continue;
			}

			// calculate final transform matrix of the unit
			unit->world_transform = transform_component * unit->local_trasform;
			unit->world_matrix = unit->world_transform.ToMatrix();

			// initialize rendering resources (buffers, textures, etc.)
			engine->InitializeUnit(*unit);
		}

		// re-register units and update a cache
		if (changed)
		{
			render_component.cache.Resize(unit_count);
			for (size_t j = 0; j < unit_count; j++)
			{
				render_component.cache[j] = render_component.units[j].Get();
			}

			engine->UnregisterEntity(entities[i].id);
			engine->RegisterEntity(entities[i].id, render_component.units);
		}
	}
}

// Updates the unit cache of the rendering engine, also calculates a
// transform matrix for every unit.
void RenderSystem::InitializeUnits()
{
	ut::ThreadPool<void, ut::pool_sync::cond_var>& thread_pool = engine->GetThreadPool();
	const size_t entity_count = entities.Count();
	const size_t thread_count = thread_pool.GetThreadCount();
	const size_t entities_per_thread = entity_count / thread_count;

	Toolset::DefaultScheduler scheduler = thread_pool.CreateScheduler();

	size_t entity_id = 0;
	for (size_t i = 0; i < thread_count; i++)
	{
		size_t entities_to_initialize = entities_per_thread + (i == 0 ? (entity_count % thread_count) : 0);

		auto function = ut::MemberFunction<RenderSystem, void(size_t, size_t)>(this, &RenderSystem::InitializeUnitsJob);
		scheduler.Enqueue(ut::MakeUnique<ut::Task<void(size_t, size_t)> >(function, entity_id, entities_to_initialize));

		entity_id += entities_to_initialize;
	}

	scheduler.WaitForCompletion();
}

// Unregisters the desired entity by its identifier.
//    @param id - identifier of the entity.
void RenderSystem::UnregisterEntity(Entity::Id id)
{
	// reset cache
	for (size_t i = 0; i < entities.Count(); i++)
	{
		if (entities[i].id == id)
		{
			RenderComponent& render_component = entities[i].Get<RenderComponent>();
			render_component.cache.Reset();
			break;
		}
	}

	// unregister
	engine->UnregisterEntity(id);
	Base::UnregisterEntity(id);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//