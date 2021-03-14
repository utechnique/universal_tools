//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_unit.h"
#include "systems/render/ve_render_api.h"
#include "systems/render/resources/ve_render_mesh.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Model is a compact general-purpose unit.
class Model : public Unit
{
public:
	// Explicitly declare defaulted constructors and move operator.
	Model() = default;
	Model(Model&&) = default;
	Model& operator =(Model&&) = default;

	// Copying is prohibited.
	Model(const Model&) = delete;
	Model& operator =(const Model&) = delete;

	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	RcRef<Mesh> mesh;
	ut::String name;

	ut::Color<3> diffuse_add = ut::Color<3>(0);
	ut::Color<3> diffuse_mul = ut::Color<3>(1);
	ut::Color<3> material_add = ut::Color<3>(0);
	ut::Color<3> material_mul = ut::Color<3>(1);

	// It makes sense to store shader uniforms in one big buffer instead of
	// many small. It saves time on updating buffers.
	struct Batch
	{
		Buffer transform;
		Buffer material;
	};

	struct DrawCall
	{
		Model& model;
		Entity::Id entity_id;
		ut::uint32 subset_id;
	};

	struct TransformBuffer
	{
		ut::Matrix<4, 4> transform;
	};

	struct MaterialBuffer
	{
		ut::Vector<4> diffuse_add;
		ut::Vector<4> diffuse_mul;
		ut::Vector<4> material_add;
		ut::Vector<4> material_mul;
	};
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//