//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "body_ui.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::ServerUI is a container for UI widgets that are specific to server
// application.
class ServerUI : public BodyUI
{
public:
	// Inform UT that this class has no default constructor.
	UT_NO_DEFAULT_CONSTRUCTOR

	// Constructor, parent application must be provided.
	// All widgets are initialized here.
	//    @param application - reference to parent (owning) application object
	ServerUI(Application& application, Fl_Group* group);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Prints provided text to the output box widget.
	//    @param text - reference to the string to be displayed.
	//    @return - error if failed.
	ut::Optional<ut::Error> DisplayText(const ut::String& text);

private:
	// server-mode title
	ut::UniquePtr<Fl_Box> title_box;

	// event log window
	ut::UniquePtr<Fl_Text_Buffer> output_buffer;
	ut::UniquePtr<Fl_Text_Display> output;

	// height of the title
	static const int skToolbarHeight;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//