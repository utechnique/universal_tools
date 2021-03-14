//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::vertex_traits namespace contains helper templates describing
// possible vertex components such as position, texture coordinates, etc.
namespace vertex_traits
{
	enum ComponentType : ut::uint32
	{
		position,
		texcoord,
		normal,
		tangent,
		bone_weight,
		bone_id,
		component_count
	};

	template<ComponentType, typename S = float, int d = 1> struct Component
	{
		static_assert(sizeof(S) != 0, "Vertex component must have specialization.");
	};

	template<typename S, int d> struct Component<position, S, d>
	{
		ut::Vector<d, S> position;
		static const char* GetName() { return "POSITION"; }
	};

	template<typename S, int d> struct Component<texcoord, S, d>
	{
		ut::Vector<d, S> texcoord;
		static const char* GetName() { return "TEXCOORD"; }
	};

	template<typename S, int d> struct Component<normal, S, d>
	{
		ut::Vector<d, S> normal;
		static const char* GetName() { return "NORMAL"; }
	};

	template<typename S, int d> struct Component<tangent, S, d>
	{
		ut::Vector<d, S> tangent;
		static const char* GetName() { return "TANGENT"; }
	};

	template<typename S, int d> struct Component<bone_weight, S, d>
	{
		ut::Vector<d, S> bone_weight;
		static const char* GetName() { return "WEIGHTS"; }
	};

	template<typename S, int d> struct Component<bone_id, S, d>
	{
		ut::Vector<d, S> bone_id;
		static const char* GetName() { return "BONES"; }
	};

	template<typename S, int d>
	pixel::Format GenerateFormat()
	{
		static_assert(sizeof(S) != 0, "Unsupported vertex vector component format.");
		return pixel::unknown;
	}

	template<> inline pixel::Format GenerateFormat<ut::int32, 1>() { return pixel::r32_sint; }
	template<> inline pixel::Format GenerateFormat<ut::int32, 2>() { return pixel::r32g32_sint; }
	template<> inline pixel::Format GenerateFormat<ut::int32, 3>() { return pixel::r32g32b32_sint; }
	template<> inline pixel::Format GenerateFormat<ut::int32, 4>() { return pixel::r32g32b32a32_sint; }
	template<> inline pixel::Format GenerateFormat<ut::uint32, 1>() { return pixel::r32_uint; }
	template<> inline pixel::Format GenerateFormat<ut::uint32, 2>() { return pixel::r32g32_uint; }
	template<> inline pixel::Format GenerateFormat<ut::uint32, 3>() { return pixel::r32g32b32_uint; }
	template<> inline pixel::Format GenerateFormat<ut::uint32, 4>() { return pixel::r32g32b32a32_uint; }
	template<> inline pixel::Format GenerateFormat<float, 1>() { return pixel::r32_float; }
	template<> inline pixel::Format GenerateFormat<float, 2>() { return pixel::r32g32_float; }
	template<> inline pixel::Format GenerateFormat<float, 3>() { return pixel::r32g32b32_float; }
	template<> inline pixel::Format GenerateFormat<float, 4>() { return pixel::r32g32b32a32_float; }

	// Returns a reference to the VertexElement object if desired
    // component is present in the provided input assembly state.
    template<ComponentType id>
    static ut::Optional<const VertexElement&> MapComponent(const InputAssemblyState& ias)
    {
        const size_t element_count = ias.elements.GetNum();
        for (size_t i = 0; i < element_count; i++)
        {
            const VertexElement& element = ias.elements[i];
            if (element.semantic_name == Component<id>::GetName())
            {
                return element;
            }
        }
        return ut::Optional<const VertexElement&>();
    }

	// Helper template structure to iterate components.
	template<ut::uint32 id> struct ComponentIterator
	{
		// Recursively initializes a map where ComponentType is a key, and
		// ut::Optional<const VertexElement&> is a value.
		static void Map(ut::Optional<const VertexElement&>* map,
		                const InputAssemblyState& ias)
		{
			map[id] = MapComponent<static_cast<ComponentType>(id)>(ias);
			ComponentIterator<id - 1>::Map(map, ias);
		}
	};

