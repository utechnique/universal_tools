//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_sandbox.h"
#include "commands/ve_cmd_add_entity.h"
#include "components/ve_render_component.h"
#include "components/ve_transform_component.h"
#include "components/ve_name_component.h"
#include "ve_component_system.h"
#include "systems/render/engine/units/ve_render_model.h"
#include "systems/render/engine/units/ve_render_directional_light.h"
#include "systems/render/engine/units/ve_render_point_light.h"
#include "systems/render/engine/units/ve_render_spot_light.h"
#include "systems/render/engine/units/ve_render_ambient_light.h"
//----------------------------------------------------------------------------//
class TestComponent : public ve::Component
{
public:
	// Constructors and move assignment operator.
	TestComponent() : str("test"), boolean(true) {}
	TestComponent(TestComponent&&) = default;
	TestComponent& operator =(TestComponent&&) = default;

	// Copying is prohibited.
	TestComponent(const TestComponent&) = delete;
	TestComponent& operator =(const TestComponent&) = delete;

	// Meta routine.
	const ut::DynamicType& Identify() const
	{
		return ut::Identify(this);
	}

	void Reflect(ut::meta::Snapshot& snapshot)
	{
		snapshot.Add(str, "str");
		snapshot.Add(boolean, "bool");
		snapshot.Add(intv8, "intv8");
		snapshot.Add(uintv8, "uintv8");
		snapshot.Add(intv16, "intv16");
		snapshot.Add(uintv16, "uintv16");
		snapshot.Add(intv32, "intv32");
		snapshot.Add(uintv32, "uintv32");
		snapshot.Add(intv64, "intv64");
		snapshot.Add(uintv64, "uintv64");
		snapshot.Add(floatv, "floatv");
		snapshot.Add(ivec, "ivec");
		snapshot.Add(arrint, "arrint");
		snapshot.Add(arrvecint, "arrvecint");
		snapshot.Add(arrvecfloat, "arrvecfloat");
		snapshot.Add(unique, "unique");
	}

	ut::String str;
	bool boolean;
	ut::int8 intv8 = 0;
	ut::uint8 uintv8 = 0;
	ut::int16 intv16 = 0;
	ut::uint16 uintv16 = 0;
	ut::int32 intv32 = 0;
	ut::uint32 uintv32 = 0;
	ut::int32 intv64 = 0;
	ut::uint32 uintv64 = 0;
	float floatv = 1.0f;
	ut::Vector<3, int> ivec = ut::Vector<3, int>();
	ut::Array<int> arrint;
	ut::Array<ut::Vector<3, int> > arrvecint;
	ut::Array<ut::Vector<3, float> > arrvecfloat;
	ut::UniquePtr<int> unique;
};
UT_REGISTER_TYPE(ve::Component, TestComponent, "test")

class TestSystem : public ve::ComponentSystem<TestComponent>
{
public:
	TestSystem() : ve::ComponentSystem<TestComponent>("test_system") {}
	ve::System::Result Update(System::Time, Access& access) override { return ve::CmdArray(); }
};

//----------------------------------------------------------------------------//

// Creates a box with random color and scale.
ut::Array< ut::UniquePtr<ve::Component> > CreateRandomBox(const ut::Vector<3>& position, size_t id)
{
	enum PrimitiveType
	{
		primitive_box,
		primitive_sphere,
		primitive_torus,
	};

	// random values
	const float r1 = static_cast<float>(300 + rand() % 800);
	const float r2 = static_cast<float>(300 + rand() % 800);
	const float r3 = static_cast<float>(300 + rand() % 800);

	PrimitiveType primitive_type = primitive_box;

	if (rand() % 4 == 0)
	{
		primitive_type = primitive_torus;
	}
	else if (rand() % 3 == 0)
	{
		primitive_type = primitive_sphere;
	}

	// transform
	ve::TransformComponent transform_component;
	transform_component.translation = position;
	transform_component.rotation = ut::Quaternion<float>::MakeFromAngleAndAxis(static_cast<float>(rand() % 360),
	                                                                           ut::Vector<3>(0, 1, 0));
	transform_component.scale = ut::Vector<3>(r1 / 500.0f, r2 / 500.0f, r3 / 500.0f);

	// mesh resource name
	ut::String mesh_name = ve::render::engine_rc::skBox;
	if (primitive_type == primitive_sphere)
	{
		mesh_name = ve::render::engine_rc::skSphere;
	}
	else if (primitive_type == primitive_torus)
	{
		mesh_name = ve::render::engine_rc::skTorus;
	}

	// model unit
	ve::render::Model box_model;
	box_model.local_trasform.translation.X() = (rand() % 200) / 40.0f;
	box_model.local_trasform.translation.Y() = (rand() % 200) / 40.0f;
	box_model.local_trasform.translation.Z() = (rand() % 200) / 40.0f;
	box_model.mesh_path = ut::String(ve::render::engine_rc::skDir) + mesh_name;
	box_model.diffuse_mul.R() = r1 / 1050.0f;
	box_model.diffuse_mul.G() = r2 / 1000.0f;
	box_model.diffuse_mul.B() = r3 / 1000.0f;

	const bool is_metallic = rand() % 10 == 0;
	float roughness = static_cast<float>((rand() % 250) + 5) / 255.0f;

	box_model.material_mul.X() = roughness;
	box_model.material_add.Y() = is_metallic ? 1.0f : 0.0f;

	// render component
	ve::RenderComponent render_component;
	render_component.units.Add(ut::MakeUnique<ve::render::Model>(ut::Move(box_model)));

	// entity
	ut::Array< ut::UniquePtr<ve::Component> > box;
	box.Add(ut::MakeUnique<ve::RenderComponent>(ut::Move(render_component)));
	box.Add(ut::MakeUnique<ve::TransformComponent>(ut::Move(transform_component)));
	box.Add(ut::MakeUnique<ve::NameComponent>(mesh_name + ut::Print(id)));
	box.Add(ut::MakeUnique<TestComponent>(TestComponent()));
	return box;
}

