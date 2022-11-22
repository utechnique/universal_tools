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
// ut::Parameter<Array> is a template specialization for array types.
template<typename T, typename Allocator, typename Preallocator>
class Parameter< Array<T, Allocator, Preallocator> > : public BaseParameter
{
	using ArrayType = Array<T, Allocator, Preallocator>;
	using ThisParameter = Parameter<ArrayType>;
public:
	// Constructor
	//    @param p - pointer to the managed array
	Parameter(ArrayType* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< ArrayType >();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		ArrayType& arr = *static_cast<ArrayType*>(ptr);

		// register all elements
		for (size_t i = 0; i < arr.GetNum(); i++)
		{
			snapshot << arr[i];
		}
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// get array reference from pointer
		ArrayType& arr = *static_cast<ArrayType*>(ptr);

		// write value type name
		if (controller.GetInfo().HasTypeInformation())
		{
			String value_type_name = BaseParameter::DeduceTypeName<T>();
			Optional<Error> write_value_type_error = controller.WriteAttribute(value_type_name, node_names::skValueType);
			if (write_value_type_error)
			{
				return write_value_type_error;
			}
		}

		// write array size
		Controller::SizeType num = static_cast<Controller::SizeType>(arr.GetNum());
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
		ArrayType& arr = *static_cast<ArrayType*>(ptr);

		// read value typename and compare with current one
		if (controller.GetInfo().HasTypeInformation())
		{
			Result<String, Error> read_type_result = controller.ReadAttribute<String>(node_names::skValueType);
			if (!read_type_result)
			{
				return read_type_result.MoveAlt();
			}

			// check types
			String current_type_name = BaseParameter::DeduceTypeName<T>();
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
		arr.Resize(static_cast<size_t>(read_num_result.Get()));

		// success
		return Optional<Error>();
	}

	// Returns a set of traits specific for this parameter.
	Traits GetTraits() override
	{
		Traits::ContainerTraits container_traits;
		container_traits.contains_multiple_elements = true;
		container_traits.managed_type_is_polymorphic = IsBaseOf<Polymorphic, T>::value;
		container_traits.callbacks.reset = MemberFunction<ThisParameter, void()>(this, &ThisParameter::Reset);
		container_traits.callbacks.push_back = MemberFunction<ThisParameter, void()>(this, &ThisParameter::PushBack);
		container_traits.callbacks.remove_element = MemberFunction<ThisParameter, void(void*)>(this, &ThisParameter::RemoveElement);

		Traits traits;
		traits.container = container_traits;

		return traits;
	}

	// Deletes all elements.
	void Reset()
	{
		ArrayType& arr = *static_cast<ArrayType*>(ptr);
		arr.Empty();
	}

	// Adds an element to the end of the array.
	void PushBack()
	{
		ArrayType& arr = *static_cast<ArrayType*>(ptr);
		T new_element = {};
		arr.Add(ut::Move(new_element));
	}

	// Removes the desired element.
	void RemoveElement(void* element_address)
	{
		ArrayType& arr = *static_cast<ArrayType*>(ptr);
		const size_t element_count = arr.GetNum();
		for (size_t i = 0; i < element_count; i++)
		{
			if (reinterpret_cast<ut::uptr>(&arr[i]) == reinterpret_cast<ut::uptr>(element_address))
			{
				arr.Remove(i);
				break;
			}
		}
	}
};

//----------------------------------------------------------------------------//
// Specialization for the binary case.
template<typename T, typename Allocator, typename Preallocator>
class BinaryParameter< Array<T, Allocator, Preallocator> > : public BaseParameter
{
	using ArrayType = Array<T, Allocator, Preallocator>;
public:
	// Constructor
	//    @param p - pointer to the managed array
	BinaryParameter(ArrayType* p,
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
		ArrayType& arr = *static_cast<ArrayType*>(ptr);
		const Controller::SizeType count = static_cast<Controller::SizeType>(arr.GetNum());
		const Controller::SizeType size = sizeof(T) * count;

		// write data size
		Optional<Error> write_size_error = controller.WriteAttribute(size, node_names::skSize);
		if (write_size_error)
		{
			return write_size_error;
		}

		// write data
		return controller.WriteBinaryValue(arr.GetAddress(), size, granularity);
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		// get array reference from pointer
		ArrayType& arr = *static_cast<ArrayType*>(ptr);

		// read data size
		Result<Controller::SizeType, Error> size = controller.ReadAttribute<Controller::SizeType>(node_names::skSize);
		if (!size)
		{
			return size.MoveAlt();
		}

		// check size value
		if (size.Get() % sizeof(T) != 0)
		{
			return Error(error::out_of_bounds, "Binary array parameter has invalid size on loading.");
		}

		// resize the array
		const size_t element_count = size.Get() / sizeof(T);
		arr.Resize(element_count);

		// read data
		return controller.ReadBinaryValue(arr.GetAddress(), size.Get(), granularity);
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