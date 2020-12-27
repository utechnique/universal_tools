//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_vertex_factory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
//    @param in_input_assembly - const reference to the input assembly
//                               state associated with a vertex data to
//                               be reflected.
//    @param in_buffer - memory address with vertices to be reflected.
VertexReflector::VertexReflector(const InputAssemblyState& in_input_assembly,
	                             void* in_buffer) : input_assembly(in_input_assembly)
	                                              , buffer(static_cast<ut::byte*>(in_buffer))
{
	constexpr ut::uint32 last_component = vertex_traits::component_count - 1;
	vertex_traits::ComponentIterator<last_component>::Map(component_map, input_assembly);
}

//----------------------------------------------------------------------------->
// Constructor.
//    @param info - a reference to the vertex element associated
//                  with this component.
//    @param vertex_data - pointer to the vertex.
VertexReflector::Component::Component(const ut::Optional<const VertexElement&>& info,
                                      ut::byte* vertex_data) : component_data(vertex_data), valid(false)
{
	// check if desired component exists
	if (!info)
	{
		return;
	}

	// check if a component has at least 1 scalar element
	channel_count = pixel::GetChannelCount(info->format);
	if (channel_count == 0)
	{
		ut::log.Lock() << "Warning! Invalid vertex component dimension." << ut::cret;
		return;
	}

	// check if a component has valid size
	ut::uint32 component_size = pixel::GetSize(info->format);
	if (component_size == 0)
	{
		ut::log.Lock() << "Warning! Invalid vertex component size." << ut::cret;
		return;
	}

	// check if a component has scalar type supported by ut
	channel_type = pixel::GetChannelType(info->format);
	if (channel_type == 0)
	{
		ut::log.Lock() << "Warning! Unsupported vertex component format." << ut::cret;
		return;
	}

	// calculate size of the channel
	channel_size = component_size / channel_count;

	// calculate the address of the desired vertex element
	component_data += info->offset;

	// everything is correct
	valid = true;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//