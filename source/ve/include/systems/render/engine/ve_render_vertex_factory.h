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
	// vectoral components
	namespace vector
	{
		enum ComponentType : ut::uint32
		{
			position,
			texcoord,
			normal,
			tangent,
			component_count
		};

		template<ComponentType, typename S, int d> struct Component
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

		template<typename S, int d>
		pixel::Format GenerateFormat()
		{
			static_assert(sizeof(S) != 0, "Unsupported vertex vector component format.");
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
	}

	// scalar components
	namespace scalar
	{
		enum ComponentType : ut::uint32
		{
			displacement,
			component_count
		};

		template<ComponentType, typename S> struct Component
		{
			static_assert(sizeof(S) != 0, "Vertex component must have specialization.");
		};

		template<typename S> struct Component<displacement, S>
		{
			S displacement;
			static const char* GetName() { return "DISPLACEMENT"; }
		};

		template<typename S>
		pixel::Format GenerateFormat()
		{
			static_assert(sizeof(S) != 0, "Unsupported vertex scalar component format.");
		}

		template<> inline pixel::Format GenerateFormat<ut::int32>() { return pixel::r32_sint; }
		template<> inline pixel::Format GenerateFormat<ut::uint32>() { return pixel::r32_uint; }
		template<> inline pixel::Format GenerateFormat<float>() { return pixel::r32_float; }
	}
}

// Universal vertex type that contains actual vertex type (Vertex<>::Type) and
template<typename PositionType = void, int position_dim = 0,
         typename TexcoordType = void, int texcoord_dim = 0,
         typename NormalType   = void, int normal_dim = 0,
         typename TangentType  = void, int tangent_dim = 0,
         typename DisplacementType = void>
struct Vertex
{
	// vector trait wrapper
	template<vertex_traits::vector::ComponentType component, typename Scalar, int dim>
	struct VectorTrait : public vertex_traits::vector::Component<component, Scalar, dim>
	{
		static ut::Optional<pixel::Format> GetFormat()
		{
			return vertex_traits::vector::GenerateFormat<Scalar, dim>();
		}
	};

	template<vertex_traits::vector::ComponentType component, typename Scalar>
	struct VectorTrait<component, Scalar, 0>
	{
		static const char* GetName() { return nullptr; }
		static ut::Optional<pixel::Format> GetFormat()
		{
			return ut::Optional<pixel::Format>();
		}
	};

	// scalar trait wrapper
	template<vertex_traits::scalar::ComponentType component, typename Scalar>
	struct ScalarTrait : public vertex_traits::scalar::Component<component, Scalar>
	{
		static ut::Optional<pixel::Format> GetFormat()
		{
			return vertex_traits::scalar::GenerateFormat<Scalar>();
		}
	};

	template<vertex_traits::scalar::ComponentType component>
	struct ScalarTrait<component, void>
	{
		static const char* GetName() { return nullptr; }
		static ut::Optional<pixel::Format> GetFormat()
		{
			return ut::Optional<pixel::Format>();
		}
	};

	// collection of all traits
	typedef ut::Container<
		VectorTrait<vertex_traits::vector::position, PositionType, position_dim>,
		VectorTrait<vertex_traits::vector::texcoord, TexcoordType, texcoord_dim>,
		VectorTrait<vertex_traits::vector::normal, NormalType, normal_dim>,
		VectorTrait<vertex_traits::vector::tangent, TangentType, tangent_dim>,
		ScalarTrait<vertex_traits::scalar::displacement, DisplacementType>
	> Traits;
	static_assert(Traits::size == vertex_traits::vector::component_count + vertex_traits::scalar::component_count,
		"All components from ve::render::vertex_traits enumerations must be "
		"included in ve::render::Vertex::Vector/ScalarTraits container.");

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
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
