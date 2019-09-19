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
//                      just call snapshot.Add(parameter, "desired_name");
void UiCfg::Reflect(ut::meta::Snapshot& snapshot)
{
#if COMMIE_DESKTOP
	snapshot.Add(position_x, "position_x");
	snapshot.Add(position_y, "position_y");
	snapshot.Add(width, "width");
	snapshot.Add(height, "height");
	snapshot.Add(lr_ratio, "lr_ratio");
	snapshot.Add(tb_ratio, "tb_ratio");
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
//                      just call snapshot.Add(parameter, "desired_name");
void Configuration::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(server_mode, "server_mode");
	snapshot.Add(address.ip, "ip");
	snapshot.Add(address.port, "port");
	snapshot.Add(authorization_password, "password");
	snapshot.Add(encryption_key, "encryption_key");
	snapshot.Add(name, "name");
	snapshot.Add(console, "console");
	snapshot.Add(ui, "ui");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//