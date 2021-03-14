//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_camera_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
CameraSystem::CameraSystem() : ComponentSystem<TransformComponent,
                                               CameraComponent,
                                               RenderComponent>("camera_sync")
{}

//----------------------------------------------------------------------------->
// Updates view unit of all entities having transform,
// render and camera component.
//    @return - empty array of commands.
System::Result CameraSystem::Update()
{
	const size_t count = entities.GetNum();
	for (size_t i = 0; i < count; i++)
	{
		CameraSystem::Set& set = entities[i];
		TransformComponent& transform = set.Get<TransformComponent>();
		CameraComponent& camera = set.Get<CameraComponent>();
		RenderComponent& render = set.Get<RenderComponent>();

		// search for the view unit
		const size_t unit_count = render.units.GetNum();
		for (size_t j = 0; j < unit_count; j++)
		{
			ut::UniquePtr<Unit>& unit = render.units[j];
			if (unit->Identify().GetHandle() != ut::GetPolymorphicHandle<ve::render::View>())
			{
				continue;
			}

			View& view = static_cast<View&>(unit.GetRef());

			UpdateView(transform, camera, view);
			if (camera.projection == CameraComponent::perspective_projection)
			{
				UpdatePerspectiveProjection(camera, view);
			}
			else if (camera.projection == CameraComponent::orthographic_projection)
			{
				UpdateOrthographicProjection(camera, view);
			}

			break;
		}
	}

	return CmdArray();
}

//----------------------------------------------------------------------------->
// Updates view and projection matrices of the render view.
//    @param transform - const reference to transform component.
//    @param camera - const reference to camera component.
//    @param view - reference to the view unit to be updated.
void CameraSystem::UpdateView(const TransformComponent& transform,
                              const CameraComponent& camera,
                              View& view)
{
	// transform camera
	const ut::Vector<3> position = transform.translation;
	const ut::Vector<3> direction = camera.GetDirection(transform.rotation);
	const ut::Vector<3> up = camera.GetUp(transform.rotation);
	const ut::Vector<3> right = camera.GetRight(transform.rotation);

	// calculate view matrix
	view.view_matrix(0, 0) = right.X();
	view.view_matrix(0, 1) = up.X();
	view.view_matrix(0, 2) = direction.X();
	view.view_matrix(0, 3) = 0.0f;
	view.view_matrix(1, 0) = right.Y();
	view.view_matrix(1, 1) = up.Y();
	view.view_matrix(1, 2) = direction.Y();
	view.view_matrix(1, 3) = 0.0f;
	view.view_matrix(2, 0) = right.Z();
	view.view_matrix(2, 1) = up.Z();
	view.view_matrix(2, 2) = direction.Z();
	view.view_matrix(2, 3) = 0.0f;
	view.view_matrix(3, 0) = -position.Dot(right);
	view.view_matrix(3, 1) = -position.Dot(up);
	view.view_matrix(3, 2) = -position.Dot(direction);
	view.view_matrix(3, 3) = 1.0f;
}

//----------------------------------------------------------------------------->
// Updates perspective projection matrix of the render view.
//    @param camera - const reference to camera component.
//    @param view - reference to the view unit to be updated.
void CameraSystem::UpdatePerspectiveProjection(const CameraComponent& camera,
                                               View& view)
{
	const float fov = ut::ToRadiands(camera.field_of_view);
	const float scale_y = 1.0f / ut::Tan(fov / 2.0f);
	const float scale_x = scale_y / camera.aspect_ratio;
	const float zn = camera.near_plane;
	const float zf = camera.far_plane;
	view.proj_matrix = ut::Matrix<4, 4>(scale_x, 0,       0,                 0,
	                                    0,       scale_y, 0,                 0,
	                                    0,       0,       zf / (zf - zn),    1,
	                                    0,       0,      -zn*zf / (zf - zn), 0);
}

//----------------------------------------------------------------------------->
// Updates perspective projection matrix of the render view.
//    @param camera - const reference to camera component.
//    @param view - reference to the view unit to be updated.
void CameraSystem::UpdateOrthographicProjection(const CameraComponent& camera,
                                                View& view)
{
	const float height = camera.width / camera.aspect_ratio;
	const float zn = camera.near_plane;
	const float zf = camera.far_plane;
	view.proj_matrix = ut::Matrix<4, 4>::MakeIdentity();
	view.proj_matrix(0, 0) = 2.0f / camera.width;
	view.proj_matrix(1, 1) = 2.0f / height;
	view.proj_matrix(2, 2) = 1.0f / (zf - zn);
	view.proj_matrix(3, 2) = zn / (zn - zf);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//