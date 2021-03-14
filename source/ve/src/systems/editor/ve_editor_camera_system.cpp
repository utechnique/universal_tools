//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/editor/ve_editor_camera_system.h"
#include "systems/render/units/ve_render_view.h"
#include "commands/ve_cmd_add_entity.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(editor)
//----------------------------------------------------------------------------//
// Constructor.
ViewportCameraSystem::ViewportCameraSystem(ut::SharedPtr<ui::Frontend::Thread> ui_frontend_thread,
                                           ut::SharedPtr<input::Manager> input_mgr_ptr) :
	Base("editor_viewports_and_cameras"), input_mgr(ut::Move(input_mgr_ptr)), ui_thread(ut::Move(ui_frontend_thread))
{

	ui_thread->Enqueue([&](ui::Frontend& frontend) { InitializeViewports(frontend); });
	timer.Start();
}

//----------------------------------------------------------------------------->
// Updates transform component of the managed entities.
//    @return - empty array of commands.
System::Result ViewportCameraSystem::Update()
{
	CmdArray out_commands;

	const size_t viewport_count = viewports.GetNum();
	for (size_t i = 0; i < viewport_count; i++)
	{
		ut::Optional< ut::UniquePtr<Cmd> > cmd = ProcessViewport(viewports[i]);
		if (cmd)
		{
			out_commands.Add(cmd.Move());
		}
	}

	timer.Start();

	return out_commands;
}

//----------------------------------------------------------------------------->
// Processes camra that is associated with the provided viewport.
// If such camera doesn't exist - a new camera entity will be created.
ut::Optional< ut::UniquePtr<Cmd> > ViewportCameraSystem::ProcessViewport(ui::Viewport& viewport)
{
	// get viewport info
	const ui::Viewport::Id viewport_id = viewport.GetId();
	ut::String desired_name = ut::String("editor_camera_") + ut::Print(viewport_id);
	const ui::Viewport::Mode mode = viewport.GetMode();

	// check if observation key is pressed
	const input::Bindings& bindings = input_mgr->config.bindings;
	const bool observation_mode = input_mgr->IsKeyDown(bindings.observation_mode);

	// search for a camera with desired name
	const size_t entity_count = entities.GetNum();
	for (size_t i = 0; i < entity_count; i++)
	{
		ViewportCameraSystem::Set& set = entities[i];

		// check name
		const ut::String& name = set.Get<NameComponent>().name;
		if (name != desired_name)
		{
			continue;
		}

		// get components
		TransformComponent& transform = set.Get<TransformComponent>();
		CameraComponent& camera = set.Get<CameraComponent>();
		RenderComponent& render = set.Get<RenderComponent>();
		FreeCameraControllerComponent& controller = set.Get<FreeCameraControllerComponent>();

		// update camera properties that are not affected by user input
		UpdateCamera(transform, camera, render, mode, viewport_id);

		// update camera position and direction
		if (mode.has_input_focus)
		{
			const bool observation_allowed = observation_mode && viewport.GetMousePosition();
			const float time_step = timer.GetTime<ut::time::seconds, float>();

			if (camera.projection == CameraComponent::perspective_projection)
			{
				ProcessPerspectiveCameraInput(transform,
												camera,
												controller,
												time_step,
												observation_allowed);
			}
			else if (camera.projection == CameraComponent::orthographic_projection)
			{
				ProcessOrthographicCameraInput(transform,
												camera,
												viewport.GetMouseOffset(true),
												observation_allowed);
			}
		}

		// success
		return ut::Optional< ut::UniquePtr<Cmd> >(); // e x i t
	}

	// a new entity must be created if desired camera doesn't exist
	ut::Result<ut::UniquePtr<Cmd>, ut::Error> new_camera = CreateCamera(viewport_id,
	                                                                    ut::Move(desired_name));
	return new_camera.MoveOrThrow();
}