	// Specialization for the last component.
	template<> struct ComponentIterator<0>
	{
		// Initializes last map element.
		static void Map(ut::Optional<const VertexElement&>* map,
		                const InputAssemblyState& ias)
		{
			map[0] = MapComponent<static_cast<vertex_traits::ComponentType>(0)>(ias);
		}
	};
}

//----------------------------------------------------------------------------//
// Universal vertex type that contains actual vertex type (Vertex<>::Type) and
template<typename PositionType = void, int position_dim = 0,
         typename TexcoordType = void, int texcoord_dim = 0,
         typename NormalType   = void, int normal_dim   = 0,
         typename TangentType  = void, int tangent_dim  = 0,
         typename WeightsType  = void, int weights_dim  = 0,
         typename BonesType    = void, int bones_dim    = 0>
struct Vertex
{
	// vector trait wrapper
	template<vertex_traits::ComponentType component, typename Scalar, int dim>
	struct VectorTrait : public vertex_traits::Component<component, Scalar, dim>
	{
		static ut::Optional<pixel::Format> GetFormat()
		{
			return vertex_traits::GenerateFormat<Scalar, dim>();
		}
	};

	template<vertex_traits::ComponentType component, typename Scalar>
	struct VectorTrait<component, Scalar, 0>
	{
		static const char* GetName() { return nullptr; }
		static ut::Optional<pixel::Format> GetFormat()
		{
			return ut::Optional<pixel::Format>();
		}
	};

	// collection of all traits
	typedef ut::Container<
		VectorTrait<vertex_traits::position, PositionType, position_dim>,
		VectorTrait<vertex_traits::texcoord, TexcoordType, texcoord_dim>,
		VectorTrait<vertex_traits::normal, NormalType, normal_dim>,
		VectorTrait<vertex_traits::tangent, TangentType, tangent_dim>,
		VectorTrait<vertex_traits::bone_weight, WeightsType, weights_dim>,
		VectorTrait<vertex_traits::bone_id, BonesType, bones_dim>
	> Traits;
	static_assert(static_cast<ut::uint32>(Traits::size) == static_cast<ut::uint32>(vertex_traits::component_count),
		"All components from ve::render::vertex_traits enumerations must be "
		"included in ve::render::Vertex::Traits container.");

	// helper template to iterate traits
	template<typename Trait, int trait_id>
	struct TraitAccumulator : public Trait::template Item<Trait::size - trait_id>::Type
	                        , public TraitAccumulator<Trait, trait_id - 1>
	{
		typedef typename Trait::template Item<trait_id - 1>::Type TraitType;
		typedef TraitAccumulator<Trait, trait_id - 1> Base;

		// Recursively adds vertex elements to array.
		static ut::Array<VertexElement> CreateLayout()
		{
			ut::Array<VertexElement> layout = Base::CreateLayout();

			ut::Optional<pixel::Format> format = TraitType::GetFormat();
			if (format)
			{
				ut::uint32 offset = 0;
				if (!layout.IsEmpty())
				{
					const VertexElement& prev = layout.GetLast();
					offset = prev.offset + pixel::GetSize(prev.format);
				}

				layout.Add(VertexElement(TraitType::GetName(), format.Get(), offset));
			}

			return layout;
		}
	};

	template<typename Trait> struct TraitAccumulator<Trait, 0>
	{
		static ut::Array<VertexElement> CreateLayout()
		{
			return ut::Array<VertexElement>();
		}
	};

	// final vertex type
	struct Type : public TraitAccumulator<Traits, Traits::size>
	{};

	// creates an array of vertex elements
	static ut::Array<VertexElement> CreateLayout()
	{
		return Type::CreateLayout();
	}

	// size of this vertex in bytes
	static constexpr ut::uint32 size = sizeof(Type);
};

