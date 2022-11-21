//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "meta/parameters/ut_binary_parameter.h"
#include "meta/parameters/containers/ut_pair_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Map> is a template specialization for map types.
template<typename Key, typename Value, typename Allocator, typename Preallocator>
class Parameter< Map<Key, Value, Allocator, Preallocator> > : public BaseParameter
{
	using MapType = Map<Key, Value, Allocator, Preallocator>;
	using ThisParameter = Parameter<MapType>;
public:
	// Constructor
	//    @param p - pointer to the managed map object
	Parameter(MapType* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName<MapType>();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		MapType& map = *static_cast<MapType*>(ptr);

		// register all elements
		for (size_t i = 0; i < map.GetNum(); i++)
		{
			snapshot << map[i];
		}
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// get array reference from pointer
		MapType& map = *static_cast<MapType*>(ptr);

		// write value and key type names
		if (controller.GetInfo().HasTypeInformation())
		{
			String value_type_name = BaseParameter::DeduceTypeName<Value>();
			Optional<Error> write_value_type_error = controller.WriteAttribute(value_type_name, node_names::skValueType);
			if (write_value_type_error)
			{
				return write_value_type_error;
			}

			String key_type_name = BaseParameter::DeduceTypeName<Key>();
			Optional<Error> write_key_type_error = controller.WriteAttribute(key_type_name, node_names::skKeyType);
			if (write_key_type_error)
			{
				return write_key_type_error;
			}
		}

		// write array size
		Controller::SizeType num = static_cast<Controller::SizeType>(map.GetNum());
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
		MapType& map = *static_cast<MapType*>(ptr);

		// read value typename and compare with current one
		if (controller.GetInfo().HasTypeInformation())
		{
			// read value type
			Result<String, Error> read_type_result = controller.ReadAttribute<String>(node_names::skValueType);
			if (!read_type_result)
			{
				return read_type_result.MoveAlt();
			}

			// check value types
			String current_type_name = BaseParameter::DeduceTypeName<Value>();
			if (current_type_name != read_type_result.Get())
			{
				return Error(error::types_not_match);
			}

			// read key type
			read_type_result = controller.ReadAttribute<String>(node_names::skKeyType);
			if (!read_type_result)
			{
				return read_type_result.MoveAlt();
			}

			// check value types
			current_type_name = BaseParameter::DeduceTypeName<Key>();
			if (current_type_name != read_type_result.Get())
			{
				return Error(error::types_not_match);
			}
		}

		// read array size
		Result<Controller::SizeType, Error> read_num_result = controller.ReadAttribute<Controller::SizeType>(node_names::skCount);
		if (!read_num_result)
		{
			return read_num_result.MoveAlt();
		}

		// resize the array
		map.Resize(static_cast<size_t>(read_num_result.Get()));

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
		MapType& map = *static_cast<MapType*>(ptr);
		map.Empty();
	}

	// Removes the desired element.
	void RemoveElement(void* element_address)
	{
		MapType& map = *static_cast<MapType*>(ptr);
		const size_t element_count = map.GetNum();
		for (size_t i = 0; i < element_count; i++)
		{
			if (reinterpret_cast<ut::uptr>(&map[i]) == reinterpret_cast<ut::uptr>(element_address) ||
			    reinterpret_cast<ut::uptr>(&map[i].second) == reinterpret_cast<ut::uptr>(element_address))
			{
				map.BaseArray< Pair<Key, Value>, Allocator, Preallocator>::Remove(i);
				break;
			}
		}
	}
};

//----------------------------------------------------------------------------//
// Specialization for the binary case.
template<typename Key, typename Value, typename Allocator, typename Preallocator>
class BinaryParameter< Map<Key, Value, Allocator, Preallocator> > : public BaseParameter
{
	using MapType = Map<Key, Value, Allocator, Preallocator>;
public:
	// Constructor
	//    @param p - pointer to the managed array
	BinaryParameter(MapType* p,
	                Controller::SizeType in_granularity) : BaseParameter(p)
	                                                     , granularity(in_granularity)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return "binary";
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		MapType& map = *static_cast<MapType*>(ptr);
		const Controller::SizeType count = static_cast<Controller::SizeType>(map.GetNum());
		const Controller::SizeType size = sizeof(Pair<Key, Value>) * count;

		// write data size
		Optional<Error> write_size_error = controller.WriteAttribute(size, node_names::skSize);
		if (write_size_error)
		{
			return write_size_error;
		}

		// write data
		return controller.WriteBinaryValue(map.GetAddress(), size, granularity);
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		// get array reference from pointer
		MapType& map = *static_cast<MapType*>(ptr);

		// read data size
		Result<Controller::SizeType, Error> size = controller.ReadAttribute<Controller::SizeType>(node_names::skSize);
		if (!size)
		{
			return size.MoveAlt();
		}

		// check size value
		if (size.Get() % sizeof(Pair<Key, Value>) != 0)
		{
			return Error(error::out_of_bounds, "Binary array parameter has invalid size on loading.");
		}

		// resize the array
		const size_t element_count = size.Get() / sizeof(Pair<Key, Value>);
		map.Resize(element_count);

		// read data
		return controller.ReadBinaryValue(map.GetAddress(), size.Get(), granularity);
	}

private:
	Controller::SizeType granularity;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//