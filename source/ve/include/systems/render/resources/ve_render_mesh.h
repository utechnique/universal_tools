//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_resource.h"
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//

class Mesh : public Resource
{
public:
	Mesh(ut::uint32 in_face_count,
	     ut::uint32 in_vertex_count,
		 Buffer in_vertex_buffer,
		 ut::Optional<Buffer> in_index_buffer,
		 IndexType in_index_type,
		 InputAssemblyState input_assembly_state) : face_count(in_face_count)
	                                              , vertex_count(in_vertex_count)
	                                              , vertex_buffer(ut::Move(in_vertex_buffer))
		                                          , index_buffer(ut::Move(in_index_buffer))
		                                          , index_type(in_index_type)
	                                              , input_assembly(ut::Move(input_assembly_state))
	{
		// mesh can be composed only of triangles
		const primitive::Topology topology = input_assembly_state.topology;
		if (topology != primitive::triangle_list &&
		    topology != primitive::triangle_list_with_adjacency &&
		    topology != primitive::patch_list)
		{
			throw ut::Error(ut::error::invalid_arg, "Mesh doesn\'t support non-triangle topology.");
		}
	}

	// number of vertices in one face
	static constexpr ut::uint32 skPolygonVertices = 3;

	ut::uint32 face_count;
	ut::uint32 vertex_count;
	Buffer vertex_buffer;
	ut::Optional<Buffer> index_buffer;
	IndexType index_type;
	InputAssemblyState input_assembly;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//