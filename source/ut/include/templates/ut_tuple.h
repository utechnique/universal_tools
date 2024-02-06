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
// Contains the actual value for one item in the tuple. The  template
// parameter `leaf_id` allows the `Get` function to find the value in O(1) time
template<int leaf_id, typename Item>
class TupleLeaf
{
public:
	TupleLeaf() {}
	TupleLeaf(Item&& item) : value(Forward<Item>(item)) {}
	Item value;
};

// TupleImpl is a proxy for the final class that has an extra 
// template parameter `leaf_id`.
template<int leaf_id, typename... Items> class TupleImpl;

// Base case: empty tuple
template<int leaf_id> class TupleImpl<leaf_id> {};

// Obtains a reference to i-th item in a tuple
template<int leaf_id, typename HeadItem, typename... TailItems>
inline HeadItem& Get(TupleImpl<leaf_id, HeadItem, TailItems...>& tuple)
{
	// Fully qualified name for the member, to find the right one 
	// (they are all called `value`).
	return tuple.TupleLeaf<leaf_id, HeadItem>::value;
}

// Helper structures to get a type or id of the value inside a tuple
// knowing it's index `leaf_id` or type `Desired`
namespace tuple_helper
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

// Recursive specialization for the tuple template
template<int i, typename HeadItem, typename... TailItems>
class TupleImpl<i, HeadItem, TailItems...> : public TupleLeaf<i, HeadItem>,
	public TupleImpl<i + 1, TailItems...>
{
private:
	// Helper alias template to conveniently get type
	// of the value in a specified leaf
	template<int leaf_id>
	using LeafType = typename tuple_helper::TypeExtractor<leaf_id, HeadItem, TailItems...>::Type;

	// Helper function to get id of the value with the specified type
	// at compile time
	template<typename ValueType>
	static constexpr int GetLeafId()
	{
		return tuple_helper::IdExtractor<0, ValueType, HeadItem, TailItems...>::id;
	}

public:
	// Item type can be accessed using this template
	template<int leaf_id>
	struct Item
	{
		typedef LeafType<leaf_id> Type;
	};

	// Default constructor
	TupleImpl()
	{ }

	// Constructor, all items must be passed here
	TupleImpl(HeadItem head,
		TailItems... tail) : TupleLeaf<i, HeadItem>(Forward<HeadItem>(head))
		, TupleImpl<i + 1, TailItems...>(Forward<TailItems>(tail) ...)
	{ }

	// Returns reference to the value inside a leaf
	template<int value_id>
	LeafType<value_id>& Get()
	{
		return this->TupleLeaf<value_id, LeafType<value_id> >::value;
	}

	// Returns const reference to the value inside a leaf, that is specified by `value_id`
	template<int value_id>
	const LeafType<value_id>& Get() const
	{
		return this->TupleLeaf<value_id, LeafType<value_id> >::value;
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

	// Count of items inside a tuple
	enum { size = tuple_helper::Counter<0, HeadItem, TailItems...>::count };
};

// Templated alias to avoid having to specify `leaf_id = 0`
template<typename... Items>
using Tuple = TupleImpl<0, Items...>;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//