//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "preprocessor/ut_enum.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
#if CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// Contains the actual value for one item in the container. The  template
// parameter `leaf_id` allows the `Get` function to find the value in O(1) time
template<int leaf_id, typename Item>
class ContainerLeaf
{
public:
	ContainerLeaf() {}
	ContainerLeaf(Item&& item) : value(Forward<Item>(item)) {}
	Item value;
};

// ContainerImpl is a proxy for the final class that has an extra 
// template parameter `leaf_id`.
template<int leaf_id, typename... Items> class ContainerImpl;

// Base case: empty container
template<int leaf_id> class ContainerImpl<leaf_id> {};

// Obtains a reference to i-th item in a container
template<int leaf_id, typename HeadItem, typename... TailItems>
inline HeadItem& Get(ContainerImpl<leaf_id, HeadItem, TailItems...>& container)
{
	// Fully qualified name for the member, to find the right one 
	// (they are all called `value`).
	return container.ContainerLeaf<leaf_id, HeadItem>::value;
}

// Helper structures to get a type or id of the value inside a container
// knowing it's index `leaf_id` or type `Desired`
namespace container_helper
{
	template<int leaf_id, typename HeadItem, typename... TailItems>
	struct TypeExtractor
	{
		typedef typename TypeExtractor<leaf_id - 1, TailItems...>::Type Type;
	};

	template<typename HeadItem, typename... TailItems>
	struct TypeExtractor<0, HeadItem, TailItems...>
	{
		typedef HeadItem Type;
	};

	template<int leaf_id, typename Desired, typename HeadItem, typename... TailItems>
	struct IdExtractor
	{
		enum { id = IdExtractor<leaf_id + 1, Desired, TailItems...>::id };
	};

	template<int leaf_id, typename Desired, typename... TailItems>
	struct IdExtractor<leaf_id, Desired, Desired, TailItems...>
	{
		enum { id = leaf_id };
	};

	template<int leaf_id, typename HeadItem, typename... TailItems>
	struct Counter
	{
		enum { count = Counter<leaf_id + 1, TailItems...>::count };
	};

	template<int leaf_id, typename HeadItem>
	struct Counter<leaf_id, HeadItem>
	{
		enum { count = leaf_id + 1 };
	};
}

// Recursive specialization for the container template
template<int i, typename HeadItem, typename... TailItems>
class ContainerImpl<i, HeadItem, TailItems...> : public ContainerLeaf<i, HeadItem>,
	public ContainerImpl<i + 1, TailItems...>
{
private:
	// Helper alias template to conveniently get type
	// of the value in a specified leaf
	template<int leaf_id>
	using LeafType = typename container_helper::TypeExtractor<leaf_id, HeadItem, TailItems...>::Type;

	// Helper function to get id of the value with the specified type
	// at compile time
	template<typename ValueType>
	static constexpr int GetLeafId()
	{
		return container_helper::IdExtractor<0, ValueType, HeadItem, TailItems...>::id;
	}

public:
	// Item type can be accessed using this template
	template<int leaf_id>
	struct Item
	{
		typedef LeafType<leaf_id> Type;
	};

	// Default constructor
	ContainerImpl()
	{ }

	// Constructor, all items must be passed here
	ContainerImpl(HeadItem head,
		TailItems... tail) : ContainerLeaf<i, HeadItem>(Forward<HeadItem>(head))
		, ContainerImpl<i + 1, TailItems...>(Forward<TailItems>(tail) ...)
	{ }

	// Returns reference to the value inside a leaf
	template<int value_id>
	LeafType<value_id>& Get()
	{
		return this->ContainerLeaf<value_id, LeafType<value_id> >::value;
	}

	// Returns const reference to the value inside a leaf, that is specified by `value_id`
	template<int value_id>
	const LeafType<value_id>& Get() const
	{
		return this->ContainerLeaf<value_id, LeafType<value_id> >::value;
	}

	// Returns reference to the first occurence of the `ValueType` item
	template<typename ValueType>
	LeafType<GetLeafId<ValueType>()>& Get()
	{
		return Get<GetLeafId<ValueType>()>();
	}

	// Returns const reference to the first occurence of the `ValueType` item
	template<typename ValueType>
	const LeafType<GetLeafId<ValueType>()>& Get() const
	{
		return Get<GetLeafId<ValueType>()>();
	}

	// Count of items inside a container
	enum { size = container_helper::Counter<0, HeadItem, TailItems...>::count };
};

