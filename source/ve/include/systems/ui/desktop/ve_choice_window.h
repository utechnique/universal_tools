//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_window.h"
#include "systems/ui/desktop/ve_scroll.h"
#include "systems/ui/desktop/ve_button.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Choice windows presents a set of variants to user and remembers the choice.
class ChoiceWindow : public Window
{
public:
	// Constructor. Creates a set of buttons for each variant.
	//    @param x - horizontal coordinate of the upper left corner of the window.
	//    @param y - vertical coordinate of the upper left corner of the window.
	//    @param w - width of the window in pixels.
	//    @param h - height of the window in pixels.
	//    @param title - string with window title.
	//    @param variants - const reference to the array of strings, each string
	//                      represents a variant that can be chosen.
	//    @param theme - reference to the color theme of the window.
	ChoiceWindow(int x,
	             int y,
	             ut::uint32 w,
	             ut::uint32 h,
	             ut::String title,
	             const ut::Array<ut::String>& variants,
	             const Theme& theme = Theme());
	
	// Returns the index of the selected variant or nothing if canceled.
	ut::Optional<ut::uint32> GetResult() const;

	// Height of the window caption in pixels.
	static const ut::uint32 skCapHeight;

private:
	struct VariantId
	{
		ut::uint32 value;
		ut::uint32 Read() const { return value; }
	};

	ut::UniquePtr<Scroll> scroll;
	ut::Array<VariantId> variant_id;
	ut::Array< ut::UniquePtr<Button> > variant_buttons;
	ut::Optional<ut::uint32> result;
};

// Shows a modal (blocking) window with the set of variants. User must choose one
// of them.
//    @param x_position - horizontal coordinate of the upper left corner of the window.
//    @param y_position - vertical coordinate of the upper left corner of the window.
//    @param width - width of the window in pixels.
//    @param height - height of the window in pixels.
//    @param title - string with window title.
//    @param variants - const reference to the array of strings, each string
//                      represents a variant that can be chosen.
//    @param theme - reference to the color theme of the window.
//    @return - id of the chosen variant or nothing if canceled.
ut::Optional<ut::uint32> SelectInDialogWindow(int x_position,
                                              int y_position,
                                              ut::uint32 width,
                                              ut::uint32 height,
                                              ut::String title,
                                              const ut::Array<ut::String>& variants,
                                              const Theme& theme = Theme());

// Asks the user to choose one of the types derived from the template argument in the
// dialog window.
//    @param x_position - horizontal coordinate of the upper left corner of the window.
//    @param y_position - vertical coordinate of the upper left corner of the window.
//    @param width - width of the window in pixels.
//    @param height - height of the window in pixels.
//    @param title - string with window title.
//    @param theme - reference to the color theme of the window.
//    @return - id of the chosen variant or nothing if canceled.
template<class BaseType>
ut::Optional<const ut::DynamicType&> SelectDerivedTypeInDialogWindow(int x_position,
                                                                     int y_position,
                                                                     ut::uint32 width,
                                                                     ut::uint32 height,
                                                                     ut::String title,
                                                                     const Theme& theme = Theme())
{
	ut::Array<ut::String> type_names;

	const size_t type_count = ut::Factory<BaseType>::CountTypes();
	for (size_t i = 0; i < type_count; i++)
	{
		const ut::DynamicType& dynamic_type = ut::Factory<BaseType>::GetTypeByIndex(i);
		type_names.Add(dynamic_type.GetName());
	}

	const ut::Optional<ut::uint32> type_id = SelectInDialogWindow(x_position,
	                                                              y_position,
	                                                              width,
	                                                              height,
	                                                              ut::Move(title),
	                                                              type_names,
	                                                              theme);
	if (type_id)
	{
		return ut::Factory<BaseType>::GetTypeByIndex(type_id.Get());
	}

	return ut::Optional<const ut::DynamicType&>();
}


//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//