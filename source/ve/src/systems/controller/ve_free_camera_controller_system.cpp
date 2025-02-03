//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/controller/ve_free_camera_controller_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
FreeCameraControllerSystem::FreeCameraControllerSystem(ut::SharedPtr<input::Manager> input_mgr_ptr) :
	Base("free_camera_controller"), input_mgr(ut::Move(input_mgr_ptr))
{}

//----------------------------------------------------------------------------->
// Updates transform component of the managed entities.
//    @param time_step_ms - time step for the current frame in milliseconds.
//    @param access - reference to the object providing access to the
//                    desired components.
//    @return - empty array of commands.
System::Result FreeCameraControllerSystem::Update(System::Time time_step_ms,
                                                  Base::Access& access)
{
	const System::Time seconds = ut::time::Convert<ut::time::Unit::millisecond,
	                                               ut::time::Unit::second,
	                                               System::Time>(time_step_ms);

	for (Base::Access::EntityIterator entity = access.BeginEntities(); entity != access.EndEntities(); ++entity)
	{
		const Entity::Id entity_id = entity->GetFirst();
		TransformComponent& transform = access.GetComponent<TransformComponent>(entity_id);
		CameraComponent& camera = access.GetComponent<CameraComponent>(entity_id);
		FreeCameraControllerComponent& controller = access.GetComponent<FreeCameraControllerComponent>(entity_id);
		UpdateCamera(transform, camera, controller, static_cast<float>(seconds));
	}

	return CmdArray();
}

//----------------------------------------------------------------------------->
// Updates transform component using camera and controller components.
//    @param transform - reference to the transform component.
//    @param camera - reference to the camera component.
//    @param controller - const reference to the controller component.
//    @param time_step - time elapsed from the previous frame (in seconds).
void FreeCameraControllerSystem::UpdateCamera(TransformComponent& transform,
                                              CameraComponent& camera,
                                              const FreeCameraControllerComponent& controller,
                                              float time_step)
{
	// get input bindings
	const input::Bindings& bindings = input_mgr->config.bindings;

	// calculate movement offset for the current frame
	const float offset = time_step * controller.speed;

	// calculate current 'direction' and 'right' vectors of the camera
	ut::Vector<3> direction = camera.GetDirection(transform.rotation);
	ut::Vector<3> right = camera.GetRight(transform.rotation);

	// process movement
	if (input_mgr->IsKeyDown(bindings.move_forward))
	{
		transform.translation += direction.ElementWise() * offset;
	}
	if (input_mgr->IsKeyDown(bindings.move_backward))
	{
		transform.translation -= direction.ElementWise() * offset;
	}
	if (input_mgr->IsKeyDown(bindings.move_left))
	{
		transform.translation -= right.ElementWise() * offset;
	}
	if (input_mgr->IsKeyDown(bindings.move_right))
	{
		transform.translation += right.ElementWise() * offset;
	}

	// process observation
	if (input_mgr->IsKeyDown(bindings.observation_mode))
	{
		// get xy signal value
		const float x = input_mgr->GetAnalogSignal(bindings.observation_source_x);
		const float y = input_mgr->GetAnalogSignal(bindings.observation_source_y);

		// calculate angular offset
		const float deltax = x * controller.sensitivity;
		const float deltay = y * controller.sensitivity;

		// process horizontal rotation
		ut::Quaternion<float> qx = ut::Quaternion<float>::MakeFromAngleAndAxis(deltax, CameraComponent::skUp);
		transform.rotation = qx * transform.rotation;
		right = camera.GetRight(transform.rotation);

		// rocess vertical rotation
		ut::Quaternion<float> qy = ut::Quaternion<float>::MakeFromAngleAndAxis(deltay, right);
		transform.rotation = qy * transform.rotation;

		// make sure the rotation quaternion is correct
		transform.rotation.Normalize();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//