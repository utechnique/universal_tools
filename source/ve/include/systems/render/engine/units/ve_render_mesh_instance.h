//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit.h"
#include "systems/render/engine/resources/ve_render_mesh.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::MeshInstance is a compact general-purpose render unit to draw
// vertex mesh.
class MeshInstance : public Unit
{
public:
	// It makes sense to store shader uniforms in one big buffer instead of
	// many small. It saves time on updating buffers.
	struct Batch
	{
		Buffer transform;
		Buffer material;
		Buffer entity_id;
	};

	// Engine divides a mesh instance unit into pieces that can be rendered 
	// using only one drawcall.
	struct DrawCall
	{
		MeshInstance& instance;
		Entity::Id entity_id;
		ut::uint32 subset_id;
	};

	// CPU representation of the transform uniform buffer.
	struct TransformBuffer
	{
		ut::Matrix<4, 4> transform;
	};

	// CPU representation of the material uniform buffer.
	struct MaterialBuffer
	{
		ut::Vector<4> diffuse_add;
		ut::Vector<4> diffuse_mul;
		ut::Vector<4> material_add;
		ut::Vector<4> material_mul;
	};

	// CPU representation of the entity-id buffer.
	struct EntityIdBuffer
	{
		typedef ut::Vector<4> Type;
		alignas(16) Type entity_id;
	};

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Registers this mesh instance unit into the reflection tree.
	//    @param snapshot - reference to the reflection tree.
	void Reflect(ut::meta::Snapshot& snapshot);

	// Reference to the mesh asset.
	RcRef<Mesh> mesh;

	// The resource name of the mesh asset. It is needed for the render engine
	// to be able to initialize @mesh reference.
	ut::String mesh_path = ut::String(Resource::GeneratorPrompt::skStarter) +
	                       ResourceCreator<Mesh>::skTypeBox;

	// Color that is added to the diffuse component of all mesh materials.
	ut::Color<3> diffuse_add = ut::Color<3>(0);

	// Color that is multiplied with the diffuse
	// component of all mesh materials.
	ut::Color<3> diffuse_mul = ut::Color<3>(1);

	// Color that is added to the material component of all mesh materials.
	ut::Color<3> material_add = ut::Color<3>(0);

	// Color that is multiplied with the material
	// component of all mesh materials.
	ut::Color<3> material_mul = ut::Color<3>(1);

	// Determines if this mesh instance has separate stencil mask bit enabled.
	bool highlighted = false;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//