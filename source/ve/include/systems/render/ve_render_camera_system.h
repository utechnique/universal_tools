//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component_system.h"
#include "components/ve_transform_component.h"
#include "components/ve_camera_component.h"
#include "components/ve_render_component.h"
#include "systems/render/engine/units/ve_render_view.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::CameraSystem updates ve::render::View unit of the
// render component to match camera component.
class CameraSystem : public ComponentSystem<TransformComponent,
                                            CameraComponent,
                                            RenderComponent>
{
public:
	// Constructor.
	CameraSystem();

	// Updates view unit of all entities having transform,
	// render and camera component.
	//    @param time_step_ms - time step for the current frame in milliseconds.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - empty array of commands.
	System::Result Update(System::Time time_step_ms,
	                      Access& access) override;

private:
	// Updates view matrix of the render view.
	//    @param transform - const reference to transform component.
	//    @param camera - const reference to camera component.
	//    @param view - reference to the view unit to be updated.
	static void UpdateView(const TransformComponent& transform,
	                       const CameraComponent& camera,
	                       View& view);

	// Updates perspective projection matrix of the render view.
	//    @param camera - const reference to camera component.
	//    @param view - reference to the view unit to be updated.
	static void UpdatePerspectiveProjection(const CameraComponent& camera,
	                                        View& view);

	// Updates orthographic projection matrix of the render view.
	//    @param camera - const reference to camera component.
	//    @param view - reference to the view unit to be updated.
	static void UpdateOrthographicProjection(const CameraComponent& camera,
	                                         View& view);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