// Creates a light source.
ut::Array< ut::UniquePtr<ve::Component> > CreateLight(ve::render::Light::SourceType type,
                                                      const ut::Vector<3>& position,
                                                      size_t id)
{
	ut::String name;

	// transform
	ve::TransformComponent transform;
	transform.translation = position;
	transform.rotation = ut::Quaternion<float>::MakeFromAngles(ut::Vector<3>(0, 0, -60));

	// light source
	ut::UniquePtr<ve::render::Unit> light;
	switch (type)
	{
		case ve::render::Light::source_ambient:
		{
			name = ut::GetPolymorphicName<ve::render::AmbientLight>();
			const ut::Vector<3> light_direction = ut::Vector<3>(-1, -1, 1).Normalize();
			transform.rotation = ut::Quaternion<float>::MakeShortestRotation(ve::render::DirectionalLight::skDirection,
			                                                                 light_direction);
			ve::render::AmbientLight light_unit;
			light_unit.color = ut::Vector<3>(0.75f, 0.95f, 1.0f);
			light_unit.intensity = 0.25f;
			light_unit.attenuation_distance = 50.0f;
			light = ut::MakeUnique<ve::render::AmbientLight>(ut::Move(light_unit));
		} break;

		case ve::render::Light::source_directional:
		{
			name = ut::GetPolymorphicName<ve::render::DirectionalLight>();
			const ut::Vector<3> light_direction = ut::Vector<3>(1, -1, 1).Normalize();
			transform.rotation = ut::Quaternion<float>::MakeShortestRotation(ve::render::DirectionalLight::skDirection,
			                                                                 light_direction);
			ve::render::DirectionalLight light_unit;
			light_unit.color = ut::Vector<3>(1, 1, 1);
			light_unit.intensity = 0.45f;
			light = ut::MakeUnique<ve::render::DirectionalLight>(ut::Move(light_unit));
		} break;

		case ve::render::Light::source_point:
		{
			name = ut::GetPolymorphicName<ve::render::PointLight>();
			ve::render::PointLight light_unit;
			light_unit.color = ut::Vector<3>(1, 1, 1);
			light_unit.intensity = 2.0f;
			light_unit.attenuation_distance = 20.0f;
			light = ut::MakeUnique<ve::render::PointLight>(ut::Move(light_unit));
		} break;

		case ve::render::Light::source_spot:
		{
			name = ut::GetPolymorphicName<ve::render::SpotLight>();
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
	ut::Array< ut::UniquePtr<ve::Component> > light_entity;
	light_entity.Add(ut::MakeUnique<ve::RenderComponent>(ut::Move(render)));
	light_entity.Add(ut::MakeUnique<ve::TransformComponent>(ut::Move(transform)));
	light_entity.Add(ut::MakeUnique<ve::NameComponent>(name + ut::Print(id)));
	return light_entity;
}

// Creates a set of entities for the test scene.
ut::Array< ut::Array< ut::UniquePtr<ve::Component> > > CreateTestScene()
{
	ut::Array< ut::Array< ut::UniquePtr<ve::Component> > > out;

	const float x_offset = 40.0f;

	// ambient light
    size_t ambient_light_id = 0;
	out.Add(CreateLight(ve::render::Light::source_ambient, ut::Vector<3>(x_offset, 0, 0), ambient_light_id++));

	// directional light
	size_t dir_light_id = 0;
	out.Add(CreateLight(ve::render::Light::source_directional, ut::Vector<3>(0), dir_light_id++));

	// point and spot lights
	size_t point_light_id = 0;
	size_t spot_light_id = 0;
	int point_light_count = 2;
	for (int i = -point_light_count; i <= point_light_count; i++)
	{
		const ut::Vector<3> point_position(x_offset - 25.0f, 40.0f, static_cast<float>(i) * 90.0f);
		out.Add(CreateLight(ve::render::Light::source_point, point_position, point_light_id++));

		const ut::Vector<3> spot_position(x_offset - 35.0f, -20.0f, static_cast<float>(i) * 90.0f);
		out.Add(CreateLight(ve::render::Light::source_spot, spot_position, spot_light_id++));
	}

	// boxes
	size_t mesh_id = 0;
	const int nx = 2, ny = 10, nz = 10;
	for (int x = -nx; x < nx; x++)
	{
		for (int y = -ny; y < ny; y++)
		{
			for (int z = -nz; z < nz; z++)
			{
				const ut::Vector<3> position(static_cast<float>(x) * 5.0f + x_offset,
				                             static_cast<float>(y) * 6.0f,
				                             static_cast<float>(z) * 6.0f);
				out.Add(CreateRandomBox(position, mesh_id++));
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
	ve::Pipeline pipeline = ve::GenDefaultPipeline();
	pipeline.AddSerial(ve::Pipeline(ut::MakeShared<TestSystem>()));
	ve::Environment environment(ut::Move(pipeline));

	// create test scene
	ut::Array< ut::Array< ut::UniquePtr<ve::Component> > > entities = CreateTestScene();
	for (size_t i = 0; i < entities.Count(); i++)
	{
		environment.EnqueueCommand(ut::MakeUnique<ve::CmdAddEntity>(ut::Move(entities[i])));
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