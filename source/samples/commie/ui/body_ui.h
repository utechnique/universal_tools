//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::BodyUI is a base class for commie::ClientUI and commie::ServerUI
// classes.
class BodyUI : public ut::Polymorphic, public ut::NonCopyable
{
public:
	// Identify() method must be implemented for the polymorphic types.
	virtual const ut::DynamicType& Identify() const = 0;

	// Virtual destructor to fulfill polymorphic behaviour.
	virtual ~BodyUI() {};

	// Prints provided text to the output box widget.
	//    @param text - reference to the string to be displayed.
	//    @return - error if failed.
	virtual ut::Optional<ut::Error> DisplayText(const ut::String& text) = 0;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//