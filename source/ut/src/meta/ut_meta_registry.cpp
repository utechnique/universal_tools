//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_registry.h"
#include "dbg/ut_log.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Writes serialized parameters to the stream.
//    @param stream - data will be written to this stream
//    @return - ut::Error if encountered an error
Optional<Error> MetaRegistry::Save(OutputStream& stream)
{
	// write the number of parameters
	const SizeType parameters_num = static_cast<SizeType>(parameters.GetNum());
	Optional<Error> save_num_error = stream.Write(&parameters_num, sizeof(parameters_num), 1);
	if (save_num_error)
	{
		return save_num_error;
	}

	// save parameters sequentially
	for (size_t i = 0; i < parameters_num; i++)
	{
		// write parameter name
		String parameter_name = parameters[i]->GetName();
		Parameter<String> name_write_parameter(&parameter_name);
		Optional<Error> save_parameter_name_error = name_write_parameter.Save(stream);
		if (save_parameter_name_error)
		{
			return save_parameter_name_error;
		}

		// get the current stream cursor position
		Result<stream::Cursor, Error> start_cursor_result = stream.GetCursor();
		if (!start_cursor_result)
		{
			error::Code error_code = start_cursor_result.GetAlt().GetCode();
			return Error(error_code, "Stream doesn't support positioning.");
		}

		// extract the starting position (before the size variable) from the result,
		// note that this position located after(!) the name of the parameter,
		// that is done so that parameter name could be read first, and then
		// it is decided if parameter to be skipped or not (using the size variable
		// located right after this position)
		const stream::Cursor start_position = start_cursor_result.GetResult();

		// write fictive parameter size
		const SizeType fictive_size = 0;
		Optional<Error> write_fictive_size_error = stream.Write(&fictive_size, sizeof(fictive_size), 1);
		if (write_fictive_size_error)
		{
			return write_fictive_size_error;
		}

		// save the parameter
		Optional<Error> save_parameter_error = parameters[i]->Save(stream);
		if (save_parameter_error)
		{
			return save_parameter_error;
		}

		// get the cursor position of the stream (parameter has already been saved)
		Result<stream::Cursor, Error> final_cursor_result = stream.GetCursor();
		if (!final_cursor_result)
		{
			error::Code error_code = final_cursor_result.GetAlt().GetCode();
			return Error(error_code, "Can't get the stream cursor after saving a parameter.");
		}

		// extract the final cursor position
		const stream::Cursor end_position = final_cursor_result.GetResult();

		// calculate the size of the parameter after it was completely written, but take
		// a note, that parameter name size is not included here
		const SizeType parameter_size = static_cast<SizeType>(end_position - start_position);

		// move back cursor position
		Optional<Error> move_back_error = stream.MoveCursor(start_position);
		if (move_back_error)
		{
			return move_back_error;
		}

		// write the real parameter size
		Optional<Error> write_real_size_error = stream.Write(&parameter_size, sizeof(parameter_size), 1);
		if (write_real_size_error)
		{
			return write_real_size_error;
		}

		// move cursor forward
		Optional<Error> move_forward_error = stream.MoveCursor(end_position);
		if (move_forward_error)
		{
			return move_forward_error;
		}
	}

	// success
	return Optional<Error>();
}

