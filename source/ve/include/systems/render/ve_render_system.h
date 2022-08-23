//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component_system.h"
#include "ve_render_api.h"
#include "systems/ui/ve_ui.h"
#include "engine/ve_render_engine.h"
#include "components/ve_render_component.h"
#include "components/ve_transform_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::RenderSystem is a system responsible for drawing visible
// components and displaying the result to user.
class RenderSystem : public ComponentSystem<RenderComponent, TransformComponent>
{
	typedef ComponentSystem<RenderComponent, TransformComponent> Base;
public:
	// Constructor.
	RenderSystem(ut::SharedPtr<Device::Thread> in_render_thread,
	             ut::SharedPtr<ui::Frontend::Thread> in_ui_thread);

	// Destructor. Engine is destructed in the render thread.
	~RenderSystem();

	// Draws all renderable components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update();

private:
	// Initializes a portion of units.
	void InitializeUnitsJob(size_t first_entity_id, size_t entity_count);

	// Updates the unit cache of the rendering engine, also calculates a
	// transform matrix for every unit.
	void InitializeUnits();

	// Unregisters the desired entity by its identifier.
	//    @param id - identifier of the entity.
	void UnregisterEntity(Entity::Id id) override;

	ut::SharedPtr<ui::Frontend::Thread> ui_thread;
	ut::SharedPtr<Device::Thread> render_thread;
	ut::UniquePtr<Engine> engine;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
