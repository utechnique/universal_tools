//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_pipeline.h"
#include "systems/ui/ve_ui.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
namespace directories
{
	// Configuration files.
	static const char* skCfg = "cfg";
}

//----------------------------------------------------------------------------//
// Generates default pipeline tree.
Pipeline GenDefaultPipeline();

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//