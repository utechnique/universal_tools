//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_sandbox.h"
#include "commands/ve_cmd_add_entity.h"
#include "components/ve_render_component.h"
#include "components/ve_transform_component.h"
#include "systems/render/units/ve_render_model.h"
//----------------------------------------------------------------------------//
// Creates and runs virtual environment.
void LaunchVirtualEnvironment()
{
	// start log
	ut::Optional<ut::Error> log_error = ut::log.Start("log.txt");

	// create default environment
	ve::Environment environment;

	const int nz = 100;
	const int ny = 100;
	srand(0);
	for (int y = -ny; y < ny; y++)
	{
		for (int z = -nz; z < nz; z++)
		{
			ve::TransformComponent transform_component;
			transform_component.translation.Y() = static_cast<float>(y) * 3;
			transform_component.translation.Z() = static_cast<float>(z) * 3;

			int deg = rand() % 360;
			int scalex = 200 + rand() % 800;
			int scaley = 200 + rand() % 800;
			int scalez = 200 + rand() % 800;

			int r = 200 + rand() % 800;
			int g = 200 + rand() % 800;
			int b = 200 + rand() % 800;

			transform_component.rotation = ut::Quaternion<float>::MakeFromAngleAndAxis((float)deg, ut::Vector<3>(0,1,0));
			transform_component.scale = ut::Vector<3>((float)scalex / 500, (float)scaley / 500, (float)scalez / 500);

			ve::render::Model box_model;
			box_model.name = ut::String(ve::render::engine_rc::skDir) + ve::render::engine_rc::skBox;
			box_model.diffuse_mul.R() = static_cast<float>(r) / 1000.0f;
			box_model.diffuse_mul.G() = static_cast<float>(g) / 1000.0f;
			box_model.diffuse_mul.B() = static_cast<float>(b) / 1000.0f;
			ve::RenderComponent render_component;
			render_component.units.Add(ut::MakeUnique<ve::render::Model>(ut::Move(box_model)));
			ve::Entity box;
			box.AddComponent(ut::MakeUnique<ve::RenderComponent>(ut::Move(render_component)));
			box.AddComponent(ut::MakeUnique<ve::TransformComponent>(ut::Move(transform_component)));
			environment.AddEntity(ut::Move(box));
		}
	}

	// run environment
	ut::Optional<ut::Error> exit_error = environment.Run();

	// process exit code
	if (exit_error)
	{
		ut::log << "Exited with error:" << ut::cret;
		ut::log << exit_error->GetDesc();
	}
	else
	{
		ut::log << "Exited successfully." << ut::cret;
	}

	// finish logging
	ut::log.End();
}

// Entry point.
int UT_MAIN(UT_MAIN_ARG)
{
	ut::CatchExceptions(LaunchVirtualEnvironment);

	// exit
	return 0;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//