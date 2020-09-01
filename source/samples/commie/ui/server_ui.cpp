//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "server_ui.h"
#include <FL/Fl.H>
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(commie::BodyUI, commie::ServerUI, "server_ui")
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// height of the toolbar
const int ServerUI::skToolbarHeight = 60;

// Constructor, parent application must be provided.
// All widgets are initialized here.
//    @param application - reference to parent (owning) application object
ServerUI::ServerUI(Application& application, Fl_Group* group)
{
	// title
	title_box = ut::MakeUnique<Fl_Box>(FL_FRAME_BOX, 0, 0, group->w(), skToolbarHeight, "Server mode");
	title_box->color(FL_GRAY);
	title_box->labelsize(24);
	title_box->labelfont(FL_BOLD);
	title_box->labelcolor(FL_DARK_GREEN);
	title_box->labeltype(FL_ENGRAVED_LABEL);

	// log output
	output_buffer = ut::MakeUnique<Fl_Text_Buffer>();
	output = ut::MakeUnique<Fl_Text_Display>(0, title_box->h(), group->w(), group->h() - title_box->h());
	output->buffer(output_buffer.Get());
	output->textfont(FL_COURIER);
	output->textsize(14);
	output->end();

	// set output widget as a main resizable element
	group->resizable(output.Get());
}

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& ServerUI::Identify() const
{
	return ut::Identify(this);
}

// Prints provided text to the output box widget.
//    @param text - reference to the string to be displayed.
//    @return - error if failed.
ut::Optional<ut::Error> ServerUI::DisplayText(const ut::String& text)
{
	// validate outpur widget
	if (!output)
	{
		return ut::Error(ut::error::fail, "Output widget isn't initialized.");
	}

	// print the text
	output->insert(text.ToCStr());
	output->redraw();

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//