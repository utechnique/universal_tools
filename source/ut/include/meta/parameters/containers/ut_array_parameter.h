//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Array> is a template specialization for array types.
template<typename T>
class Parameter< Array<T> > : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed string
	Parameter(Array<T>* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return skTypeName;
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		Array<T>& arr = *static_cast<Array<T>*>(ptr);

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
		Array<T>& arr = *static_cast<Array<T>*>(ptr);

		// write value type name
		if (controller.GetInfo().HasTypeInformation())
		{
			String value_type_name = GetValueTypeName();
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
		Array<T>& arr = *static_cast<Array<T>*>(ptr);

		// read value typename and compare with current one
		if (controller.GetInfo().HasTypeInformation())
		{
			Result<String, Error> read_type_result = controller.ReadAttribute<String>(node_names::skValueType);
			if (!read_type_result)
			{
				return read_type_result.MoveAlt();
			}

			// check types
			String current_type_name = GetValueTypeName();
			if (current_type_name != read_type_result.GetResult())
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
		arr.Resize(static_cast<size_t>(read_num_result.GetResult()));

		// success
		return Optional<Error>();
	}

	// Returns 'true' - managed object is an array.
	bool IsArray() const
	{
		return true;
	}

private:

	// Returns a name of the contained value type
	String GetValueTypeName() const
	{
		const Parameter<T> parameter(static_cast<T*>(ptr));
		return parameter.GetTypeName();
	}

	// name of the ut::Array type
	static const char* skTypeName;
};

//----------------------------------------------------------------------------//
// name of the ut::Array type
template<typename T>
const char* Parameter< Array<T> >::skTypeName = "array";

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//