//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "meta/parameters/ut_binary_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Pair> is a template specialization for pair types.
template<typename Type1, typename Type2>
class Parameter< Pair<Type1, Type2> > : public BaseParameter
{
	using PairType = Pair<Type1, Type2>;
public:
	// Constructor
	//    @param p - pointer to the managed object
	Parameter(PairType* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName<PairType>();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		PairType& pair = *static_cast<PairType*>(ptr);
		snapshot << pair.first;
		snapshot << pair.second;
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		if (controller.GetInfo().HasTypeInformation())
		{
			PairType& pair = *static_cast<PairType*>(ptr);

			// write key type name for type1
			String key_type_name = BaseParameter::DeduceTypeName<Type1>();
			Optional<Error> write_key_type_error = controller.WriteAttribute(key_type_name, node_names::skKeyType);
			if (write_key_type_error)
			{
				return write_key_type_error;
			}

			// write value type name for type2
			String value_type_name = BaseParameter::DeduceTypeName<Type2>();
			Optional<Error> write_value_type_error = controller.WriteAttribute(value_type_name, node_names::skValueType);
			if (write_value_type_error)
			{
				return write_value_type_error;
			}
		}

		// success
		return Optional<Error>();
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		// read value typename and compare with current one
		if (controller.GetInfo().HasTypeInformation())
		{
			PairType& map = *static_cast<PairType*>(ptr);

			// read key type
			Result<String, Error> read_type_result = controller.ReadAttribute<String>(node_names::skKeyType);
			if (!read_type_result)
			{
				return read_type_result.MoveAlt();
			}

			// check key types
			String current_type_name = BaseParameter::DeduceTypeName<Type1>();
			if (current_type_name != read_type_result.Get())
			{
				return Error(error::types_not_match);
			}

			// read value type
			read_type_result = controller.ReadAttribute<String>(node_names::skValueType);
			if (!read_type_result)
			{
				return read_type_result.MoveAlt();
			}

			// check value types
			current_type_name = BaseParameter::DeduceTypeName<Type2>();
			if (current_type_name != read_type_result.Get())
			{
				return Error(error::types_not_match);
			}
		}

		// success
		return Optional<Error>();
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//