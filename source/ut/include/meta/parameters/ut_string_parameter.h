//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
// class ut::Parameter<TString> is specialization for string types. This is
// a special case, because ut::TString can't be derived from ut::Archive due
// to multiple cross-reference problems (being very basic type of the UT library).
// Thus, parameter for the ut::TString must be implemented implicitly.
template<typename T>
class Parameter< TString<T> > : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed string
	Parameter(TString<T>* str) : BaseParameter(str)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return "string";
	}

	// Writes managed data to the stream
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(OutputStream& stream)
	{
		TString<T>& str = *((TString<T>*)ptr);
		size_t len = str.Length() + 1;
		return stream.Write(str.GetAddress(), sizeof(T), len);
	}

	// Loads managed data from the stream
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(InputStream& stream)
	{
		// get string reference from the pointer
		TString<T>& str = *((TString<T>*)ptr);

		// clear the string
		str.Empty();

		// read symbol by symbol and exit after
		// meeting a null-terminator
		T c;
		do
		{
			Optional<Error> read_error = stream.Read(&c, sizeof(T), 1);
			if (read_error)
			{
				return read_error;
			}

			str.Add(c);
		} while (c != '\0');

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

		// set value
		TString<T>& str = *((TString<T>*)ptr);
		node.data.value = str;

		// success
		return Optional<Error>();
	}

	// Loads managed object data from the text node.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(const Tree<text::Node>& node)
	{
		// get string object reference
		TString<T>& str = *((TString<T>*)ptr);
		
		// get the value from the provided node
		Result<String, Error> value_result = ExtractValueFromTextNode(node);
		if (!value_result)
		{
			return value_result.MoveAlt();
		}

		// copy string
		str = value_result.MoveResult();

		// success
		return Optional<Error>();
	}

	// Calculates size of the managed string
	//    @return - size of the string, in bytes
	size_t GetSize()
	{
		TString<T>& str = *((TString<T>*)ptr);
		size_t len = str.Length() + 1;
		return len * sizeof(T);
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//