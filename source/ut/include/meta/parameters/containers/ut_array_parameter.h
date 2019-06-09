//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
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
		const Parameter<T> parameter(static_cast<T*>(ptr));
		return parameter.GetTypeName() + "_array";
	}

	// Writes managed data to the stream
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(OutputStream& stream)
	{
		// get array reference from pointer
		Array<T>& arr = *static_cast<Array<T>*>(ptr);

		// get array size
		const uint32 num = (uint32)arr.GetNum();

		// write array size as 32-bit integer
		Optional<Error> write_num_error = stream.Write(&num, sizeof(num), 1);
		if (write_num_error)
		{
			return write_num_error;
		}

		// write array elements sequentially
		for (uint32 i = 0; i < num; i++)
		{
			// save every element via corresponding parameter
			Parameter<T> parameter(&arr[i]);
			Optional<Error> save_element_error = parameter.Save(stream);
			if (save_element_error)
			{
				return save_element_error;
			}
		}

		// success
		return Optional<Error>();
	}

	// Loads managed data from the stream
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(InputStream& stream)
	{
		// get array reference from pointer
		Array<T>& arr = *static_cast<Array<T>*>(ptr);

		// read array size as 32-bit integer
		uint32 num;
		Optional<Error> read_num_error = stream.Read(&num, sizeof(num), 1);
		if (read_num_error)
		{
			return read_num_error;
		}

		// resize the array
		arr.Resize(num);

		// read array elements sequentially
		for (uint32 i = 0; i < num; i++)
		{
			// load every element via corresponding parameter
			Parameter<T> parameter(&arr[i]);
			Optional<Error> load_element_error = parameter.Load(stream);
			if (load_element_error)
			{
				return load_element_error;
			}
		}

		// success
		return Optional<Error>();
	}

	// Writes managed object data to the text node.
	//    @param node - text node to contain the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Tree<text::Node>& node)
	{
		// set node name
		SetTextNodeName(node);

		// this is array node
		node.data.is_array = true;

		// get array reference from pointer
		Array<T>& arr = *static_cast<Array<T>*>(ptr);

		// get array size
		const size_t num = arr.GetNum();

		// write array elements sequentially
		for (size_t i = 0; i < num; i++)
		{
			// create a new node for the element
			Tree<text::Node> element_node;

			// save every element via corresponding parameter
			Parameter<T> parameter(&arr[i]);
			Optional<Error> save_element_error = parameter.Save(element_node);

			// validate the result
			if (save_element_error)
			{
				return save_element_error;
			}
			
			// add node to the tree
			if (!node.Add(element_node))
			{
				return Error(error::out_of_memory);
			}
		}

		// success
		return Optional<Error>();
	}

	// Loads managed object data from the text node.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(const Tree<text::Node>& node)
	{
		// get array reference from pointer
		Array<T>& arr = *static_cast<Array<T>*>(ptr);

		// get array size
		const size_t num = node.GetNumChildren();

		// resize the array
		if (!arr.Resize(num))
		{
			return Error(error::out_of_memory);
		}

		// read array elements sequentially
		for (size_t i = 0; i < num; i++)
		{
			// create a new node for the element
			Tree<text::Node> element_node;

			// save every element via corresponding parameter
			Parameter<T> parameter(&arr[i]);
			Optional<Error> load_element_error = parameter.Load(node[i]);

			// validate the result
			if (load_element_error)
			{
				return load_element_error;
			}
		}

		// success
		return Optional<Error>();
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//