//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie_common.h"
#include "../cfg.h"
#include "../client_list.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::UI is a base class for all UI containers. Every platform-specific
// UI container must be derived from this class.
class UI
{
public:
	// Constructor
	UI();

	// Destructor
	virtual ~UI();

	virtual void Run() = 0;

	// Synchronizes internal UI data, options and preferences with
	// main application. Just save your data into provided cfg reference.
	//    @param cfg - reference to the configuration object
	//                 of the main application
	virtual void SyncCfg(Configuration & cfg) = 0;

	// Prints provided text to the output box widget.
	//    @param text - reference to the string to be displayed.
	//    @return - error if failed.
	virtual ut::Optional<ut::Error> DisplayText(const ut::String& text) = 0;

	// Updates client list widget.
	//    @param list - list of clients.
	//    @return - error if failed.
	virtual ut::Optional<ut::Error> UpdateClientList(const ClientList& list) = 0;

	// Signal when user presses 'ENTER' in the input widget.
	ut::Signal<void(const ut::String&, const ut::net::HostAddress&, bool)> message_sent;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//