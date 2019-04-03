//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_base_parameter.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_base_of.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Parameter without specializations is a template parameter for fundamental
// types and simple structures (containing only simple types) or classes
// inherited from ut::Archive.
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
		return GetTypeNameVariant<T>();
	}

	// Writes managed data to the stream
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(OutputStream& stream)
	{
		return SaveVariantBinary<T>(stream);
	}

	// Loads managed data from the stream
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(InputStream& stream)
	{
		return LoadVariantBinary<T>(stream);
	}

	// Writes managed object data to the text node.
	//    @param node - text node to contain the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Tree<text::Node>& node)
	{
		return SaveVariantText<T>(node);
	}

	// Loads managed object data from the text node.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(const Tree<text::Node>& node)
	{
		return LoadVariantText<T>(node);
	}

private:
	// SFINAE_IS_ARCHIVE and SFINAE_IS_NOT_ARCHIVE are temporarily defined
	// here to make short SFINAE parameter. Default template parameters are allowed
	// only since C++11, so we need to apply SFINAE pattern via default function
	// argument to deduce the correct way to save/load appropriate parameter.
#define SFINAE_IS_ARCHIVE \
	typename EnableIf<IsBaseOf<Archive, ElementType>::value>::Type* sfinae = nullptr
#define SFINAE_IS_NOT_ARCHIVE \
	typename EnableIf<!IsBaseOf<Archive, ElementType>::value>::Type* sfinae = nullptr

	// If managed object is an archive (derived from ut::Archive)
	// then typename is "archive" for all derived classes
	template<typename ElementType>
	inline static String GetTypeNameVariant(SFINAE_IS_ARCHIVE)
	{
		return Archive::skTypeName;
	}

	// If managed object is not an archive (not derived from ut::Archive)
	// then type name is the name of the intrinsic type
	template<typename ElementType>
	inline static String GetTypeNameVariant(SFINAE_IS_NOT_ARCHIVE)
	{
		return TypeName<T>();
	}

	// If managed object is an archive (derived from ut::Archive)
	// then just call overriden ut::Archive::Save() function
	template<typename ElementType>
	inline Optional<Error> SaveVariantBinary(OutputStream& stream, SFINAE_IS_ARCHIVE)
	{
		Archive* archive = static_cast<Archive*>(ptr);
		UT_ASSERT(archive != nullptr);
		return archive->Save(stream);
	}

	// Text version for archive types
	template<typename ElementType>
	inline Optional<Error> SaveVariantText(Tree<text::Node>& node, SFINAE_IS_ARCHIVE)
	{
		// set node name
		SetTextNodeName(node);

		// save archive
		Archive* archive = static_cast<Archive*>(ptr);
		UT_ASSERT(archive != nullptr);
		return archive->Save(node);
	}

	// If managed object is not an archive (not derived from ut::Archive)
	// then just write raw byte data of the corresponding type size
	template<typename ElementType>
	inline Optional<Error> SaveVariantBinary(OutputStream& stream, SFINAE_IS_NOT_ARCHIVE)
	{
		return stream.Write(ptr, sizeof(T), 1);
	}

	// Text version for simple types
	template<typename ElementType>
	inline Optional<Error> SaveVariantText(Tree<text::Node>& node, SFINAE_IS_NOT_ARCHIVE)
	{
		// set node name
		SetTextNodeName(node);

		// value is a number
		node.data.value_type = String(TypeName<T>());

		// print and save element
		const String text_form = Print<T>(*(static_cast<T*>(ptr)));
		node.data.value = text_form;

		// success
		return Optional<Error>();
	}

	// If managed object is an archive (derived from ut::Archive)
	// then just call overriden ut::Archive::Load() function
	template<typename ElementType>
	inline Optional<Error> LoadVariantBinary(InputStream& stream, SFINAE_IS_ARCHIVE)
	{
		Archive* archive = static_cast<Archive*>(ptr);
		UT_ASSERT(archive != nullptr);
		return archive->Load(stream);
	}

	// Text version for archive types
	template<typename ElementType>
	inline Optional<Error> LoadVariantText(const Tree<text::Node>& node, SFINAE_IS_ARCHIVE)
	{
		// load the archive
		Archive* archive = static_cast<Archive*>(ptr);
		UT_ASSERT(archive != nullptr);
		return archive->Load(node);
	}

	// If managed object is not an archive (not derived from ut::Archive)
	// then just read raw byte data of the corresponding type size
	template<typename ElementType>
	inline Optional<Error> LoadVariantBinary(InputStream& stream, SFINAE_IS_NOT_ARCHIVE)
	{
		return stream.Read(ptr, sizeof(T), 1);
	}

	// Text version for simple types
	template<typename ElementType>
	inline Optional<Error> LoadVariantText(const Tree<text::Node>& node, SFINAE_IS_NOT_ARCHIVE)
	{
		// scan element
		Result<String, Error> value_result = ExtractValueFromTextNode(node);
		if (value_result)
		{
			T& element = *(static_cast<T*>(ptr));
			element = Scan<T>(value_result.GetResult());
		}
		else
		{
			return value_result.MoveAlt();
		}

		// success
		return Optional<Error>();
	}

	// undef macros here
#undef SFINAE_IS_ARCHIVE
#undef SFINAE_IS_NOT_ARCHIVE
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//