//----------------------------------------------------------------------------->
// Updates camera according to the associated viewport.
//    @param transform - reference to the transform component.
//    @param camera - reference to the camera component.
//    @param render - reference to the render component.
//    @param mode - const reference to the mode of the associated viewport.
//    @param viewport_id - id of the viewport associated with the camera.
void ViewportCameraSystem::UpdateCamera(TransformComponent& transform,
                                        CameraComponent& camera,
                                        RenderComponent& render,
                                        const ui::Viewport::Mode& mode,
                                        ui::Viewport::Id viewport_id)
{
	// find render view
	ut::Optional<render::View&> view;
	const size_t unit_count = render.units.GetNum();
	for (size_t i = 0; i < unit_count; i++)
	{
		ut::UniquePtr<render::Unit>& unit = render.units[i];
		if (unit->Identify().GetHandle() == ut::GetPolymorphicHandle<ve::render::View>())
		{
			view = static_cast<render::View&>(unit.GetRef());
			break;
		}
	}

	// re-create view unit if it was somehow removed
	if (!view)
	{
		render::View render_view;
		render_view.viewport_id = viewport_id;
		render.units.Add(ut::MakeUnique<render::View>(ut::Move(render_view)));
		view = static_cast<render::View&>(render.units.GetLast().GetRef());
	}

	// do not render view if its viewport is inactive
	view->is_active = mode.is_active;

	// update mode
	switch (mode.render_mode)
	{
		case ui::Viewport::render_mode_complete: view->mode = render::View::mode_complete; break;
		case ui::Viewport::render_mode_diffuse: view->mode = render::View::mode_diffuse; break;
		case ui::Viewport::render_mode_normal: view->mode = render::View::mode_normal; break;
	}

	// update resolution
	ut::uint32 desired_width = mode.width;
	ut::uint32 desired_height = mode.height;

	switch (mode.resolution)
	{
	case ui::Viewport::resolution_4k:
		desired_width = 3840;
		desired_height = 2160;
		break;
	case ui::Viewport::resolution_full_hd:
		desired_width = 1920;
		desired_height = 1080;
		break;
	case ui::Viewport::resolution_hd:
		desired_width = 1280;
		desired_height = 720;
		break;
	}

	if (view->width != desired_width || view->height != desired_height)
	{
		view->width = desired_width;
		view->height = desired_height;
		view->Invalidate();
	}

	// update aspect ration
	camera.aspect_ratio = static_cast<float>(mode.width) /
	                      static_cast<float>(mode.height);

	// update projection
	if (mode.projection == ui::Viewport::perspective)
	{
		// position can be too far after orthographic projection
		if (camera.projection != CameraComponent::perspective_projection)
		{
			transform.translation = ut::Vector<3>(0);
			transform.rotation = ut::Quaternion<float>();
		}

		camera.projection = CameraComponent::perspective_projection;
	}
	else
	{
		// save previous direction vector
		const ut::Vector<3> old_direction = camera.GetDirection(transform.rotation);

		// camera orientation
		ut::Vector<3> euler(0);
		switch (mode.projection)
		{
		case ui::Viewport::orthographic_negative_x: euler.Y() = 180.0f; break;
		case ui::Viewport::orthographic_positive_x: break;
		case ui::Viewport::orthographic_negative_y: euler.Z() = -90.0f; break;
		case ui::Viewport::orthographic_positive_y: euler.Z() = 90.0f; break;
		case ui::Viewport::orthographic_negative_z: euler.Y() = 90.0f; break;
		case ui::Viewport::orthographic_positive_z: euler.Y() = -90.0f; break;
		}
		transform.rotation = ut::Quaternion<float>();
		transform.rotation *= ut::Quaternion<float>::MakeFromAngleAndAxis(euler.Y(), ut::Vector<3>(0, 1, 0));
		transform.rotation *= ut::Quaternion<float>::MakeFromAngleAndAxis(euler.Z(), ut::Vector<3>(0, 0, 1));
		transform.rotation = transform.rotation.Normalize();

		// set camera back to 0 if projection has changed
		const ut::Vector<3> new_direction = camera.GetDirection(transform.rotation);
		if (camera.projection != CameraComponent::orthographic_projection || new_direction != old_direction)
		{
			transform.translation = ut::Vector<3>(0);
		}

		// set correct projection
		camera.projection = CameraComponent::orthographic_projection;

		// set maximum possible camera distance
		const float d = (camera.far_plane - camera.near_plane) / 2.0f;
		switch (mode.projection)
		{
		case ui::Viewport::orthographic_negative_x: transform.translation.X() = d; break;
		case ui::Viewport::orthographic_positive_x: transform.translation.X() = -d; break;
		case ui::Viewport::orthographic_negative_y: transform.translation.Y() = d; break;
		case ui::Viewport::orthographic_positive_y: transform.translation.Y() = -d; break;
		case ui::Viewport::orthographic_negative_z: transform.translation.Z() = d; break;
		case ui::Viewport::orthographic_positive_z: transform.translation.Z() = -d; break;
		}
	}
}

