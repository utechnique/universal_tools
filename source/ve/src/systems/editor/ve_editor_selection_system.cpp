//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/editor/ve_editor_selection_system.h"
#include "commands/ve_cmd_add_entity.h"
#include "commands/ve_cmd_delete_entity.h"
#include "commands/ve_cmd_add_component.h"
#include "commands/ve_cmd_delete_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(editor)
//----------------------------------------------------------------------------//
// Constructor.
ViewportSelectionSystem::ViewportSelectionSystem(ut::SharedPtr<ui::Frontend::Thread> ui_frontend_thread,
	ut::SharedPtr<input::Manager> input_mgr_ptr) : ViewportSelectionSystem::Base("editor_select_entities_in_viewport")
	                                             , input_mgr(ut::Move(input_mgr_ptr))
	                                             , ui_thread(ut::Move(ui_frontend_thread))
{
	ui_thread->Enqueue([&](ui::Frontend& frontend) { InitializeViewports(frontend); });
}

//----------------------------------------------------------------------------->
// Performs entity selection.
//    @param time_step_ms - time step for the current frame in milliseconds.
//    @param access - reference to the object providing access to the
//                    desired components.
//    @return - array of commands.
System::Result ViewportSelectionSystem::Update(System::Time time_step_ms,
                                               Base::Access& access)
{
	CmdArray out_commands;

	for (ui::Viewport& viewport : viewports)
	{
		const ui::Viewport::Mode mode = viewport.GetMode();
		if (!mode.is_active)
		{
			continue;
		}

		out_commands += ProcessViewportSelection(access, viewport);
	}

	return out_commands;
}

//----------------------------------------------------------------------------->
// Processes entity selection routine in the desired viewport.
CmdArray ViewportSelectionSystem::ProcessViewportSelection(Base::Access& access,
                                                           ui::Viewport& viewport)
{
	CmdArray commands;

	// get access
	VssCameraAccess cameras = access.Get<VssCameraAccess>();
	VssSelectedEntitiesAccess all_selected_entities = access.Get<VssSelectedEntitiesAccess>();
	VssRenderableSelectedEntitiesAccess renderable_selected_entities = access.Get<VssRenderableSelectedEntitiesAccess>();

	// get viewport info
	const ui::Viewport::Id viewport_id = viewport.GetId();
	ut::String desired_name = ut::String("editor_camera_") + ut::Print(viewport_id);
	const ui::Viewport::Mode mode = viewport.GetMode();

	// search for a camera with desired name
	ut::Optional<Entity::Id> camera_entity_id;
	for (const auto& camera : cameras)
	{
		const Entity::Id id = camera.GetFirst();

		// check name
		const ut::String& name = cameras.GetComponent<NameComponent>(id).name;
		if (name == desired_name)
		{
			camera_entity_id = id;
			break;
		}
	}

	// exit if no camera was found
	if (!camera_entity_id)
	{
		return commands;
	}

	// get components
	RenderComponent& camera_render = cameras.GetComponent<RenderComponent>(camera_entity_id.Get());

	// find render view
	ut::Optional<render::View&> render_view;
	for (ut::UniquePtr<render::Unit>& unit : camera_render.units)
	{
		if (unit->Identify().GetHandle() == ut::GetPolymorphicHandle<ve::render::View>())
		{
			render_view = static_cast<render::View&>(unit.GetRef());
			break;
		}
	}

	// exit if there is no render view
	if (!render_view)
	{
		ut::log.Lock() << "[Warning] ViewportSelectionSystem: no render view unit "
			"for the viewport having an input focus, viewport_id=" << viewport_id << ut::cret;
		return commands;
	}

	// enable highlighting effects if at least one entity is selected
	render_view->post_process.stencil_highlight.enabled = renderable_selected_entities.CountEntities() > 0;

	// now the selection itself is performed below,
	// exit if current viewport is not available for input
	if (!mode.is_interactive)
	{
		return commands;
	}

	// process delete entities operation
	const input::Bindings& bindings = input_mgr->config.bindings;
	if (input_mgr->IsKeyDown(bindings.delete_entity))
	{
		for (const auto& selected_entity : all_selected_entities)
		{
			const Entity::Id id = selected_entity.GetFirst();
			commands.Add(ut::MakeUnique<CmdDeleteEntity>(id));
		}
	}

	// process input
	ut::Optional< ut::Vector<2> > cursor_position = viewport.GetCursorPosition();
	const bool select_key_down = input_mgr->IsKeyDown(bindings.select_entity);
	const bool multiple_selection_key_down = input_mgr->IsKeyDown(bindings.multiplication_modifier);
	if (cursor_position && select_key_down && !prev_frame_select_key_down)
	{
		select_cursor_position = cursor_position;
	}
	prev_frame_select_key_down = select_key_down && cursor_position;

	// deselect previous entities or apply highlight effect for the current ones
	if (select_cursor_position && !multiple_selection_key_down)
	{
		commands += Deselect(all_selected_entities, renderable_selected_entities);
	}
	else
	{
		commands += HighlightSelectedEntities(renderable_selected_entities);
	}

	// get the entity id under the cursor
	const ut::Optional<Entity::Id> selected_entity = SelectEntity(render_view.Get());
	if (!selected_entity)
	{
		return commands;
	}

	// select or deselect an entity under the sursor
	if (multiple_selection_key_down && IsEntitySelected(selected_entity.Get(), all_selected_entities))
	{
		commands += Deselect(all_selected_entities, renderable_selected_entities, selected_entity);
	}
	else
	{
		ut::UniquePtr<Cmd> add_select_component_cmd = ut::MakeUnique<CmdAddComponent>(selected_entity.Get(),
			                                                                          ut::MakeUnique<SelectedInEditorComponent>(),
			                                                                          false);
		commands.Add(ut::Move(add_select_component_cmd));
	}

	// success
	return commands;
}

