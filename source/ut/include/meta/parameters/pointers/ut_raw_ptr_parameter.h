//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "meta/ut_polymorphic.h"
#include "meta/ut_meta_node.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Ptr> is a template specialization for raw pointers.
template<typename T>
class Parameter<T*> : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed string
	Parameter(T** p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName<T*>();
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// write value type name
		const T* ptr_addr = *static_cast<const T**>(ptr);
		String value_type_name = ptr_addr ? BaseParameter::DeduceTypeName<T>() : String(Type<void>::Name());
		Optional<Error> write_error = controller.WriteAttribute(value_type_name, node_names::skValueType);
		if (write_error)
		{
			return write_error;
		}

		// write id
		return ptr_addr == nullptr ? Optional<Error>() : controller.WriteLink(this, ptr_addr);
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		// read type name
		Result<String, Error> read_type_result = controller.ReadAttribute<String>(node_names::skValueType);
		if (!read_type_result)
		{
			return read_type_result.MoveAlt();
		}

		// check if serialized pointer is not null
		if (read_type_result.GetResult() == Type<void>::Name())
		{
			return Optional<Error>(); // exit, ok
		}

		// read id and link up with the correct object
		return controller.ReadLink(this);
	}

	// Links pointer with provided parameter.
	// Every parameter calling Controller::WriteLink() and
	// Controller::ReadLink() must override this function so that
	// linker could operate with it.
	//    @param parameter - parameter to link with.
	//    @return - ut::Error if encountered an error
	Optional<Error> Link(BaseParameter& parameter)
	{
		// check types
		String value_type_name = BaseParameter::DeduceTypeName<T>();
		if (value_type_name != parameter.GetTypeName())
		{
			String error_desc = "Pointer value type is \"";
			error_desc += value_type_name + "\" and linked type is \"";
			error_desc += parameter.GetTypeName() + "\".";
			return Error(error::types_not_match, error_desc);
		}

		// set pointer value
		*static_cast<const T**>(ptr) = static_cast<const T*>(parameter.GetAddress());

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