//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component_system.h"
#include "systems/input/ve_input_manager.h"
#include "components/ve_transform_component.h"
#include "components/ve_camera_component.h"
#include "components/ve_free_camera_controller_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::FreeCameraControllerSystem modifies camera transform component according
// to the user input.
class FreeCameraControllerSystem : public ComponentSystem<TransformComponent,
                                                          CameraComponent,
                                                          FreeCameraControllerComponent>
{
	typedef ComponentSystem<TransformComponent,
	                        CameraComponent,
	                        FreeCameraControllerComponent> Base;
public:
	// Constructor.
	FreeCameraControllerSystem(ut::SharedPtr<input::Manager> input_mgr_ptr);

	// Updates transform component of the managed entities.
	//    @param time_step_ms - time step for the current frame in milliseconds.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update(System::Time time_step_ms,
	                      Base::Access& access) override;

private:
	// Updates transform component using camera and controller components.
	//    @param transform - reference to the transform component.
	//    @param camera - reference to the camera component.
	//    @param controller - const reference to the controller component.
	//    @param time_step - time elapsed from the previous frame (in seconds).
	void UpdateCamera(TransformComponent& transform,
	                  CameraComponent& camera,
	                  const FreeCameraControllerComponent& controller,
	                  float time_step);

	ut::SharedPtr<input::Manager> input_mgr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
