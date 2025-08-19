//----------------------------------------------------------------------------//
//-----------------------------|  C R A D L E  |----------------------------- //
//----------------------------------------------------------------------------//
#include "cradle_env_gen.h"
#include "components/ve_render_component.h"
#include "components/ve_transform_component.h"
#include "components/ve_name_component.h"
#include "systems/render/engine/units/ve_render_mesh_instance.h"
#include "systems/render/engine/units/ve_render_directional_light.h"
#include "systems/render/engine/units/ve_render_point_light.h"
#include "systems/render/engine/units/ve_render_spot_light.h"
#include "systems/render/engine/units/ve_render_ambient_light.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(cradle)
//----------------------------------------------------------------------------//
// Creates a box with random color and scale.
ut::Array< ut::UniquePtr<ve::Component> > CreateRandomBox(const ut::Vector<3>& position, size_t id)
{
	enum class PrimitiveType
	{
		box,
		sphere,
		torus,
	};

	// random values
	const float r1 = static_cast<float>(300 + rand() % 800);
	const float r2 = static_cast<float>(300 + rand() % 800);
	const float r3 = static_cast<float>(300 + rand() % 800);

	PrimitiveType primitive_type = PrimitiveType::box;

	if (rand() % 4 == 0)
	{
		primitive_type = PrimitiveType::torus;
	}
	else if (rand() % 3 == 0)
	{
		primitive_type = PrimitiveType::sphere;
	}

	// transform
	ve::TransformComponent transform_component;
	transform_component.translation = position;
	transform_component.rotation = ut::Quaternion<float>::MakeFromAngleAndAxis(static_cast<float>(rand() % 360),
	                                                                           ut::Vector<3>(0, 1, 0));
	transform_component.scale = ut::Vector<3>(r1 / 500.0f, r2 / 500.0f, r3 / 500.0f);

	// mesh resource name
	const ut::String gen_starter = ve::render::Resource::GeneratorPrompt::skStarter;
	ut::String mesh_name = ve::render::ResourceCreator<ve::render::Mesh>::skTypeBox;
	if (primitive_type == PrimitiveType::sphere)
	{
		mesh_name = ve::render::ResourceCreator<ve::render::Mesh>::skTypeSphere;
	}
	else if (primitive_type == PrimitiveType::torus)
	{
		mesh_name = ve::render::ResourceCreator<ve::render::Mesh>::skTypeTorus;
	}

	// mesh unit
	ve::render::MeshInstance mesh_instance;
	mesh_instance.local_trasform.translation.X() = (rand() % 200) / 40.0f;
	mesh_instance.local_trasform.translation.Y() = (rand() % 200) / 40.0f;
	mesh_instance.local_trasform.translation.Z() = (rand() % 200) / 40.0f;
	mesh_instance.mesh_path = gen_starter + mesh_name;
	mesh_instance.diffuse_mul.R() = r1 / 1050.0f;
	mesh_instance.diffuse_mul.G() = r2 / 1000.0f;
	mesh_instance.diffuse_mul.B() = r3 / 1000.0f;

	const bool is_metallic = rand() % 10 == 0;
	float roughness = static_cast<float>((rand() % 250) + 5) / 255.0f;

	mesh_instance.material_mul.X() = roughness;
	mesh_instance.material_add.Y() = is_metallic ? 1.0f : 0.0f;

	// render component
	ve::RenderComponent render_component;
	render_component.units.Add(ut::MakeUnique<ve::render::MeshInstance>(ut::Move(mesh_instance)));

	// entity
	ut::Array< ut::UniquePtr<ve::Component> > box;
	box.Add(ut::MakeUnique<ve::RenderComponent>(ut::Move(render_component)));
	box.Add(ut::MakeUnique<ve::TransformComponent>(ut::Move(transform_component)));
	box.Add(ut::MakeUnique<ve::NameComponent>(mesh_name + ut::Print(id)));
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
		case ve::render::Light::SourceType::ambient:
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

		case ve::render::Light::SourceType::directional:
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

		case ve::render::Light::SourceType::point:
		{
			name = ut::GetPolymorphicName<ve::render::PointLight>();
			ve::render::PointLight light_unit;
			light_unit.color = ut::Vector<3>(1, 1, 1);
			light_unit.intensity = 2.0f;
			light_unit.attenuation_distance = 20.0f;
			light = ut::MakeUnique<ve::render::PointLight>(ut::Move(light_unit));
		} break;

		case ve::render::Light::SourceType::spot:
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

ut::Array< ut::Array< ut::UniquePtr<ve::Component> > > GenerateEnvironment()
{
	ut::Array< ut::Array< ut::UniquePtr<ve::Component> > > out;

	const float x_offset = 40.0f;

	// ambient light
	size_t ambient_light_id = 0;
	out.Add(CreateLight(ve::render::Light::SourceType::ambient, ut::Vector<3>(x_offset, 0, 0), ambient_light_id++));

	// directional light
	size_t dir_light_id = 0;
	out.Add(CreateLight(ve::render::Light::SourceType::directional, ut::Vector<3>(0), dir_light_id++));

	// point and spot lights
	size_t point_light_id = 0;
	size_t spot_light_id = 0;
	int point_light_count = 2;
	for (int i = -point_light_count; i <= point_light_count; i++)
	{
		const ut::Vector<3> point_position(x_offset - 25.0f, 40.0f, static_cast<float>(i) * 90.0f);
		out.Add(CreateLight(ve::render::Light::SourceType::point, point_position, point_light_id++));

		const ut::Vector<3> spot_position(x_offset - 35.0f, -20.0f, static_cast<float>(i) * 90.0f);
		out.Add(CreateLight(ve::render::Light::SourceType::spot, spot_position, spot_light_id++));
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
END_NAMESPACE(cradle)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//