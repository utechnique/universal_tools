//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "cfg.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Constructor, default values are set here.
UiCfg::UiCfg()
{
#if COMMIE_DESKTOP
	position_x = 0;
	position_y = 0;
	width = 640;
	height = 480;
	lr_ratio = 0.25f;
	tb_ratio = 2.0f;
#endif // COMMIE_DESKTOP
}

// Registers named parameters to the intermediate serialization registry.
//    @param registry - register parameters here,
//                      just call registry.Add(parameter, "desired_name");
void UiCfg::Register(ut::MetaRegistry& registry)
{
#if COMMIE_DESKTOP
	registry.Add(position_x, "position_x");
	registry.Add(position_y, "position_y");
	registry.Add(width, "width");
	registry.Add(height, "height");
	registry.Add(lr_ratio, "lr_ratio");
	registry.Add(tb_ratio, "tb_ratio");
#endif // COMMIE_DESKTOP
}

//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Configuration::Configuration() : server_mode(false)
                               , address("127.0.0.1", 4443)
                               , authorization_password("auth_pass")
                               , encryption_key("enc_key")
                               , name("user")
                               , console(false)
{

}

// Registers named parameters to the intermediate serialization registry.
//    @param registry - register parameters here,
//                      just call registry.Add(parameter, "desired_name");
void Configuration::Register(ut::MetaRegistry& registry)
{
	registry.Add(server_mode, "server_mode");
	registry.Add(address.ip, "ip");
	registry.Add(address.port, "port");
	registry.Add(authorization_password, "password");
	registry.Add(encryption_key, "encryption_key");
	registry.Add(name, "name");
	registry.Add(console, "console");
	registry.Add(ui, "ui");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//