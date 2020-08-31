//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "client_ui.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(commie::BodyUI, commie::ClientUI, "client_ui")
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// This callback signalizes that message is ready to be sent.
//    @param widget - pointer to the input widget.
//    @param userdata - expected to be a pointer to the ClientUI object.
static void SendMsgCallback(Fl_Widget *widget, void *userdata)
{
	// validate arguments
	UT_ASSERT(widget != nullptr);
	UT_ASSERT(userdata != nullptr);

	// cast types
	Fl_Multiline_Input* input = static_cast<Fl_Multiline_Input*>(widget);
	ClientUI* ui = static_cast<ClientUI*>(userdata);

	// get receiver address
	ut::Result<ut::net::HostAddress, ut::Error> select_result = ui->GetSelectedClientAddress();
	if (select_result)
	{
		// call signal
		ut::String message(input->value());
		ui->message_sent(message, select_result.Get());
	}
	else
	{
		// display error message
		ui->DisplayText(ut::String("[receiver is not selected]") + ut::cret);
	}

	// clear input text
	input->value("");
}
//----------------------------------------------------------------------------//

// border offset of the tile widget
// where the resizable area begins
const int ClientUI::skTileBorder = 80;

// Constructor, parent application must be provided.
// All widgets are initialized here.
//    @param application - reference to parent (owning) application object
ClientUI::ClientUI(Application& application,
                   const Fl_Group* group,
                   float lr_ratio,
                   float tb_ratio)
{
	// calculate all metrics
	int text_width = static_cast<int>(static_cast<float>(group->w()) /
		(lr_ratio + 1.0f));
	int list_width = group->w() - text_width;
	int input_height = static_cast<int>(static_cast<float>(group->h()) /
		(tb_ratio + 1.0f));
	int output_height = group->h() - input_height;

	// start tile widget
	tile = ut::MakeUnique<Fl_Tile>(0, 0, group->w(), group->h());
	border_box = ut::MakeUnique<Fl_Box>(skTileBorder,
	                        skTileBorder,
	                        group->w() - skTileBorder * 2,
	                        group->h() - skTileBorder * 2);
	tile->resizable(border_box.Get());

	// clients list
	clients = ut::MakeUnique<Fl_Hold_Browser>(0, 0, list_width, group->h());
	clients->end();

	// output window
	output_buffer = ut::MakeUnique<Fl_Text_Buffer>();
	output = ut::MakeUnique<Fl_Text_Display>(list_width,
	                                         0,
	                                         text_width,
	                                         output_height);
	output->buffer(output_buffer.Get());
	output->textfont(FL_COURIER);
	output->textsize(14);
	output->end();

	// input window
	input = ut::MakeUnique<Fl_Multiline_Input>(list_width,
	                                           output_height,
	                                           text_width,
	                                           input_height);
	input->textfont(FL_COURIER);
	input->textsize(14);
	input->when(FL_WHEN_ENTER_KEY_ALWAYS);
	input->callback(SendMsgCallback, this);

	// finish the tile widget
	tile->end();

	// init client list
	ClientList empty_list;
	UpdateClientList(empty_list);
}

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& ClientUI::Identify() const
{
	return ut::Identify(this);
}

// Prints provided text to the output box widget.
//    @param text - reference to the string to be displayed.
//    @return - error if failed.
ut::Optional<ut::Error> ClientUI::DisplayText(const ut::String& text)
{
	// validate output widget
	if (!output)
	{
		return ut::Error(ut::error::fail, "Output widget isn't initialized.");
	}

	// print the text
	output->insert(text.GetAddress());
	output->redraw();

	// update vertical scroll position
	Fl_Text_Buffer* text_buffer = output->buffer();
	output->scroll(text_buffer->count_lines(0, text_buffer->length()), 0);

	// success
	return ut::Optional<ut::Error>();
}

// Updates client list widget.
//    @param list - list of clients.
//    @return - error if failed.
ut::Optional<ut::Error> ClientUI::UpdateClientList(const ClientList& list)
{
	// clear widget
	int selected_item = clients->value() - 2; // -1 for 'everybody'
	clients->clear();

	// get selected client address and copy new list
	ut::net::HostAddress selected_address;
	if (selected_item >= 0)
	{
		selected_address = client_list[selected_item].address;
	}
	client_list = list;

	// update widget
	clients->add("everybody");
	clients->select(1);
	for (size_t i = 0; i < list.GetNum(); i++)
	{
		// add new line
		clients->add(list[i].name);

		// select item if it was previously selected (in old list)
		if (selected_item >= 0 && list[i].address == selected_address)
		{
			// widget starts enumeration from 1 (not 0) also 'everybody' item
			// is the first in a list, so we need to add '2' to final id
			clients->select(static_cast<int>(i + 2));
		}
	}

	// success
	return ut::Optional<ut::Error>();
}

// Returns the address of the selected client in client-list widget.
ut::Result<ut::net::HostAddress, ut::Error> ClientUI::GetSelectedClientAddress()
{
	// get selected item id
	int selected_item = clients->value() - 1;

	// validate id
	if (selected_item < 0)
	{
		return ut::MakeError(ut::error::fail);
	}

	// check if selected item is 'everybody'
	if (selected_item == 0)
	{
		return ut::net::HostAddress();
	}
	else
	{
		selected_item--;
	}

	// check if selected item is in client-list range
	if (selected_item >= client_list.GetNum())
	{
		return ut::MakeError(ut::error::out_of_bounds);
	}

	// return the address of the selected client
	return client_list[selected_item].address;
}

// Ratio of the client list widget width and output widget width.
float ClientUI::GetHorizontalRatio() const
{
	float width = static_cast<float>(clients->w());
	return width / static_cast<float>(output->w());
}

// Ratio of the output widget height and input widget height.
float ClientUI::GetVerticalRatio() const
{
	return static_cast<float>(output->h()) / static_cast<float>(input->h());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//