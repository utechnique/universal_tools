//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_resource.h"
#include "systems/render/engine/ve_render_rc_mgr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Null-terminated version of @skStarterChr.
const char Resource::GeneratorPrompt::skStarter[2] = { skStarterChr, 0 };
 
// Checks if the provided text is a generator prompt.
//    @param text - a string to be tested, it must start
//                  with @skStarter to pass the check.
//    @return - the result of the check.
bool Resource::GeneratorPrompt::Check(const ut::String& text)
{
	// every prompt must start with a special symbol
	if (text.Length() < 1 || text[0] != skStarterChr)
	{
		return false;
	}

	return true;
}

// Parses the provided generator prompt into separate attributes.
//    @param prompt - generator prompt.
//    @param out_attributes - an array of attributes to store
//                            parsed attributes in.
//    @return - a name of the resource's type or ut::Error if failed.
ut::Result<ut::String, ut::Error> Resource::GeneratorPrompt::Parse(const ut::String& prompt,
                                                                   Attributes& out_attributes)
{
	// every prompt must start with a special symbol
    if (!Check(prompt))
    {
        return ut::MakeError(ut::error::invalid_arg);
    }

	// split the prompt
	ut::Array<ut::String> elements = prompt.Split(skSeparatorChr0);
	if (elements.IsEmpty())
	{
		return ut::MakeError(ut::error::empty);
	}

	// extract resource type without a special starter symbol
	ut::String rc_type_name(elements.GetFirst().GetAddress() + 1);
	elements.Remove(elements.Begin());

	// parse attributes
	out_attributes.Reset();
	for (const ut::String& element : elements)
	{
		// each attribute must have at least 3 characters: one character for the type,
		// one for the separator and at least one for the value
		if (element.Length() < 3 || element[1] != skValueSeparatorChr)
		{
			continue;
		}

		// the value starts on third character after the type
		// and the separator characters
		ut::String value(element.GetAddress() + 2);
		value.Replace(skSeparatorChr1, skSeparatorChr0);
		value.Replace(skSeparatorChr2, skSeparatorChr1);

		// add parsed attribute data to the final array
		Attribute attribute = { element[0], ut::Move(value) };
		if (!out_attributes.Add(ut::Move(attribute)))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	return rc_type_name;
}

//----------------------------------------------------------------------------//
// Constructor, takes an ownership of the provided resource and
// initializes a counter of references to this resource.
ReferencedResource::ReferencedResource(ResourceManager& rc_mgr,
                                       ut::UniquePtr<Resource> unique_rc,
                                       Resource::Id rc_id,
                                       ut::Optional<ut::String> rc_name) : ptr(ut::Move(unique_rc))
                                                                         , ref_counter(ut::MakeShared<Counter>(rc_mgr, rc_id))
                                                                         , name(ut::Move(rc_name))
{}

//----------------------------------------------------------------------------//
// Counter constructor, takes a reference to the resource manager that
// is used to delete the resource when its reference count becomes zero.
ReferencedResource::Counter::Counter(ResourceManager& rc_mgr,
                                     Resource::Id rc_id) : manager(rc_mgr)
                                                         , id(rc_id)
{}

// Decrements reference count and enqueues a deletion if it becomes zero.
void ReferencedResource::Counter::Decrement()
{
	if (count.Decrement() == 0)
	{
		manager.DeleteResource(id);
	}
}

// Increments reference count.
void ReferencedResource::Counter::Increment()
{
	count.Increment();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//