//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// This component indicates that owning entity was selected in editor.
class SelectedInEditorComponent : public Component
{
public:
	// Explicitly declare defaulted constructors and move operator.
	SelectedInEditorComponent() = default;
	SelectedInEditorComponent(SelectedInEditorComponent&&) = default;
	SelectedInEditorComponent& operator =(SelectedInEditorComponent&&) = default;

	// Copying is prohibited.
	SelectedInEditorComponent(const SelectedInEditorComponent&) = delete;
	SelectedInEditorComponent& operator =(const SelectedInEditorComponent&) = delete;

	// Meta routine.
	const ut::DynamicType& Identify() const;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//