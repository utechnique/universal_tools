//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
template<> const char* ve::Config<ve::ui::Settings>::skName = "ui";
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : position_x(0)
                     , position_y(0)
                     , width(640)
                     , height(480)
{}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(position_x, "position_x");
	snapshot.Add(position_y, "position_y");
	snapshot.Add(width, "width");
	snapshot.Add(height, "height");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
