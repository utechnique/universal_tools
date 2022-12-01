//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "meta/parameters/ut_binary_parameter.h"
#include "containers/ut_hashmap.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Meta parameter for the hashmap types.
template<template<typename, typename, typename, typename, typename> class HashMapTemplate,
         typename Key,
         typename Value,
         class HashFunction,
         class KeyEqual,
         class Allocator>
class HashMapParameter : public BaseParameter
{
	using HashMapType = HashMapTemplate<Key,
	                                    Value,
	                                    HashFunction,
	                                    KeyEqual,
	                                    Allocator>;
	using ThisParameter = HashMapParameter<HashMapTemplate,
	                                       Key,
	                                       Value,
	                                       HashFunction,
	                                       KeyEqual,
	                                       Allocator>;

	// Helper proxy template class to simplify
	// serialization of the ut::HashMap container.
	struct ProxyNode : public Reflective
	{
		ProxyNode(Key key, Value* value) : key_copy(Move(key))
		                                 , value_ptr(value)
		{}

		Key key_copy;
		Value* value_ptr;

		void Reflect(Snapshot& snapshot)
		{
			snapshot.Add(key_copy, "key");
			snapshot.Add(*value_ptr, "value");
		}
	};

public:
	// Constructor
	//    @param p - pointer to the managed array
	HashMapParameter(HashMapType* p) : BaseParameter(p)
	{}

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< HashMapType >();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		HashMapType& map = *static_cast<HashMapType*>(ptr);

		// import hashmap to the proxy structure
		Import();

		// register proxy nodes
		const size_t element_count = proxy_nodes.Count();
		for (size_t i = 0; i < element_count; i++)
		{
			snapshot << proxy_nodes[i];
		}

		// deserialized proxy will be exported back to the hashmap
		// after loading is done
		snapshot.SetPostLoadCallback(MemberFunction<ThisParameter, void()>(this, &ThisParameter::Export));
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// get array reference from pointer
		HashMapType& map = *static_cast<HashMapType*>(ptr);

		// write value and key type names
		if (controller.GetInfo().HasTypeInformation())
		{
			// value type
			String type_name = BaseParameter::DeduceTypeName<Value>();
			Optional<Error> write_type_error = controller.WriteAttribute(type_name, node_names::skValueType);
			if (write_type_error)
			{
				return write_type_error;
			}

			// key type
			type_name = BaseParameter::DeduceTypeName<Key>();
			write_type_error = controller.WriteAttribute(type_name, node_names::skKeyType);
			if (write_type_error)
			{
				return write_type_error;
			}
		}

		// write map size
		Controller::SizeType num = static_cast<Controller::SizeType>(map.Count());
		Optional<Error> write_num_error = controller.WriteAttribute(num, node_names::skCount);
		if (write_num_error)
		{
			return write_num_error;
		}

		// success
		return Optional<Error>();
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		// get array reference from pointer
		HashMapType& map = *static_cast<HashMapType*>(ptr);

		// read key/value typenames and compare with the current ones
		if (controller.GetInfo().HasTypeInformation())
		{
			// check value type
			Result<String, Error> type_result = controller.ReadAttribute<String>(node_names::skValueType);
			if (!type_result)
			{
				return type_result.MoveAlt();
			}
			else if (BaseParameter::DeduceTypeName<Value>() != type_result.Get())
			{
				return Error(error::types_not_match);
			}

			// check key type
			type_result = controller.ReadAttribute<String>(node_names::skKeyType);
			if (!type_result)
			{
				return type_result.MoveAlt();
			}
			else if (BaseParameter::DeduceTypeName<Key>() != type_result.Get())
			{
				return Error(error::types_not_match);
			}
		}

		// read map size
		Result<Controller::SizeType, Error> read_num_result = controller.ReadAttribute<Controller::SizeType>(node_names::skCount);
		if (!read_num_result)
		{
			return read_num_result.MoveAlt();
		}

		// clear the map
		map.Reset();

		// resize the proxy array
		const size_t element_count = read_num_result.Get();
		proxy_values.Resize(element_count);
		proxy_nodes.Reset();
		for (size_t i = 0; i < element_count; i++)
		{
			if (!proxy_nodes.Add(ProxyNode(Key(), &proxy_values[i])))
			{
				return Error(error::out_of_memory);
			}
		}