// Loads serialized parameters from the stream.
//    @param stream - data will be loaded from this stream
//    @return - ut::Error if encountered an error
Optional<Error> MetaRegistry::Load(InputStream& stream)
{
	// read the number of parameters
	SizeType parameters_num = 0;
	Optional<Error> read_num_error = stream.Read(&parameters_num, sizeof(parameters_num), 1);
	if (read_num_error)
	{
		return read_num_error;
	}

	// read parameters sequentially
	for (size_t i = 0; i < parameters_num; i++)
	{
		// read parameter name
		String parameter_name = parameters[i]->GetName();
		Parameter<String> name_read_parameter(&parameter_name);
		Optional<Error> load_parameter_name_error = name_read_parameter.Load(stream);
		if (load_parameter_name_error)
		{
			return load_parameter_name_error;
		}

		// get the current stream cursor position
		Result<stream::Cursor, Error> start_cursor_result = stream.GetCursor();
		if (!start_cursor_result)
		{
			error::Code error_code = start_cursor_result.GetAlt().GetCode();
			return Error(error_code, "Stream doesn't support positioning.");
		}
		stream::Cursor parameter_position = start_cursor_result.GetResult();

		// read parameter size
		SizeType parameter_size = 0;
		Optional<Error> read_parameter_size_error = stream.Read(&parameter_size, sizeof(parameter_size), 1);
		if (read_parameter_size_error)
		{
			return read_parameter_size_error;
		}

		// get the corresponding parameter
		Optional< Ref<NamedParameter> > parameter_search_result = FindParameter(parameter_name);
		if (parameter_search_result)
		{
			// read the parameter
			NamedParameter& parameter = parameter_search_result.Get();
			Optional<Error> load_parameter_error = parameter.Load(stream);
			if (load_parameter_error)
			{
				// skip this parameter
				Optional<Error> move_cursor_error = stream.MoveCursor(parameter_position + (stream::Cursor)parameter_size);
				if (move_cursor_error)
				{
					return move_cursor_error;
				}

				// log that we had to skip current parameter, but it's not fatal and we can move on
				if (skLogSerializationEvents)
				{
					log << CarriageReturn<char>() << "Serialization error: Reflective parameter with the name \"";
					log << parameter_name << "\" failed to load, error code is" << move_cursor_error.Get().GetCode() << ".";
					log << CarriageReturn<char>();
				}
			}
		}
		else
		{
			// skip this parameter
			Optional<Error> move_cursor_error = stream.MoveCursor(parameter_position + (stream::Cursor)parameter_size);
			if (move_cursor_error)
			{
				return move_cursor_error;
			}
			// log that we had to skip current parameter due to names mismatch,
			// but it's not fatal and we can move on
			if (skLogSerializationEvents)
			{
				log << CarriageReturn<char>() << "Serialization error: Reflective parameter with the name \"";
				log << parameter_name << "\" wasn't found in the registry and was skipped.";
				log << CarriageReturn<char>();
			}
		}
	}

	// success
	return Optional<Error>();
}

// Writes managed object data to the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node to contain the managed data
//    @return - ut::Error if encountered an error
Optional<Error> MetaRegistry::Save(Tree<text::Node>& node)
{
	// save every parameter
	node.data.is_array = false;
	for (size_t i = 0; i < parameters.GetNum(); i++)
	{
		// create parameter node
		Tree<text::Node> parameter_node;
		parameters[i]->SetTextNodeName(parameter_node);
		Optional<Error> save_parameter_error = parameters[i]->Save(parameter_node);
		if (save_parameter_error)
		{
			return save_parameter_error;
		}

		// add parameter node to the parent node
		if (!node.Add(Move(parameter_node)))
		{
			return Error(error::out_of_memory);
		}
	}

	// success
	return Optional<Error>();
}

// Loads managed object data from the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node containing the managed data
//    @return - ut::Error if encountered an error
Optional<Error> MetaRegistry::Load(const Tree<text::Node>& node)
{
	// iterate child nodes
	const size_t children_num = node.GetNumChildren();
	for (size_t i = 0; i < children_num; i++)
	{
		// find the parameter by name
		Optional< Ref<NamedParameter> > parameter_result = FindParameter(node[i].data.name);
		if (!parameter_result)
		{
			// log that we had to skip the parameter, because it was not found by name,
			// but it's not fatal and we can move on
			if (skLogSerializationEvents)
			{
				log << CarriageReturn<char>()  << "Serialization error: Can't find reflective parameter \"";
				log << node[i].data.name << "\"." << CarriageReturn<char>();
			}

			// skip this iteration if no parameter was found
			continue;
		}

		// load the parameter
		NamedParameter& parameter = parameter_result.Get();
		Optional<Error> load_error = parameter.Load(node[i]);

		// log that we had to skip the parameter, because it failed to load,
		// but it's not fatal and we can move on
		if (load_error && skLogSerializationEvents)
		{
			log << CarriageReturn<char>() << "Serialization error: failed to load reflective parameter \"";
			log << node[i].data.name << "\", error code is ";
			log << load_error.Get().GetCode() << "." << CarriageReturn<char>();
		}
	}

	// success
	return Optional<Error>();
}

// Searches for the parameter with the specified name
//    @param parameter_name - name of the parameter to be found
//    @return - reference to the found parameter or ut::Error if parameter wasn't found
Optional< Ref<NamedParameter> > MetaRegistry::FindParameter(const String& parameter_name)
{
	for (size_t i = 0; i < parameters.GetNum(); i++)
	{
		if (parameters[i]->GetName() == parameter_name)
		{
			return Ref<NamedParameter>(parameters[i].GetRef());
		}
	}
	return Optional< Ref<NamedParameter> >();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//