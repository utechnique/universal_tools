//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_pipeline.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
namespace directories
{
	// Configuration files.
	static const char* skCfg = "config";

	// Resources.
	static const char* skRc = "resources";
	static const char* skRcAlt = "../../../../resources";
}

//----------------------------------------------------------------------------//
// Generates default pipeline tree.
Pipeline GenDefaultPipeline();

// Searches for a file using provided filename, if desired file wasn't found,
// searches for it in ve::skRc and ve::skRcAlt directories and returns the
// final path.
//    @param filename - path to the file.
//    @return - optional path to the file.
ut::Optional<ut::String> FindResourceFile(const ut::String& filename);

// Opens a file using ve::FindResourceFile() function.
//    @param filename - path to the file.
//    @return - opened file or ut::Error if failed.
ut::Result<ut::File, ut::Error> OpenResourceFile(const ut::String& filename);

// Reads data from file using ve::FindResourceFile() function.
//    @param filename - path to the file.
//    @return - contents of the desired file or ut::Error if failed.
ut::Result<ut::Array<ut::byte>, ut::Error> ReadResourceFile(const ut::String& filename);

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//