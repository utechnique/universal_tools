//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_meta_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::BinaryParameter reads/writes managed data in the binary form.
template<typename T>
class BinaryParameter : public BaseParameter
{
public:
	BinaryParameter(T* object,
	                Controller::SizeType in_granularity) : BaseParameter(object)
	                                                     , granularity(in_granularity)
	{}

	// Returns name of the managed type. All derived classes must
	// override this member function. Good practice is to override it with
	// a call to BaseParameter::DeduceTypeName() template function.
	String GetTypeName() const override
	{
		return "binary";
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller) override
	{
		constexpr Controller::SizeType obj_size = sizeof(T);

		// write data size
		Controller::SizeType size = static_cast<Controller::SizeType>(obj_size);
		Optional<Error> write_size_error = controller.WriteAttribute(size, node_names::skSize);
		if (write_size_error)
		{
			return write_size_error;
		}

		// write data
		return controller.WriteBinaryValue(BaseParameter::ptr, obj_size, granularity);
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller) override
	{
		// read data size
		Result<Controller::SizeType, Error> size = controller.ReadAttribute<Controller::SizeType>(node_names::skSize);
		if (!size)
		{
			return size.MoveAlt();
		}

		// read data
		return controller.ReadBinaryValue(BaseParameter::ptr, size.Get(), granularity);
	}

private:
	Controller::SizeType granularity;
};

// Binary data parameter.
template<typename T>
class Parameter< BinaryParameter<T> > : public BinaryParameter<T>
{
public:
	Parameter(BinaryParameter<T>* p) : BinaryParameter<T>(*p)
	{}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//