//----------------------------------------------------------------------------->
// Updates transform component of the perspective camera.
//    @param transform - reference to the transform component.
//    @param camera - reference to the camera component.
//    @param controller - const reference to the controller component.
//    @param time_step - time elapsed from the previous frame (in seconds).
//    @param observation_allowed - boolean indicating if camera can rotate.
void ViewportCameraSystem::ProcessPerspectiveCameraInput(TransformComponent& transform,
                                                         CameraComponent& camera,
                                                         const FreeCameraControllerComponent& controller,
                                                         float time_step,
                                                         bool observation_allowed)
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
	if (observation_allowed)
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

//----------------------------------------------------------------------------->
// Updates transform component of the orthographic camera.
//    @param transform - reference to the transform component.
//    @param camera - reference to the camera component.
//    @param viewport_offset - relative (from -1 to +1) 2d offset.
//    @param observation_allowed - boolean indicating if camera can rotate.
void ViewportCameraSystem::ProcessOrthographicCameraInput(TransformComponent& transform,
                                                          CameraComponent& camera,
                                                          ut::Optional< ut::Vector<2> > viewport_offset,
                                                          bool observation_allowed)
{
	// get input bindings
	const input::Bindings& bindings = input_mgr->config.bindings;

	// process zoom
	const float zoom = input_mgr->GetAnalogSignal(bindings.zoom_source);
	if (!ut::Equal(zoom, 0.0f))
	{
		camera.width *= zoom > 0.0f ? 0.8f : 1.25f;
	}

	// next actions are available only if observation key is pressed
	if (observation_allowed && viewport_offset)
	{
		// movement up and right
		const ut::Vector<3> right = camera.GetRight(transform.rotation);
		const ut::Vector<3> up = camera.GetUp(transform.rotation);
		const float height = camera.width / camera.aspect_ratio;
		const float dx = viewport_offset->X() * camera.width * 0.5f;
		const float dy = viewport_offset->Y() * height * 0.5f;
		transform.translation -= right.ElementWise() * dx;
		transform.translation -= up.ElementWise() * dy;
	}
}

//----------------------------------------------------------------------------->
// Creates a new camera entity.
//    @param viewport_id - id of the viewport associated with a new camera.
//    @param name - name of the entity.
//    @return - ve::CmdAddEntity command or ut::Error.
ut::Result<ut::UniquePtr<Cmd>, ut::Error> ViewportCameraSystem::CreateCamera(ui::Viewport::Id viewport_id,
                                                                             ut::String entity_name)
{
	Entity camera;
	ut::Optional<ut::Error> add_error;

	// transform
	TransformComponent transform_component;
	transform_component.translation = ut::Vector<3>(0);
	add_error = camera.AddComponent(ut::MakeUnique<TransformComponent>(ut::Move(transform_component)));
	if (add_error)
	{
		return ut::MakeError(add_error.Move());
	}

	// camera
	add_error = camera.AddComponent(ut::MakeUnique<CameraComponent>());
	if (add_error)
	{
		return ut::MakeError(add_error.Move());
	}

	// controller
	add_error = camera.AddComponent(ut::MakeUnique<FreeCameraControllerComponent>());
	if (add_error)
	{
		return ut::MakeError(add_error.Move());
	}

	// render
	RenderComponent render_component;
	render::View render_view;
	render_view.viewport_id = viewport_id;
	render_component.units.Add(ut::MakeUnique<render::View>(ut::Move(render_view)));
	add_error = camera.AddComponent(ut::MakeUnique<RenderComponent>(ut::Move(render_component)));
	if (add_error)
	{
		return ut::MakeError(add_error.Move());
	}

	// name
	add_error = camera.AddComponent(ut::MakeUnique<NameComponent>(ut::Move(entity_name)));
	if (add_error)
	{
		return ut::MakeError(add_error.Move());
	}

	// success
	ut::UniquePtr<Cmd> cmd = ut::MakeUnique<CmdAddEntity>(ut::Move(camera));
	return ut::Move(cmd);
}

//----------------------------------------------------------------------------->
// Initializes @viewports array.
void ViewportCameraSystem::InitializeViewports(ui::Frontend& ui_frontend)
{
	viewports.Empty();
	ut::Array< ut::Ref<ui::Viewport> >::Iterator start = ui_frontend.BeginViewports();
	ut::Array< ut::Ref<ui::Viewport> >::Iterator end = ui_frontend.EndViewports();
	for (ut::Array< ut::Ref<ui::Viewport> >::Iterator iterator = start; iterator != end; iterator++)
	{
		viewports.Add(*iterator);
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(editor)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