//----------------------------------------------------------------------------//
// ve::render::VertexReflector is an interface to operate with a vertex buffer
// without knowing the exact vertex type. It makes possible read and write
// vertex components such as position, texcoord, normal, etc. One can read/write
// components that are missing from a vertex and without conserning about its
// type (float, byte, etc).
class VertexReflector
{
public:
	// Constructor.
	//    @param in_input_assembly - const reference to the input assembly
	//                               state associated with a vertex data to
	//                               be reflected.
	//    @param in_buffer - memory address with vertices to be reflected.
	VertexReflector(const InputAssemblyState& in_input_assembly,
	                void* in_buffer);

	// An interface to read/write component data to the desired vertex.
	struct Component
	{
		// Constructor.
		//    @param info - a reference to the vertex element associated
		//                  with this component.
		//    @param vertex_data - pointer to the vertex.
		Component(const ut::Optional<const VertexElement&>& info,
		          ut::byte* vertex_data);

		// Writes individual channels (dimensions) to the vertex component.
		// All arguments will be converted to the appropriate type.
		// Example: call Write(x, y, z) to write 3d vector.
		template<typename... Scalars>
		void Write(Scalars... scalars)
		{
			if (!valid)
			{
				return;
			}

			void* arg_values[]{ static_cast<void*>(&scalars)... };
			ut::TypeId arg_types[]{ ut::Type<Scalars>::Id()... };
			const size_t arg_count = sizeof(arg_values) / sizeof(void*);

			ut::byte* channel = component_data;
			for (ut::uint32 i = 0; i < channel_count; i++)
			{
				if (i < arg_count)
				{
					ut::Convert(arg_values[i], arg_types[i], channel, channel_type);
				}
				else
				{
					ut::memory::Set(channel, 0, channel_size);
				}
				channel += channel_size;
			}
		}

		// Writes a vector to the vertex component. Value
		// will be converted to the appropriate type.
		template<ut::MatrixElementId dim, typename Scalar>
		void Write(const ut::Vector<dim, Scalar>& vector)
		{
			if (!valid)
			{
				return;
			}

			ut::byte* channel = component_data;
			for (ut::uint32 i = 0; i < channel_count; i++)
			{
				if (i < dim)
				{
					ut::Convert(vector.GetData() + i,
					            ut::Type<Scalar>::Id(),
					            channel,
					            channel_type);
				}
				else
				{
					ut::memory::Set(channel, 0, channel_size);
				}
				channel += channel_size;
			}
		}

		// Returns a vector that is a converted copy of the
		// vertex component.
		template<ut::MatrixElementId dim, typename Scalar>
		ut::Vector<dim, Scalar> Read()
		{
			if (!valid)
			{
				return ut::Vector<dim, Scalar>(0);
			}

			ut::Vector<dim, Scalar> out(0);
			ut::byte* channel = component_data;
			const ut::uint32 components_to_read = ut::Min(dim, channel_count);

			for (ut::uint32 i = 0; i < components_to_read; i++)
			{
				ut::Convert(channel,
				            channel_type,
				            out.GetData() + i,
				            ut::Type<Scalar>::Id());
				channel += channel_size;
			}

			return out;
		}

		// Indicates if managed vertex has desired component.
		bool valid;

		// Type of a component channel, for example this value
		// will be 'float' for r32g32_float format.
		ut::TypeId channel_type;

		// Number of channels for this component.
		// Example: channel count == 2 for r32g32_float format.
		ut::uint32 channel_count;

		// Size of the individual channel.
		// Example: channel size == 4 for r32g32_float format.
		ut::uint32 channel_size;

		// Address of the component.
		ut::byte* component_data;
	};

	// Returns an interface that is capable to read/write
	// component data of the desired vertex.
	template<vertex_traits::ComponentType component>
	Component Get(size_t vertex_id)
	{
		ut::byte* out = static_cast<ut::byte*>(buffer);
		return Component(component_map[component],
		                 out + vertex_id * input_assembly.vertex_stride);
	}

private:
	ut::Optional<const VertexElement&> component_map[vertex_traits::component_count];
	InputAssemblyState input_assembly;
	ut::byte* buffer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