//----------------------------------------------------------------------------->
// Returnes the identifier of the selected entity id using the hitmask of
// the desired viewport.
ut::Optional<Entity::Id> ViewportSelectionSystem::SelectEntity(render::View& render_view)
{
	// exit if user didn't click
	if (!select_cursor_position)
	{
		return ut::Optional<Entity::Id>();
	}

	// check if hitmask is ready
	if (render_view.hitmask.Count() == 0)
	{
		render_view.draw_hitmask = true;
		return ut::Optional<Entity::Id>();
	}

	// get entity id from the hitmask
	const ut::uint32 x = static_cast<ut::uint32>((select_cursor_position->X() * 0.5f + 0.5f) * render_view.width);
	const ut::uint32 y = static_cast<ut::uint32>((1.0f - select_cursor_position->Y() * 0.5f - 0.5f) * render_view.height);
	const Entity::Id entity_id = render_view.hitmask[y * render_view.width + x];

	// disable hitmask
	render_view.draw_hitmask = false;

	// reset cursor click position
	select_cursor_position = ut::Optional< ut::Vector<2> >();

	// id was successfully extracted
	return entity_id == -1 ? ut::Optional<Entity::Id>() : ut::Optional<Entity::Id>(entity_id);
}

//----------------------------------------------------------------------------->
// Initializes @viewports array.
void ViewportSelectionSystem::InitializeViewports(ui::Frontend& ui_frontend)
{
	viewports.Reset();
	ut::Array< ut::Ref<ui::Viewport> >::Iterator start = ui_frontend.BeginViewports();
	ut::Array< ut::Ref<ui::Viewport> >::Iterator end = ui_frontend.EndViewports();
	for (ut::Array< ut::Ref<ui::Viewport> >::Iterator iterator = start;
	     iterator != end;
	     iterator++)
	{
		viewports.Add(*iterator);
	}
}

//----------------------------------------------------------------------------->
// Deselects all entities that were selected before, or only one of them.
	//    @param all_selected_entities - reference to the access to all
	//                                   previously selected entities.
	//    @param renderable_selected_entities - reference to the access to
	//                                          previously selected
	//                                          renderable entities.
	// 	  @param entity_id - optional id of the entity to be deselected, if
	// 	                     this parameter is empty, all entities will be
	//                       deselected.
	//    @return - array of commands to be executed by environment.
CmdArray ViewportSelectionSystem::Deselect(VssSelectedEntitiesAccess& all_selected_entities,
                                           VssRenderableSelectedEntitiesAccess& renderable_selected_entities,
                                           ut::Optional<Entity::Id> entity_id)
{
	CmdArray commands;

	// discard color change for all previously selected entities
	for (const auto& renderable_entity : renderable_selected_entities)
	{
		const Entity::Id id = renderable_entity.GetFirst();
		RenderComponent& selected_render = renderable_selected_entities.GetComponent<RenderComponent>(id);

		if (entity_id && entity_id.Get() != id)
		{
			continue;
		}

		for (ut::UniquePtr<render::Unit>& unit : selected_render.units)
		{
			if (unit->Identify().GetHandle() == ut::GetPolymorphicHandle<ve::render::MeshInstance>())
			{
				render::MeshInstance& mesh_instance = static_cast<render::MeshInstance&>(unit.GetRef());
				mesh_instance.highlighted = false;
			}
		}

		if (entity_id)
		{
			break;
		}
	}

	// remove all selection components
	const ut::DynamicType::Handle sel_comp_type_handle = ut::GetPolymorphicHandle<SelectedInEditorComponent>();
	for (const auto& selected_entity : all_selected_entities)
	{
		const Entity::Id id = selected_entity.GetFirst();

		if (entity_id && entity_id.Get() != id)
		{
			continue;
		}

		commands.Add(ut::MakeUnique<CmdDeleteComponent>(id, sel_comp_type_handle));

		if (entity_id)
		{
			break;
		}
	}

	return commands;
}

//----------------------------------------------------------------------------->
// Checks if the provided entity is already selected.
// 	  @param entity_id - id of the entity to check.
//    @return - returns true if the entity identified by
//              @entity_id is already selected.
bool ViewportSelectionSystem::IsEntitySelected(Entity::Id entity_id,
                                               VssSelectedEntitiesAccess& all_selected_entities)
{
	for (const auto& selected_entity : all_selected_entities)
	{
		if (selected_entity.GetFirst() == entity_id)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------->
// Highlights selected entities.
//    @param renderable_selected_entities - reference to the access to
//                                          previously selected
//                                          renderable entities.
//    @return -  array of commands to be executed by environment.
CmdArray ViewportSelectionSystem::HighlightSelectedEntities(VssRenderableSelectedEntitiesAccess& renderable_selected_entities)
{
	CmdArray commands;

	// change color for all selected entities
	for (const auto& renderable_entity : renderable_selected_entities)
	{
		const Entity::Id id = renderable_entity.GetFirst();
		RenderComponent& selected_render = renderable_selected_entities.GetComponent<RenderComponent>(id);

		for (ut::UniquePtr<render::Unit>& unit : selected_render.units)
		{
			if (unit->Identify().GetHandle() == ut::GetPolymorphicHandle<ve::render::MeshInstance>())
			{
				render::MeshInstance& mesh_instance = static_cast<render::MeshInstance&>(unit.GetRef());
				mesh_instance.highlighted = true;
			}
		}
	}

	return commands;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(editor)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
