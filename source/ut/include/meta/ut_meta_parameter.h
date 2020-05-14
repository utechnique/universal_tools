//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_meta_base_parameter.h"
#include "meta/ut_meta_controller.h"
#include "meta/ut_meta_snapshot.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Parameter without specialization is a template parameter for
// fundamental types and simple structures (containing only simple types) or
// classes inherited from ut::meta::Reflective.
template<typename T>
class Parameter : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed object
	Parameter(T* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName<T>();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		ReflectVariant<T>(snapshot);
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		return SaveVariant<T>(controller);
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		return LoadVariant<T>(controller);
	}

private:
	// SFINAE_IS_REFLECTIVE and SFINAE_IS_NOT_REFLECTIVE are temporarily defined
	// here to make short SFINAE parameter. Default template parameters in member
	// functions are allowed only since C++11 for visual studio, so we need to apply
	// SFINAE pattern via default function argument to deduce the correct way to
	// save/load appropriate parameter.
#define SFINAE_IS_REFLECTIVE \
	typename EnableIf<IsBaseOf<Reflective, ElementType>::value>::Type* sfinae = nullptr
#define SFINAE_IS_NOT_REFLECTIVE \
	typename EnableIf<!IsBaseOf<Reflective, ElementType>::value>::Type* sfinae = nullptr

	// If managed object is a reflective node (derived from ut::meta::Reflective)
	// then reflect all of it's contents
	template<typename ElementType>
	inline void ReflectVariant(Snapshot& snapshot, SFINAE_IS_REFLECTIVE)
	{
		T* object_ptr = static_cast<T*>(ptr);
		object_ptr->Reflect(snapshot);
	}

	// If managed object is not a reflective node (not derived from ut::meta::Reflective)
	// then do nothing
	template<typename ElementType>
	inline void ReflectVariant(Snapshot& snapshot, SFINAE_IS_NOT_REFLECTIVE)
	{
		// v o i d
	}

	// If managed object is a reflective object (derived from ut::meta::Reflective),
	// there is nothing to save (all reflected members will be saved separately)
	template<typename ElementType>
	inline Optional<Error> SaveVariant(Controller& controller,
	                                   SFINAE_IS_REFLECTIVE) const
	{
		return Optional<Error>();
	}

	// If managed object is not a reflective object (not derived from ut::meta::Reflective),
	// then just write value to the value node using meta controller
	template<typename ElementType>
	inline Optional<Error> SaveVariant(Controller& controller,
	                                   SFINAE_IS_NOT_REFLECTIVE) const
	{
		return controller.WriteValue<T>(*static_cast<const T*>(ptr));
	}

	// If managed object is a reflective object (derived from ut::meta::Reflective),
	// there is nothing to load (all reflected members will be loaded separately)
	template<typename ElementType>
	inline Optional<Error> LoadVariant(Controller& controller,
	                                   SFINAE_IS_REFLECTIVE) const
	{
		return Optional<Error>();
	}

	// If managed object is not an archive (not derived from ut::Archive)
	// then just read raw byte data of the corresponding type size
	template<typename ElementType>
	inline Optional<Error> LoadVariant(Controller& controller,
	                                   SFINAE_IS_NOT_REFLECTIVE)
	{
		// read value
		Result<T, Error> read_result = controller.ReadValue<T>();
		if (!read_result)
		{
			return read_result.MoveAlt();
		}

		// move result to the managed object
		*static_cast<T*>(ptr) = read_result.MoveResult();

		// success
		return Optional<Error>();
	}

	// undef macros here
#undef SFINAE_IS_REFLECTIVE
#undef SFINAE_IS_NOT_REFLECTIVE
};

//----------------------------------------------------------------------------//
// Partial specialization for static arrays.
template<typename T, size_t arr_size>
class Parameter<T[arr_size]> : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed array
	Parameter(T (*p)[arr_size]) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName<T[arr_size]>();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		T(&arr)[arr_size] = *static_cast<T(*)[arr_size]>(ptr);

		// register all elements
		for (size_t i = 0; i < arr_size; i++)
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
		T(&arr)[arr_size] = *static_cast<T(*)[arr_size]>(ptr);

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
		const Controller::SizeType num = arr_size;
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
		T(&arr)[arr_size] = *static_cast<T(*)[arr_size]>(ptr);

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
			if (current_type_name != read_type_result.GetResult())
			{
				return Error(error::types_not_match, "Static arrays have different type.");
			}
		}

		// read array size
		Result<Controller::SizeType, Error> read_num_result = controller.ReadAttribute<Controller::SizeType>(node_names::skCount);
		if (!read_num_result)
		{
			return read_num_result.MoveAlt();
		}

		// check array size
		if (read_num_result.GetResult() != arr_size)
		{
			return Error(error::types_not_match, "Static arrays have different size.");
		}

		// success
		return Optional<Error>();
	}

	// Returns 'true' - managed object is an array.
	bool IsArray() const
	{
		return true;
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//