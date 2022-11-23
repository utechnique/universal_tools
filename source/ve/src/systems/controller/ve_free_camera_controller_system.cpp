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
{
    timer.Start();
}

//----------------------------------------------------------------------------->
// Updates transform component of the managed entities.
//    @return - empty array of commands.
System::Result FreeCameraControllerSystem::Update()
{
	const float time_step = timer.GetTime<ut::time::seconds, float>();
	timer.Start();

	const size_t count = entities.Count();
	for (size_t i = 0; i < count; i++)
	{
		FreeCameraControllerSystem::Set& set = entities[i];
		TransformComponent& transform = set.Get<TransformComponent>();
		CameraComponent& camera = set.Get<CameraComponent>();
		FreeCameraControllerComponent& controller = set.Get<FreeCameraControllerComponent>();
		UpdateCamera(transform, camera, controller, time_step);
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