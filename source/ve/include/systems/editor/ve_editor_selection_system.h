//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_compound_system.h"
#include "systems/input/ve_input_manager.h"
#include "systems/ui/ve_ui.h"
#include "systems/render/engine/units/ve_render_view.h"
#include "components/ve_camera_component.h"
#include "components/ve_name_component.h"
#include "components/ve_render_component.h"
#include "components/ve_selected_in_editor_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(editor)
//----------------------------------------------------------------------------//
// Compound accesses for the ve::editor::ViewportSelectionSystem.
typedef CompoundAccess<CameraComponent,
                       NameComponent,
                       RenderComponent> VssCameraAccess;
typedef CompoundAccess<SelectedInEditorComponent> VssSelectedEntitiesAccess;
typedef CompoundAccess<SelectedInEditorComponent,
                       RenderComponent> VssRenderableSelectedEntitiesAccess;

// ve::editor::ViewportCameraSystem creates and updates cameras for ui viewports.
class ViewportSelectionSystem : public CompoundSystem<VssCameraAccess,
                                                      VssSelectedEntitiesAccess,
                                                      VssRenderableSelectedEntitiesAccess>
{
	typedef CompoundSystem<VssCameraAccess,
	                       VssSelectedEntitiesAccess,
	                       VssRenderableSelectedEntitiesAccess> Base;

public:
	// Constructor.
	ViewportSelectionSystem(ut::SharedPtr<ui::Frontend::Thread> ui_frontend_thread,
	                        ut::SharedPtr<input::Manager> input_mgr_ptr);

	// Updates transform component of the managed entities.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands.
	System::Result Update(Base::Access& access) override;

private:
	// Processes entity selection in the desired viewport.
	CmdArray ProcessViewportSelection(Base::Access& access,
	                                  ui::Viewport& viewport);

	// Returnes the identifier of the selected entity id using the hitmask of
	// the desired viewport.
	ut::Optional<Entity::Id> SelectEntity(render::View& render_view);

	// Initializes @viewports array.
	void InitializeViewports(ui::Frontend& ui_frontend);

	// Deselects all entities that were selected before.
	//    @param all_selected_entities - reference to the access to all
	//                                   previously selected entities.
	//    @param renderable_selected_entities - reference to the access to
	//                                          previously selected
	//                                          renderable entities.
	//    @return - array of commands to be executed by environment.
	static CmdArray DeselectAllEntities(VssSelectedEntitiesAccess& all_selected_entities,
	                                    VssRenderableSelectedEntitiesAccess& renderable_selected_entities);

	// Highlights selected entities.
	//    @param renderable_selected_entities - reference to the access to
	//                                          previously selected
	//                                          renderable entities.
	//    @return -  array of commands to be executed by environment.
	static CmdArray HighlightSelectedEntities(VssRenderableSelectedEntitiesAccess& renderable_selected_entities);

	ut::Optional< ut::Vector<2> > select_cursor_position;
	bool prev_frame_select_key_down = false;
	ut::SharedPtr<ui::Frontend::Thread> ui_thread;
	ut::SharedPtr<input::Manager> input_mgr;
	ut::Array< ut::Ref<ui::Viewport> > viewports;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(editor)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
