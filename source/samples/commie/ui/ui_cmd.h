//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie_common.h"
#include "body_ui.h"
#include "client_ui.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::UiCmd is a base class for UI commands. UI commands are needed to
// make possible operating with widgets from different threads, so that final
// actions will be performed only in the main thread. Otherwise FLTK behaves
// buggy (if using numerous Fl::lock() - Fl::unlock() sections).
class UiCmd
{
public:
	virtual void Execute() = 0;
};

//----------------------------------------------------------------------------//
// Displays provided text in the output widget of the provided UI object.
class DisplayTextCmd : public UiCmd
{
public:
	// Constructor
	//    @param in_ui - reference to the UI object.
	//    @param in_text - cons reference to the text string.
	DisplayTextCmd(BodyUI& in_ui, const ut::String& in_text);

	// Copy constructor
	DisplayTextCmd(const DisplayTextCmd& copy);

	// Just calls DisplayText() method of the provided UI object.
	void Execute();

private:
	BodyUI& ui;
	ut::String text;
};

//----------------------------------------------------------------------------//
// Updates client list of the provided UI object.
class UpdateClientListCmd : public UiCmd
{
public:
	// Constructor
	//    @param in_ui - reference to the UI object.
	//    @param in_list - cons reference to the list to update with.
	UpdateClientListCmd(ClientUI& in_ui, const ClientList& in_list);

	// Copy constructor
	UpdateClientListCmd(const UpdateClientListCmd& copy);

	// Just calls UpdateClientList() method of the provided UI object.
	void Execute();

private:
	ClientUI& ui;
	ClientList list;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//