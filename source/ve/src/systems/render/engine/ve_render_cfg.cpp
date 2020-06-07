//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_cfg.h"
#include "ve_default.h"
//----------------------------------------------------------------------------//
template<> const char* ve::Config<ve::render::Settings>::skName = "render";
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : vsync(true)
{}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(vsync, "vertical_synchronization");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
