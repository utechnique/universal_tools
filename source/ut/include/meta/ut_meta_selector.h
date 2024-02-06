//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_polymorphic.h"
#include "templates/ut_tuple.h"
#include "containers/ut_array.h"
#include "pointers/ut_unique_ptr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::SelectorHelper is a class helping ut::meta::Selector recursively
// iterate derived types.
template<typename SelectorType, typename Head, typename... Tail>
struct SelectorHelper
{
	template<typename BasePtr = ut::UniquePtr<typename SelectorType::Base> >
	static void Select(SelectorType& selector, ut::Array<BasePtr>& src)
	{
		Factory<typename SelectorType::Base>::template Select<Head>(src, selector.template Get<Head>());
		SelectorHelper<SelectorType, Tail...>::template Select<BasePtr>(selector, src);
	}

	static void Reset(SelectorType& selector)
	{
		selector.template Get<Head>().Reset();
		SelectorHelper<SelectorType, Tail...>::Reset(selector);
	}

	static void DisableReduction(SelectorType& selector)
	{
		selector.template Get<Head>().SetCapacityReduction(false);
		SelectorHelper<SelectorType, Tail...>::DisableReduction(selector);
	}
};

// ut::meta::SelectorHelper specialization for the last element.
template<typename SelectorType, typename Last>
struct SelectorHelper<SelectorType, Last>
{
	template<typename BasePtr = ut::UniquePtr<typename SelectorType::Base> >
	static void Select(SelectorType& selector, ut::Array<BasePtr>& src)
	{
		Factory<typename SelectorType::Base>::template Select<Last>(src, selector.template Get<Last>());
	}

	static void Reset(SelectorType& selector)
	{
		selector.template Get<Last>().Reset();
	}

	static void DisableReduction(SelectorType& selector)
	{
		selector.template Get<Last>().SetCapacityReduction(false);
	}
};

//----------------------------------------------------------------------------//
// ut::meta::Selector is a template class main purpose of which is to select
// objects of specified types from an array of pointers to objects of the base
// type. Main difference from the ut::Factory::Select() function is that
// selector contains arrays of references to objects of predefined derived
// types, so there is no need to allocate memory for every new selection.
template<class BaseType, class... Derived>
class Selector : public Tuple<Array< Ref<Derived> >... >
{
	typedef Tuple<Array< Ref<Derived> >... > BaseTuple;
public:
	// Type that is base for all managed @Derived types.
	typedef BaseType Base;

	// Constructor, disables capacity reduction for all managed arrays
	// to optimize possible memory allocations after a Reset() call.
	Selector()
	{
		SelectorHelper<Selector, Derived...>::DisableReduction(*this);
	}

	// Resets all managed arrays of derived types, makes them empty.
	void Reset()
	{
		SelectorHelper<Selector, Derived...>::Reset(*this);
	}

	// Selects objects of the @Derived types and appends them to the
	// corresponding array.
	//    @param src - reference to the array of pointers of the base type.
	template<typename BasePtr = ut::UniquePtr<Base> >
	inline void Select(ut::Array<BasePtr>& src)
	{
		SelectorHelper<Selector, Derived...>::template Select<BasePtr>(*this, src);
	}

	// Returns a reference to the array of references to objects of the type
	// specified by template parameter.
	template<class DerivedType>
	inline Array< Ref<DerivedType> >& Get()
	{
		return BaseTuple::template Get< Array< Ref<DerivedType> > >();
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//