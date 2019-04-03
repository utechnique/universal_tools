//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "body_ui.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::ClientUI is a container for UI widgets that are specific to client
// application.
class ClientUI : public BodyUI
{
public:
	// Inform UT that this class has no default constructor.
	UT_NO_DEFAULT_CONSTRUCTOR

	// Constructor, parent application must be provided.
	// All widgets are initialized here.
	//    @param application - reference to parent (owning) application object
	ClientUI(Application& application,
	         const Fl_Group* group,
	         float lr_ratio,
	         float tb_ratio);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Prints provided text to the output box widget.
	//    @param text - reference to the string to be displayed.
	//    @return - error if failed.
	ut::Optional<ut::Error> DisplayText(const ut::String& text);

	// Updates client list widget.
	//    @param list - list of clients.
	//    @return - error if failed.
	ut::Optional<ut::Error> UpdateClientList(const ClientList& list);

	// Returns the address of the selected client in client-list widget.
	ut::Result<ut::net::HostAddress, ut::Error> GetSelectedClientAddress();

	// Ratio of the client list widget width and output widget width.
	float GetHorizontalRatio() const;

	// Ratio of the output widget height and input widget height.
	float GetVerticalRatio() const;

	// Signal when user presses 'ENTER' in the input widget.
	ut::Signal<void(const ut::String&, const ut::net::HostAddress&)> message_sent;

private:
	// tile widget for resizing input/output boxes
	// and the client list window
	ut::UniquePtr<Fl_Tile> tile;

	// box to set the range of the tile widget
	ut::UniquePtr<Fl_Box> border_box;

	// client list
	ut::UniquePtr<Fl_Hold_Browser> clients;

	// output (chat-log) window
	ut::UniquePtr<Fl_Text_Buffer> output_buffer;
	ut::UniquePtr<Fl_Text_Display> output;

	// input window
	ut::UniquePtr<Fl_Multiline_Input> input;

	// list of the clients
	ClientList client_list;

	// border offset of the tile widget
	// where the resizable area begins
	static const int skTileBorder;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//