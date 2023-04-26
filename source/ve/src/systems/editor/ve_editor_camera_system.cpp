//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/editor/ve_editor_camera_system.h"
#include "systems/render/engine/units/ve_render_view.h"
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
//    @param access - reference to the object providing access to the
//                    desired components.
//    @return - array of commands.
System::Result ViewportCameraSystem::Update(Base::Access& access)
{
	CmdArray out_commands;

	const size_t viewport_count = viewports.Count();
	for (size_t i = 0; i < viewport_count; i++)
	{
		ut::Optional< ut::UniquePtr<Cmd> > cmd = ProcessViewport(access, viewports[i]);
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
ut::Optional< ut::UniquePtr<Cmd> > ViewportCameraSystem::ProcessViewport(Base::Access& access,
                                                                         ui::Viewport& viewport)
{
	// get viewport info
	const ui::Viewport::Id viewport_id = viewport.GetId();
	ut::String desired_name = ut::String("editor_camera_") + ut::Print(viewport_id);
	const ui::Viewport::Mode mode = viewport.GetMode();

	// check if observation key is pressed
	const input::Bindings& bindings = input_mgr->config.bindings;
	const bool observation_mode = input_mgr->IsKeyDown(bindings.observation_mode);

	// search for a camera with desired name
	ut::Optional<Entity::Id> entity_id;
	for (Base::Access::EntityIterator entity = access.BeginEntities(); entity != access.EndEntities(); ++entity)
	{
		const Entity::Id id = entity->GetFirst();

		// check name
		const ut::String& name = access.GetComponent<NameComponent>(id).name;
		if (name == desired_name)
		{
			entity_id = id;
			break;
		}
	}

	// a new entity must be created if desired camera doesn't exist
	if (!entity_id)
	{
		ut::Result<ut::UniquePtr<Cmd>, ut::Error> new_camera = CreateCamera(viewport_id,
		                                                                    ut::Move(desired_name));
		return new_camera.MoveOrThrow();
	}

	// get components
	TransformComponent& transform = access.GetComponent<TransformComponent>(entity_id.Get());
	CameraComponent& camera = access.GetComponent<CameraComponent>(entity_id.Get());
	RenderComponent& render = access.GetComponent<RenderComponent>(entity_id.Get());
	FreeCameraControllerComponent& controller = access.GetComponent<FreeCameraControllerComponent>(entity_id.Get());

	// find render view
	ut::Optional<render::View&> render_view;
	const size_t unit_count = render.units.Count();
	for (size_t i = 0; i < unit_count; i++)
	{
		ut::UniquePtr<render::Unit>& unit = render.units[i];
		if (unit->Identify().GetHandle() == ut::GetPolymorphicHandle<ve::render::View>())
		{
			render_view = static_cast<render::View&>(unit.GetRef());
			break;
		}
	}

	// re-create view unit if it was somehow removed
	if (!render_view)
	{
		render::View new_render_view;
		new_render_view.viewport_id = viewport_id;
		render.units.Add(ut::MakeUnique<render::View>(ut::Move(new_render_view)));
		render_view = static_cast<render::View&>(render.units.GetLast().GetRef());
	}

	// update camera properties that are not affected by user input
	UpdateCamera(transform, camera, render_view.Get(), mode, viewport_id);

	// check if input is allowed
	if (!mode.has_input_focus)
	{
		return ut::Optional< ut::UniquePtr<Cmd> >(); // e x i t
	}

	// update camera position and direction
	ut::Optional< ut::Vector<2> > cursor_position = viewport.GetCursorPosition();
	const bool observation_allowed = observation_mode && cursor_position;
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
		                               viewport.GetCursorOffset(true),
		                               observation_allowed);
	}

	// select entities
	if (cursor_position && input_mgr->IsKeyDown(bindings.select_entity))
	{
		select_cursor_position = cursor_position;
	}

	ut::Optional<Entity::Id> selected_entity = SelectEntity(viewport,
	                                                        render_view.Get());
	if (selected_entity)
	{

	}

	// success
	return ut::Optional< ut::UniquePtr<Cmd> >(); // e x i t
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
                                        render::View& render_view,
                                        const ui::Viewport::Mode& mode,
                                        ui::Viewport::Id viewport_id)
{
	

	// do not render view if its viewport is inactive
	render_view.is_active = mode.is_active;

	// update mode
	switch (mode.render_mode)
	{
		case ui::Viewport::render_mode_complete: render_view.mode = render::View::mode_complete; break;
		case ui::Viewport::render_mode_diffuse: render_view.mode = render::View::mode_diffuse; break;
		case ui::Viewport::render_mode_normal: render_view.mode = render::View::mode_normal; break;
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

	if (render_view.width != desired_width || render_view.height != desired_height)
	{
		render_view.width = desired_width;
		render_view.height = desired_height;
		render_view.Invalidate();
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
	if (!observation_allowed)
	{
		return;
	}

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
	ut::Array< ut::UniquePtr<ve::Component> > components;

	// transform
	TransformComponent transform_component;
	transform_component.translation = ut::Vector<3>(0);
	if (!components.Add(ut::MakeUnique<TransformComponent>(ut::Move(transform_component))))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// camera
	if (!components.Add(ut::MakeUnique<CameraComponent>()))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// controller
	if (!components.Add(ut::MakeUnique<FreeCameraControllerComponent>()))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// render
	RenderComponent render_component;
	render::View render_view;
	render_view.viewport_id = viewport_id;
	render_component.units.Add(ut::MakeUnique<render::View>(ut::Move(render_view)));
	if (!components.Add(ut::MakeUnique<RenderComponent>(ut::Move(render_component))))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// name
	if (!components.Add(ut::MakeUnique<NameComponent>(ut::Move(entity_name))))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// success
	ut::UniquePtr<Cmd> cmd = ut::MakeUnique<CmdAddEntity>(ut::Move(components));
	return ut::Move(cmd);
}

//----------------------------------------------------------------------------->
// Initializes @viewports array.
void ViewportCameraSystem::InitializeViewports(ui::Frontend& ui_frontend)
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
// Returnes the identifier of the selected entity id using the hitmask of
// the desired viewport.
ut::Optional<Entity::Id> ViewportCameraSystem::SelectEntity(const ui::Viewport& ui_viewport,
                                                            render::View& render_view)
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
	return entity_id;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(editor)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