// Templated alias to avoid having to specify `leaf_id = 0`
template<typename... Items>
using Container = ContainerImpl<0, Items...>;

//----------------------------------------------------------------------------//
#else // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// Maximum possible number of elements in container template
// minus one argument for void type.
#define UT_CONTAINER_MAX 10

//----------------------------------------------------------------------------//
// Some macros to declare all specialized versions of the ut::Container
// template using UT_PP_ENUM() macro. Here in comments 'j' is an Id.
// 1: , Tj
#define UT_CONTAINER_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) T##id
// 2: , Tj (except first one)
#define UT_CONTAINER_TYPENAME_ITERATOR_SKIP_I(id) \
	UT_PP_IF(id, UT_CONTAINER_TYPENAME_ITERATOR, UT_PP_EMPTY_I)(id)
// 3: , typename Tj
#define UT_CONTAINER_FULL_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename T##id
// 4: , typename Tj (except first one)
#define UT_CONTAINER_FULL_TYPENAME_ITERATOR_SKIP_I(id) \
	UT_PP_IF(id, UT_CONTAINER_FULL_TYPENAME_ITERATOR, UT_PP_EMPTY_I)(id)
// 5: , typename Tj = void
#define UT_CONTAINER_FULL_VOID_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename T##id = void
// 6: , Tj ij
#define UT_CONTAINER_INPUT_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) T##id i##id
// 7: , vn(ij)
#define UT_CONTAINER_INPUT_ARG_INIT_ITERATOR(id) UT_PP_COMMA_IF(id) v##id(Forward<T##id>(i##id))
// 8: Tj vj;
#define UT_CONTAINER_DECLARE_TYPEDEF(id) typedef T##id Type##id;
// 9: typedef Tj Typej;
#define UT_CONTAINER_DECLARE_VAR(id) T##id v##id;
// 10: 0, ValueType, T0, T1, ... Tj
#define UT_CONTAINER_EXT_ARGS(id) \
	0, ValueType UT_PP_COMMA_IF(id) UT_PP_ENUM_IN(id, UT_CONTAINER_TYPENAME_ITERATOR)
// 11: , Tj
#define UT_CONTAINER_DEF_CONSTRUCTOR() Container() {}
// 12: , Tj (except first one)
#define UT_CONTAINER_DEF_CONSTRUCTOR_SKIP_FIRST(id) \
	UT_PP_IF(id, UT_CONTAINER_DEF_CONSTRUCTOR, UT_PP_EMPTY)()
// 13: Value extractor
#define UT_CONTAINER_DECLARE_EXTRACTOR(id)                                             \
    template <typename CT> struct TypeExtractor<id, CT>                                \
    {                                                                                  \
        typedef typename CT::Type##id Type;                                            \
        typedef typename RemoveReference<typename CT::Type##id>::Type& Ref;            \
        typedef const typename RemoveReference<typename CT::Type##id>::Type& ConstRef; \
        static Ref Apply(CT* c)                                                        \
        { return c->v##id; }                                                           \
        static ConstRef Apply(const CT* c)                                             \
        { return c->v##id; }                                                           \
    };

//----------------------------------------------------------------------------//
// ut::Container is a wrapper template struct to contain variable number of
// arguments. Here is declared base template. Lower in this file it will be
// specialized according to the number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename T0 = void, typename T1 = void ...>
// struct Container
// {
// };
template<UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_FULL_VOID_TYPENAME_ITERATOR)>
struct Container {};

//----------------------------------------------------------------------------//
// Helper structures to get a type or id of the value inside a container
// knowing it's index `leaf_id` or type `Desired`
namespace container_helper
{
	template <int value_id, typename ContainerType> struct TypeExtractor {};
	UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_DECLARE_EXTRACTOR)

		template<
		int leaf_id,
		typename Desired,
		UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_FULL_VOID_TYPENAME_ITERATOR)
		>
		struct IdExtractor
	{
		enum
		{
			id = IdExtractor<
			leaf_id + 1,
			Desired
			UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_TYPENAME_ITERATOR_SKIP_I),
			void
			>::id
		};
	};

	template<
		int leaf_id,
		typename Desired
		UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_FULL_TYPENAME_ITERATOR_SKIP_I)
	>
		struct IdExtractor<
		leaf_id,
		Desired,
		Desired
		UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_TYPENAME_ITERATOR_SKIP_I)
		>
	{
		enum { id = leaf_id };
	};
}

