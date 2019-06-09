//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_parameter.h"
#include "meta/ut_polymorphic.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Parameter<Ptr> is a template specialization for pointers.
template<typename T>
class Parameter< UniquePtr<T> > : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed string
	Parameter(UniquePtr<T>* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		const Parameter<T> parameter(static_cast<T*>(ptr));
		return parameter.GetTypeName();
	}

	// Writes managed data to the stream. Uses @ParameterType template
	// class for writing.
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(OutputStream& stream)
	{
		return SaveVariantBinary<T>(stream);
	}

	// Loads managed data from the stream. Creates new instance of T, adds
	// this instance to the array, and then uses @ParameterType template
	// class for reading serialized data.
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
	// SFINAE_IS_POLYMORPHIC and SFINAE_IS_NOT_POLYMORPHIC are temporarily defined
	// here to make short SFINAE argument. MS Visual Studio 2008 and 2010
	// doesn't support template specialization inside template classes, so the
	// only way to implement Save() and Load() methods both for polymorphic and
	// non-polymorphic archive types - is to use SFINAE pattern (feature).
#define SFINAE_IS_POLYMORPHIC \
	typename EnableIf<IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr
#define SFINAE_IS_NOT_POLYMORPHIC \
	typename EnableIf<!IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr

	// If managed object is a custom (not derived from ut::Polymorphic)
	// element - just call Save() method of the corresponding parameter.
	template<typename ElementType>
	inline Optional<Error> SaveVariantBinary(OutputStream& stream, SFINAE_IS_NOT_POLYMORPHIC)
	{
		UniquePtr<ElementType>* p = static_cast<UniquePtr<ElementType>*>(ptr);
		Parameter<ElementType> parameter(p->Get());
		return parameter.Save(stream);
	}

	// Text version for non-polymorphic types
	template<typename ElementType>
	inline Optional<Error> SaveVariantText(Tree<text::Node>& node, SFINAE_IS_NOT_POLYMORPHIC)
	{
		// set node name
		SetTextNodeName(node);

		// create and fill a new node
		UniquePtr<ElementType>* p = static_cast<UniquePtr<ElementType>*>(ptr);
		Parameter<ElementType> parameter(p->Get());
		Optional<Error> save_parameter_error = parameter.Save(node);

		// validate the result
		if (save_parameter_error)
		{
			return save_parameter_error;
		}

		// success
		return Optional<Error>();
	}

	// If managed object is a polymorphic archive - then
	// we must save the name of the polymorphic type as a string
	// before saving archive itself.
	template<typename ElementType>
	inline Optional<Error> SaveVariantBinary(OutputStream& stream, SFINAE_IS_POLYMORPHIC)
	{
		// get reference to the managed object
		UniquePtr<ElementType>& ptr_ref = *static_cast<UniquePtr<ElementType>*>(ptr);

		// get the name of the polymorphic type
		const DynamicType& dyn_type = ptr_ref->Identify();
		String name(dyn_type.GetName());

		// save name as string parameter
		Parameter<String> name_write_parameter(&name);
		Optional<Error> write_name_error = name_write_parameter.Save(stream);
		if (write_name_error)
		{
			return write_name_error;
		}

		// save archive
		Optional<Error> save_archive_error = ptr_ref->Save(stream);
		if (save_archive_error)
		{
			return save_archive_error;
		}

		// success
		return Optional<Error>();
	}

	// Text version for polymorphic types
	template<typename ElementType>
	inline Optional<Error> SaveVariantText(Tree<text::Node>& node, SFINAE_IS_POLYMORPHIC)
	{
		// set node name
		SetTextNodeName(node);

		// get reference to the managed object
		UniquePtr<ElementType>& ptr_ref = *static_cast<UniquePtr<ElementType>*>(ptr);

		// get the name of the polymorphic type
		const DynamicType& dyn_type = ptr_ref->Identify();
		const String dynamic_type_name(dyn_type.GetName());

		// create dynamic type name as an attribute
		Tree<text::Node> dyn_type_attribute;
		dyn_type_attribute.data.is_attribute = true;
		dyn_type_attribute.data.name = skDynamicTypeNodeName;
		dyn_type_attribute.data.value = dynamic_type_name;

		// add attribute to the object node
		if (!node.Add(dyn_type_attribute))
		{
			return Error(error::out_of_memory);
		}

		// save archive
		Optional<Error> save_archive_error = ptr_ref->Save(node);
		if (save_archive_error)
		{
			return save_archive_error;
		}

		// success
		return Optional<Error>();
	}

	// If managed object is a custom (not derived from ut::Polymorphic)
	// element - just call Load() method of the appropriate parameter after
	// creating a new instance of the archive
	template<typename ElementType>
	inline Optional<Error> LoadVariantBinary(InputStream& stream, SFINAE_IS_NOT_POLYMORPHIC)
	{
		UniquePtr<ElementType>& ptr_ref = *static_cast<UniquePtr<ElementType>*>(ptr);
		ptr_ref = new ElementType;
		Parameter<ElementType> parameter(ptr_ref.Get());
		return parameter.Load(stream);
	}

	// Text version for non-polymorphic types
	template<typename ElementType>
	inline Optional<Error> LoadVariantText(const Tree<text::Node>& node, SFINAE_IS_NOT_POLYMORPHIC)
	{
		// load the element
		UniquePtr<ElementType>& ptr_ref = *static_cast<UniquePtr<ElementType>*>(ptr);
		ptr_ref = new ElementType;
		Parameter<ElementType> parameter(ptr_ref.Get());
		return parameter.Load(node);
	}

	// If managed object is a polymorphic archive - then
	// we must load polymorphic name string, and create an
	// object of the corresponding type.
	template<typename ElementType>
	inline Optional<Error> LoadVariantBinary(InputStream& stream, SFINAE_IS_POLYMORPHIC)
	{
		// get reference to the managed object
		UniquePtr<ElementType>& ptr_ref = *static_cast<UniquePtr<ElementType>*>(ptr);

		// read type name of the object to be loaded
		String type_name;
		Parameter<String> name_read_parameter(&type_name);
		Optional<Error> load_name_error = name_read_parameter.Load(stream);
		if (load_name_error)
		{
			return load_name_error;
		}

		// get dynamic type by name
		Result<ConstRef<DynamicType>, Error> type_result = Factory<T>::GetType(type_name);
		if (!type_result)
		{
			return type_result.MoveAlt();
		}

		// create new object
		const DynamicType& dyn_type = type_result.GetResult();
		ptr_ref = (T*)dyn_type.CreateInstance();

		// load archive
		Optional<Error> load_archive_error = ptr_ref->Load(stream);
		if (load_archive_error)
		{
			return load_archive_error;
		}

		// success
		return Optional<Error>();
	}

	// Text version for polymorphic types
	template<typename ElementType>
	inline Optional<Error> LoadVariantText(const Tree<text::Node>& node, SFINAE_IS_POLYMORPHIC)
	{
		// get reference to the managed object
		UniquePtr<ElementType>& ptr_ref = *static_cast<UniquePtr<ElementType>*>(ptr);

		// check if the node has the leaf for dynamic type name
		if (node.GetNumChildren() == 0)
		{
			String description("Text node for the dynamic types must have at least one child");
			description += "node with the dynamic type name string.";
			return Error(error::fail, description);
		}

		// copy the node
		Tree<text::Node> pure_node(node);

		// get type name of the object to be loaded
		String type_name;
		bool found_type_name = false;
		for (size_t i = 0; i < pure_node.GetNumChildren(); i++)
		{
			if (pure_node[i].data.name == skDynamicTypeNodeName)
			{
				if (!pure_node[i].data.value)
				{
					String description("Text node has the node for the dynamic type ");
					description += "but it has no value inside.";
					return Error(error::fail, description);
				}
				else
				{
					// get the dynamic type name
					type_name = node[i].data.value.Get();
					found_type_name = true;

					// get rid of the current node to not break the loading of pure parameter
					// (parameter node should not contain "dynamic type" child node)
					pure_node.Remove(i);

					// break the loop
					break;
				}
			}
		}

		// check if at least one node contains the dynamic type name
		if (!found_type_name)
		{
			// dynamic type node is allowed to have no name,
			// in this case the dynamic type name must be the first node,
			// and this node must be unnamed.
			if (pure_node.GetNumChildren() > 0 && pure_node[0].data.name.Length() == 0)
			{
				if (pure_node[0].data.value)
				{
					// get the dynamic type name
					type_name = pure_node[0].data.value.Get();

					// get rid of the current node to not break the loading of pure parameter
					// (parameter node should not contain "dynamic type" child node)
					pure_node.Remove(0);
				}
				else
				{
					String description("Text node has unnamed child node ");
					description += "that is expected to contain the dynamic type ";
					description += "name, but it has no value inside.";
					return Error(error::fail, description);
				}
			}
			else
			{
				String description("Text node has child nodes, but none of them ");
				description += "contains the actual dynamic type name.";
				return Error(error::fail, description);
			}
		}

		// get dynamic type by name
		Result<ConstRef<DynamicType>, Error> type_result = Factory<T>::GetType(type_name);
		if (!type_result)
		{
			return Error(type_result.MoveAlt());
		}

		// create new object
		const DynamicType& dyn_type = type_result.GetResult();
		ptr_ref = (T*)dyn_type.CreateInstance();

		// load archive
		Optional<Error> load_archive_error = ptr_ref->Load(pure_node);
		if (load_archive_error)
		{
			return load_archive_error;
		}

		// success
		return Optional<Error>();
	}

	// the name of the attribute text node for the dynamic type name
	static const String skDynamicTypeNodeName;

	// undef macros here
#undef SFINAE_IS_POLYMORPHIC
#undef SFINAE_IS_NOT_POLYMORPHIC
};

//----------------------------------------------------------------------------->
// the name of the attribute text node for the dynamic type name
template <typename T>
const String Parameter< UniquePtr<T> >::skDynamicTypeNodeName = "ut_dynamic_type";

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//