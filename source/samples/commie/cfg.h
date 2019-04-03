//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "commie_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::UiCfg is serializable part of commie::Configuration that is related
// to user interface preferences.
class UiCfg : public ut::Reflective
{
public:
	// Constructor, default values are set here.
	UiCfg();

	// Registers named parameters to the intermediate serialization registry.
	//    @param registry - register parameters here,
	//                      just call registry.Add(parameter, "desired_name");
	void Register(ut::MetaRegistry& registry);

#if COMMIE_DESKTOP
	ut::uint32 position_x; // left coordinate of the window
	ut::uint32 position_y; // top coordinate of the window
	ut::uint32 width; // width of the window
	ut::uint32 height; // height of the window
	float lr_ratio; // ratio of the client-list and output-box widgets' width.
	float tb_ratio; // ratio of the output-box and input-box widgets' height.
#endif // COMMIE_DESKTOP
};

class Configuration : public ut::Reflective
{
public:
	// Constructor, default values are set here.
	Configuration();

	// Registers named parameters to the intermediate serialization registry.
	//    @param registry - register parameters here,
	//                      just call registry.Add(parameter, "desired_name");
	void Register(ut::MetaRegistry& registry);

	// Whether the node will be launched as a server.
	bool server_mode;

	// Address of the connection.
	ut::net::HostAddress address;

	// Password for authorization on server.
	ut::String authorization_password;

	// Key to encrypt messages.
	ut::String encryption_key;

	// Name of the client.
	ut::String name;

	// Whether to open a console window.
	bool console;

	// user interface
	UiCfg ui;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//