//----------------------------------------------------------------------------//
// Macro for declaring specialized versions of the ut::Container template.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename T0, typename T1, ... typename Tj>
// struct Container<T0, T1, ... T(j + 1)>
// {
//     Container() {}
//     Container(T0 i0, T1 i1, ... Tj ij) : v0(i0), v1(i1), ... vj(ij) {}
//     
//     template <int v_id>
//     typename container_helper::Extractor<v_id, Container>::Ref Get()
//     {
//         return container_helper::Extractor<v_id, Container>::Apply(this);
//     }
//     
//     template <int v_id>
//     typename container_helper::Extractor<v_id, Container>::ConstRef Get() const
//     {
//         return container_helper::Extractor<v_id, Container>::Apply(this);
//     }
//
//     template <typename ValueType>
//     typename container_helper::TypeExtractor<
//         container_helper::IdExtractor<0, ValueType, T0, T1 ... Tj>::id, Container
//     >::Ref Get()
//     {
//         return container_helper::TypeExtractor<
//             container_helper::IdExtractor<0, ValueType, T0, T1 ... Tj>::id, Container
//         >::Apply(this);
//     }
//
//     template <typename ValueType>
//     typename container_helper::TypeExtractor<
//         container_helper::IdExtractor<0, ValueType, T0, T1 ... Tj>::id, Container
//     >::ConstRef Get() const
//     {
//         return container_helper::TypeExtractor<
//             container_helper::IdExtractor<0, ValueType, T0, T1 ... Tj>::id, Container
//         >::Apply(this);
//     }
//
//     T0 v0;
//     T1 v1;
//     ...
//     Tj vj;
//
//     enum { size = j + 1 };
// };
#define UT_CONTAINER_SPECIALIZATION(items_count)                                         \
template <UT_PP_ENUM_IN(items_count, UT_CONTAINER_FULL_TYPENAME_ITERATOR)>               \
struct Container<UT_PP_ENUM_IN(items_count, UT_CONTAINER_TYPENAME_ITERATOR)>             \
{                                                                                        \
  UT_CONTAINER_DEF_CONSTRUCTOR_SKIP_FIRST(items_count)                                   \
  Container(                                                                             \
    UT_PP_ENUM_IN(items_count, UT_CONTAINER_INPUT_ARG_ITERATOR)                          \
  ) UT_PP_COLON_IF(items_count)                                                          \
    UT_PP_ENUM_IN(items_count, UT_CONTAINER_INPUT_ARG_INIT_ITERATOR)                     \
  {}                                                                                     \
  UT_PP_ENUM_IN(items_count, UT_CONTAINER_DECLARE_TYPEDEF)                               \
  template<int v_id>                                                                     \
  struct Item                                                                            \
  {                                                                                      \
    typedef typename container_helper::TypeExtractor<v_id, Container>::Type Type;        \
  };                                                                                     \
  template <int v_id>                                                                    \
  typename container_helper::TypeExtractor<v_id, Container>::Ref Get()                   \
  { return container_helper::TypeExtractor<v_id, Container>::Apply(this); }              \
  template <int v_id>                                                                    \
  typename container_helper::TypeExtractor<v_id, Container>::ConstRef Get() const        \
  { return container_helper::TypeExtractor<v_id, Container>::Apply(this); }              \
  template <typename ValueType>                                                          \
  typename container_helper::TypeExtractor<                                              \
    container_helper::IdExtractor<UT_CONTAINER_EXT_ARGS(items_count)>::id, Container     \
  >::Ref Get()                                                                           \
  {                                                                                      \
     return container_helper::TypeExtractor<                                             \
        container_helper::IdExtractor<UT_CONTAINER_EXT_ARGS(items_count)>::id, Container \
     >::Apply(this);                                                                     \
  }                                                                                      \
  template <typename ValueType>                                                          \
  typename container_helper::TypeExtractor<                                              \
    container_helper::IdExtractor<UT_CONTAINER_EXT_ARGS(items_count)>::id, Container     \
  >::ConstRef Get() const                                                                \
  {                                                                                      \
    return container_helper::TypeExtractor<                                              \
      container_helper::IdExtractor<UT_CONTAINER_EXT_ARGS(items_count)>::id, Container   \
    >::Apply(this);                                                                      \
  }                                                                                      \
  UT_PP_ENUM_IN(items_count, UT_CONTAINER_DECLARE_VAR)                                   \
  enum { size = items_count };                                                           \
};

//----------------------------------------------------------------------------//
// All specialized variations are declared here.
UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_SPECIALIZATION)

//----------------------------------------------------------------------------//
#endif // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//