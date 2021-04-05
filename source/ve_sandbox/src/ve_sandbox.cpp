//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_sandbox.h"
#include "commands/ve_cmd_add_entity.h"
#include "components/ve_render_component.h"
#include "components/ve_transform_component.h"
#include "systems/render/engine/units/ve_render_model.h"
#include "systems/render/engine/units/ve_render_directional_light.h"
#include "systems/render/engine/units/ve_render_point_light.h"
#include "systems/render/engine/units/ve_render_spot_light.h"
//----------------------------------------------------------------------------//
// Creates a box with random color and scale.
ve::Entity CreateRandomBox(const ut::Vector<3>& position)
{
	// random values
	const float r1 = static_cast<float>(200 + rand() % 800);
	const float r2 = static_cast<float>(200 + rand() % 800);
	const float r3 = static_cast<float>(200 + rand() % 800);

	// transform
	ve::TransformComponent transform_component;
	transform_component.translation = position;
	transform_component.rotation = ut::Quaternion<float>::MakeFromAngleAndAxis(static_cast<float>(rand() % 360),
	                                                                           ut::Vector<3>(0, 1, 0));
	transform_component.scale = ut::Vector<3>(r1 / 500.0f, r2 / 500.0f, r3 / 500.0f);

	// model unit
	ve::render::Model box_model;
	box_model.local_trasform.translation.X() = (rand() % 200) / 40.0f;
	box_model.local_trasform.translation.Y() = (rand() % 200) / 40.0f;
	box_model.local_trasform.translation.Z() = (rand() % 200) / 40.0f;
	box_model.name = ut::String(ve::render::engine_rc::skDir) + ve::render::engine_rc::skBox;
	box_model.diffuse_mul.R() = r1 / 1000.0f;
	box_model.diffuse_mul.G() = r2 / 1000.0f;
	box_model.diffuse_mul.B() = r3 / 1000.0f;

	const bool is_metallic = rand() % 10 == 0;
	float roughness = static_cast<float>(rand() % 255) / 255.0f;

	box_model.material_mul.X() = roughness;
	box_model.material_add.Y() = is_metallic ? 1.0f : 0.0f;

	// render component
	ve::RenderComponent render_component;
	render_component.units.Add(ut::MakeUnique<ve::render::Model>(ut::Move(box_model)));

	// entity
	ve::Entity box;
	box.AddComponent(ut::MakeUnique<ve::RenderComponent>(ut::Move(render_component)));
	box.AddComponent(ut::MakeUnique<ve::TransformComponent>(ut::Move(transform_component)));
	return box;
}

// Creates a light source.
ve::Entity CreateLight(ve::render::Light::SourceType type,
                       const ut::Vector<3>& position = ut::Vector<3>(0))
{
	// transform
	ve::TransformComponent transform;
	transform.translation = position;
	transform.rotation = ut::Quaternion<float>::MakeFromAngles(ut::Vector<3>(0, 0, -60));

	// light source
	ut::UniquePtr<ve::render::Unit> light;
	switch (type)
	{
		case ve::render::Light::source_directional:
		{
			const ut::Vector<3> light_direction = ut::Vector<3>(1, -1, 1).Normalize();
			transform.rotation = ut::Quaternion<float>::MakeShortestRotation(ve::render::DirectionalLight::skDirection,
				light_direction);
			ve::render::DirectionalLight light_unit;
			light_unit.color = ut::Vector<3>(1, 1, 1);
			light_unit.intensity = 0.8f;
			light = ut::MakeUnique<ve::render::DirectionalLight>(ut::Move(light_unit));
		} break;

		case ve::render::Light::source_point:
		{
			ve::render::PointLight light_unit;
			light_unit.color = ut::Vector<3>(1, 1, 1);
			light_unit.intensity = 2.0f;
			light_unit.attenuation_distance = 20.0f;
			light = ut::MakeUnique<ve::render::PointLight>(ut::Move(light_unit));
		} break;

		case ve::render::Light::source_spot:
		{
			transform.rotation = ut::Quaternion<float>::MakeFromAngles(ut::Vector<3>(0, 0, -60));
			ve::render::SpotLight light_unit;
			light_unit.color = ut::Vector<3>(1, 1, 1);
			light_unit.intensity = 2.0f;
			light_unit.attenuation_distance = 80.0f;
			light_unit.inner_cone = 26.0f;
			light_unit.outer_cone = 35.0f;
			light = ut::MakeUnique<ve::render::SpotLight>(ut::Move(light_unit));
		} break;
	}

	// render component
	ve::RenderComponent render;
	render.units.Add(ut::Move(light));

	// entity
	ve::Entity entity;
	entity.AddComponent(ut::MakeUnique<ve::RenderComponent>(ut::Move(render)));
	entity.AddComponent(ut::MakeUnique<ve::TransformComponent>(ut::Move(transform)));
	return entity;
}

// Creates a set of entities for the test scene.
ut::Array<ve::Entity> CreateTestScene()
{
	ut::Array<ve::Entity> out;

	const float x_offset = 40.0f;
	
	// directional light
	out.Add(CreateLight(ve::render::Light::source_directional));

	// point and spot lights
	int point_light_count = 2;
	for (int i = -point_light_count; i <= point_light_count; i++)
	{
		const ut::Vector<3> point_position(x_offset - 25.0f, 40.0f, static_cast<float>(i) * 90.0f);
		out.Add(CreateLight(ve::render::Light::source_point, point_position));

		const ut::Vector<3> spot_position(x_offset - 35.0f, -20.0f, static_cast<float>(i) * 90.0f);
		out.Add(CreateLight(ve::render::Light::source_spot, spot_position));
	}

	// boxes
	const int nx = 2, ny = 50, nz = 50;
	for (int x = -nx; x < nx; x++)
	{
		for (int y = -ny; y < ny; y++)
		{
			for (int z = -nz; z < nz; z++)
			{
				const ut::Vector<3> position(static_cast<float>(x) * 5.0f + x_offset,
				                             static_cast<float>(y) * 6.0f,
				                             static_cast<float>(z) * 6.0f);
				out.Add(CreateRandomBox(position));
			}
		}
	}

	return out;
}

//----------------------------------------------------------------------------//
// Creates and runs virtual environment.
void LaunchVirtualEnvironment()
{
	srand(0);

	// start log
	ut::Optional<ut::Error> log_error = ut::log.Start("log.txt");

	// create default environment
	ve::Environment environment;

	// create test scene
	ut::Array<ve::Entity> entities = CreateTestScene();
	for (size_t i = 0; i < entities.GetNum(); i++)
	{
		environment.AddEntity(ut::Move(entities[i]));
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