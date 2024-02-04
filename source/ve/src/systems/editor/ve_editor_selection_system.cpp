//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/editor/ve_editor_selection_system.h"
#include "commands/ve_cmd_add_entity.h"
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

	const size_t viewport_count = viewports.Count();
	for (size_t i = 0; i < viewport_count; i++)
	{
		ui::Viewport& viewport = viewports[i];
		const ui::Viewport::Mode mode = viewport.GetMode();
		if (mode.is_active && mode.has_input_focus)
		{
			out_commands += ProcessViewportSelection(access, viewports[i]);
			break;
		}
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
	ut::Optional<Entity::Id> entity_id;
	for (Base::Access::EntityIterator entity = cameras.BeginEntities(); entity != cameras.EndEntities(); ++entity)
	{
		const Entity::Id id = entity->GetFirst();

		// check name
		const ut::String& name = cameras.GetComponent<NameComponent>(id).name;
		if (name == desired_name)
		{
			entity_id = id;
			break;
		}
	}

	// exit if no camera was found
	if (!entity_id)
	{
		return commands;
	}

	// get components
	RenderComponent& camera_render = cameras.GetComponent<RenderComponent>(entity_id.Get());

	// find render view
	ut::Optional<render::View&> render_view;
	const size_t unit_count = camera_render.units.Count();
	for (size_t i = 0; i < unit_count; i++)
	{
		ut::UniquePtr<render::Unit>& unit = camera_render.units[i];
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

	// process input
	ut::Optional< ut::Vector<2> > cursor_position = viewport.GetCursorPosition();
	const input::Bindings& bindings = input_mgr->config.bindings;
	const bool select_key_down = input_mgr->IsKeyDown(bindings.select_entity);
	const bool multiple_selection_key_down = input_mgr->IsKeyDown(bindings.multiplication_modifier);
	if (cursor_position && select_key_down && !prev_frame_select_key_down)
	{
		select_cursor_position = cursor_position;
	}
	prev_frame_select_key_down = select_key_down;

	// deselect previous entities or apply highlight effect for the current ones
	if (select_cursor_position && !multiple_selection_key_down)
	{
		commands += DeselectAllEntities(all_selected_entities, renderable_selected_entities);
	}
	else
	{
		commands += HighlightSelectedEntities(renderable_selected_entities);
	}

	// select entities
	const ut::Optional<Entity::Id> selected_entity = SelectEntity(render_view.Get());
	if (selected_entity)
	{
		// this command adds editor component to the desired entity
		ut::UniquePtr<Cmd> add_select_component_cmd = ut::MakeUnique<CmdAddComponent>(selected_entity.Get(),
		                                                                              ut::MakeUnique<SelectedInEditorComponent>(),
		                                                                              false);
		commands.Add(ut::Move(add_select_component_cmd));
	}

	// enable highlighting effects if at least one entity is selected
	render_view->post_process.stencil_highlight.enabled = renderable_selected_entities.CountEntities() > 0;

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
	for (ut::Array< ut::Ref<ui::Viewport> >::Iterator iterator = start; iterator != end; iterator++)
	{
		viewports.Add(*iterator);
	}
}

//----------------------------------------------------------------------------->
// Deselects all entities that were selected before.
//    @param all_selected_entities - reference to the access to all
//                                   previously selected entities.
//    @param renderable_selected_entities - reference to the access to
//                                          previously selected
//                                          renderable entities.
//    @return - array of commands.
CmdArray ViewportSelectionSystem::DeselectAllEntities(VssSelectedEntitiesAccess& all_selected_entities,
                                                      VssRenderableSelectedEntitiesAccess& renderable_selected_entities)
{
	CmdArray commands;

	// discard color change for all previously selected entities
	for (Access::EntityIterator renderable_entity_iterator = renderable_selected_entities.BeginEntities();
	     renderable_entity_iterator != renderable_selected_entities.EndEntities();
	     ++renderable_entity_iterator)
	{
		const Entity::Id id = renderable_entity_iterator->GetFirst();
		RenderComponent& selected_render = renderable_selected_entities.GetComponent<RenderComponent>(id);

		const size_t unit_count = selected_render.units.Count();
		for (size_t i = 0; i < unit_count; i++)
		{
			ut::UniquePtr<render::Unit>& unit = selected_render.units[i];
			if (unit->Identify().GetHandle() == ut::GetPolymorphicHandle<ve::render::Model>())
			{
				render::Model& model = static_cast<render::Model&>(unit.GetRef());
				model.highlighted = false;
			}
		}
	}

	// remove all selection components
	const ut::DynamicType::Handle sel_comp_type_handle = ut::GetPolymorphicHandle<SelectedInEditorComponent>();
	for (Access::EntityIterator selected_entity_iterator = all_selected_entities.BeginEntities();
	     selected_entity_iterator != all_selected_entities.EndEntities();
	     ++selected_entity_iterator)
	{
		const Entity::Id id = selected_entity_iterator->GetFirst();
		commands.Add(ut::MakeUnique<CmdDeleteComponent>(id, sel_comp_type_handle));
	}

	return commands;
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
	for (Access::EntityIterator renderable_entity_iterator = renderable_selected_entities.BeginEntities();
	     renderable_entity_iterator != renderable_selected_entities.EndEntities();
	     ++renderable_entity_iterator)
	{
		const Entity::Id id = renderable_entity_iterator->GetFirst();
		RenderComponent& selected_render = renderable_selected_entities.GetComponent<RenderComponent>(id);

		const size_t unit_count = selected_render.units.Count();
		for (size_t i = 0; i < unit_count; i++)
		{
			ut::UniquePtr<render::Unit>& unit = selected_render.units[i];
			if (unit->Identify().GetHandle() == ut::GetPolymorphicHandle<ve::render::Model>())
			{
				render::Model& model = static_cast<render::Model&>(unit.GetRef());
				model.highlighted = true;
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
