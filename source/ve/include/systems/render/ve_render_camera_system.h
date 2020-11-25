//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "components/ve_camera_component.h"
#include "components/ve_render_component.h"
#include "systems/render/units/ve_render_view.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::CameraSystem updates ve::render::View unit of the
// render component to match camera component.
class CameraSystem : public ComponentSystem<CameraComponent, RenderComponent>
{
public:
	// Constructor.
	CameraSystem();

	// Updates view unit of all entities having both
	// render and camera component.
	//    @return - empty array of commands.
	System::Result Update();

private:
	// Updates view and projection matrices of the render view.
	//    @param camera - const reference to camera component.
	//    @param view - reference to the view unit to be updated.
	static void UpdateView(const CameraComponent& camera, View& view);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
