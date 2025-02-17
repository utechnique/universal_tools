//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component_system.h"
#include "systems/input/ve_input_manager.h"
#include "systems/ui/ve_ui.h"
#include "components/ve_transform_component.h"
#include "components/ve_camera_component.h"
#include "components/ve_name_component.h"
#include "components/ve_render_component.h"
#include "components/ve_free_camera_controller_component.h"
#include "systems/render/engine/units/ve_render_view.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(editor)
//----------------------------------------------------------------------------//
// ve::editor::ViewportCameraSystem creates and updates cameras for ui viewports.
class ViewportCameraSystem : public ComponentSystem<TransformComponent,
                                                    CameraComponent,
                                                    NameComponent,
                                                    RenderComponent,
                                                    FreeCameraControllerComponent>
{
	typedef ComponentSystem<TransformComponent,
	                        CameraComponent,
	                        NameComponent,
	                        RenderComponent,
	                        FreeCameraControllerComponent> Base;
public:
	// Constructor.
	ViewportCameraSystem(ut::SharedPtr<ui::Frontend::Thread> ui_frontend_thread,
	                     ut::SharedPtr<input::Manager> input_mgr_ptr);

	// Updates transform component of the managed entities.
	//    @param time_step_ms - time step for the current frame in milliseconds.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update(System::Time time_step_ms,
	                      Base::Access& access) override;

private:
	// Processes camera that is associated with the provided viewport.
	// If such camera doesn't exist - a new camera entity will be created.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @param viewport - reference to the viewport to be processed.
	//    @param time_step - time elapsed from the previous frame (in seconds).
	CmdArray ProcessViewport(Base::Access& access,
	                         ui::Viewport& viewport,
	                         float time_step);

	// Updates camera according to the associated viewport.
	//    @param transform - reference to the transform component.
	//    @param camera - reference to the camera component.
	//    @param render - reference to the render component.
	//    @param mode - const reference to the mode of the associated viewport.
	//    @param use_resize_delay - indicates if to resize render view immediately.
	void UpdateCamera(TransformComponent& transform,
	                  CameraComponent& camera,
	                  render::View& render_view,
	                  const ui::Viewport::Mode& mode,
	                  bool force_resize = false);

	// Updates transform component of the perspective camera.
	//    @param transform - reference to the transform component.
	//    @param camera - reference to the camera component.
	//    @param controller - const reference to the controller component.
	//    @param time_step - time elapsed from the previous frame (in seconds).
	//    @param observation_allowed - boolean indicating if camera can rotate.
	void ProcessPerspectiveCameraInput(TransformComponent& transform,
	                                   CameraComponent& camera,
	                                   const FreeCameraControllerComponent& controller,
	                                   float time_step,
	                                   bool observation_allowed);

	// Updates transform component of the orthographic camera.
	//    @param transform - reference to the transform component.
	//    @param camera - reference to the camera component.
	//    @param viewport_offset - relative (from -1 to +1) 2d offset.
	//    @param observation_allowed - boolean indicating if camera can rotate.
	void ProcessOrthographicCameraInput(TransformComponent& transform,
	                                    CameraComponent& camera,
	                                    ut::Optional< ut::Vector<2> > viewport_offset,
	                                    bool observation_allowed);

	// Creates a new camera entity.
	//    @param viewport_id - id of the viewport associated with a new camera.
	// 	  @param mode - const reference to the mode of the associated viewport.
	//    @param name - name of the entity.
	//    @return - ve::CmdAddEntity command or ut::Error.
	ut::Result<ut::UniquePtr<Cmd>, ut::Error> CreateCamera(ui::Viewport::Id viewport_id,
	                                                       const ui::Viewport::Mode& mode,
	                                                       ut::String entity_name);

	// Initializes @viewports array.
	void InitializeViewports(ui::Frontend& ui_frontend);

	ut::SharedPtr<input::Manager> input_mgr;
	ut::Array< ut::Ref<ui::Viewport> > viewports;

	static constexpr ut::uint64 skResolutionUpdateIntervalMs = 500;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(editor)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