		// success
		return Optional<Error>();
	}

	// Returns a set of traits specific for this parameter.
	Traits GetTraits() override
	{
		Traits::ContainerTraits container_traits;
		container_traits.contains_multiple_elements = true;
		container_traits.managed_type_is_polymorphic = IsBaseOf<Polymorphic, Value>::value;
		container_traits.callbacks.reset = MemberFunction<ThisParameter, void()>(this, &ThisParameter::Reset);
		container_traits.callbacks.remove_element = MemberFunction<ThisParameter, void(void*)>(this, &ThisParameter::RemoveElement);

		Traits traits;
		traits.container = container_traits;

		return traits;
	}

	// Deletes all elements.
	void Reset()
	{
		HashMapType& map = *static_cast<HashMapType*>(ptr);
		map.Reset();
	}

	// Removes the desired element.
	void RemoveElement(void* element_address)
	{
		const ut::uptr element_ptr = reinterpret_cast<ut::uptr>(element_address);
		HashMapType& map = *static_cast<HashMapType*>(ptr);
		typename HashMapType::Iterator iterator;
		for (iterator = map.Begin(); iterator != map.End(); ++iterator)
		{
			Pair<const Key, Value>& pair = *iterator;
			if (reinterpret_cast<ut::uptr>(&pair) == element_ptr ||
				reinterpret_cast<ut::uptr>(&pair.GetFirst()) == element_ptr ||
				reinterpret_cast<ut::uptr>(&pair.GetSecond()) == element_ptr)
			{
				map.Remove(pair.GetFirst());
				break;
			}
		}
	}

private:
	// Imports map data to the temporary proxy objects.
	//    @param copy - reference to the original hashmap
	//                  representing a source of data.
	void Import()
	{
		HashMapType& map = *static_cast<HashMapType*>(ptr);
		typename HashMapType::Iterator iterator;
		for (iterator = map.Begin(); iterator != map.End(); ++iterator)
		{
			Pair<const Key, Value>& element = *iterator;
			if (!proxy_nodes.Add(ProxyNode(element.GetFirst(), &element.second)))
			{
				throw Error(error::out_of_memory);
			}
		}
	}

	// Exports temporary proxy object to the original map.
	void Export()
	{
		// cast pointer to the reference to the hashmap
		HashMapType& map = *static_cast<HashMapType*>(ptr);

		// export data
		const size_t element_count = proxy_nodes.Count();
		for (size_t i = 0; i < element_count; i++)
		{
			const ProxyNode& proxy_node = proxy_nodes[i];
			map.Insert(Move(proxy_node.key_copy), Move(*proxy_node.value_ptr));
		}

		// clear memory
		proxy_nodes.Reset();
		proxy_values.Reset();
	}

	// Proxy objects to perform save/load.
	ut::Array<ProxyNode> proxy_nodes;
	ut::Array<Value> proxy_values;
};

//----------------------------------------------------------------------------//
// Specialization for the ut::DenseHashMap type parameter.
template<typename Key,
         typename Value,
         class HashFunction,
         class KeyEqual,
         class Allocator>
class Parameter<DenseHashMap<Key,
                             Value,
                             HashFunction,
                             KeyEqual,
                             Allocator> > : public HashMapParameter<DenseHashMap,
                                                                    Key,
                                                                    Value,
                                                                    HashFunction,
                                                                    KeyEqual,
                                                                    Allocator>
{
public:
	Parameter(DenseHashMap<Key,
	                       Value,
	                       HashFunction,
	                       KeyEqual,
	                       Allocator>* p) : HashMapParameter<DenseHashMap,
	                                                         Key,
	                                                         Value,
	                                                         HashFunction,
	                                                         KeyEqual,
	                                                         Allocator>(p)
	{}
};

//----------------------------------------------------------------------------//
// Specialization for the ut::SparseHashMap type parameter.
template<typename Key,
         typename Value,
         class HashFunction,
         class KeyEqual,
         class Allocator>
class Parameter<SparseHashMap<Key,
                              Value,
                              HashFunction,
                              KeyEqual,
                              Allocator> > : public HashMapParameter<SparseHashMap,
                                                                     Key,
                                                                     Value,
                                                                     HashFunction,
                                                                     KeyEqual,
                                                                     Allocator>
{
public:
	Parameter(SparseHashMap<Key,
	                        Value,
	                        HashFunction,
	                        KeyEqual,
	                        Allocator>* p) : HashMapParameter<SparseHashMap,
	                                                          Key,
	                                                          Value,
	                                                          HashFunction,
	                                                          KeyEqual,
	                                                          Allocator>(p)
	{}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//