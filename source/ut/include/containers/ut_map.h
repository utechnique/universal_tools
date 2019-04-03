//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_array.h"
#include "containers/ut_pair.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Map is a standard container made up of (key,value) pairs, which can be
// retrieved based on a key, search and other operations are linear. If you
// need faster container - use ut::AVLTree instead.
template <typename Key, typename Value>
class Map : public Array< Pair<Key, Value> >
{
	typedef Pair<Key, Value> PairType;
	typedef Array<PairType> Base;
public:
	// Iterator types are inherited from the base class
	typedef typename Base::ConstIterator ConstIterator;
	typedef typename Base::Iterator Iterator;

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	bool Insert(const Key& key, const Value& value)
	{
		return EmplacePair(key, value);
	}

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
#if CPP_STANDARD >= 2011
	bool Insert(Key && key, Value && value)
	{
		return EmplacePair(Move(key), Move(value));
	}
#endif

	// Inserts new key-value pair to the map
	//    @param key - constant l-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
#if CPP_STANDARD >= 2011
	bool Insert(const Key& key, Value && value)
	{
		return EmplacePair(key, Move(value));
	}
#endif

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - constant l-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
#if CPP_STANDARD >= 2011
	bool Insert(Key && key, const Value& value)
	{
		return EmplacePair(Move(key), value);
	}
#endif

	// Inserts new key-value pair to the map
	//    @param key - r-value refenrence to the key
	//    @param value - r-value refenrence to the value
	//    @return - 'true' if pair was inserted successfully,
	//              'false' if pair with such key already exists
	template <typename ArgType1, typename ArgType2>
#if CPP_STANDARD >= 2011
	inline bool EmplacePair(ArgType1 && key, ArgType2 && value)
#else
	inline bool EmplacePair(const ArgType1& key, const ArgType2& value)
#endif
	{
		for (size_t i = 0; i < Base::num; i++)
		{
			if (Base::arr[i].first == key)
			{
				return false;
			}
		}
		return Base::Add(PairType(Forward<ArgType1>(key), Forward<ArgType2>(value)));
	}

	// Finds an element with key equivalent to @key
	//    @param key - const reference to the key value
	//    @return - reference to the value, or ut::Error
	Result<Ref<Value>, Error> Find(const Key& key)
	{
		for (size_t i = 0; i < Base::num; i++)
		{
			if (Base::arr[i].first == key)
			{
				return Ref<Value>(Base::arr[i].second);
			}
		}
		return MakeError(error::not_found);
	}

	// Removes an element with key equivalent to @key from the map
	//    @param key - const reference to the key value
	//    @return - 'true' if element was found and deleted,
	//              'false' if element was not found
	bool Remove(const Key& key)
	{
		for (size_t i = 0; i < Base::num; i++)
		{
			if (Base::arr[i].first == key)
			{
				Base::Remove(i);
				return true;
			}
		}
		return false;
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//