//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ui_cmd.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::DisplayTextCmd displays provided text in the output widget of the
// provided UI object.
//
// Constructor
//    @param in_ui - reference to the UI object.
//    @param in_text - cons reference to the text string.
DisplayTextCmd::DisplayTextCmd(BodyUI& in_ui,
                               const ut::String& in_text) : ui(in_ui)
                                                          , text(in_text)
{ }

// Copy constructor
DisplayTextCmd::DisplayTextCmd(const DisplayTextCmd& copy) : ui(copy.ui)
                                                           , text(copy.text)
{ }

// Just calls DisplayText() method of the provided UI object.
void DisplayTextCmd::Execute()
{
	ui.DisplayText(text);
}

//----------------------------------------------------------------------------//
// commie::UpdateClientListCmd updates client list of the provided UI object.
// 
// Constructor
//    @param in_ui - reference to the UI object.
//    @param in_list - cons reference to the list to update with.
UpdateClientListCmd::UpdateClientListCmd(ClientUI& in_ui,
                                         const ClientList& in_list) : ui(in_ui)
                                                                    , list(in_list)
{ }

// Copy constructor
UpdateClientListCmd::UpdateClientListCmd(const UpdateClientListCmd& copy) : ui(copy.ui)
                                                                          , list(copy.list)
{ }

// Just calls UpdateClientList() method of the provided UI object.
void UpdateClientListCmd::Execute()
{
	ui.UpdateClientList(list);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//