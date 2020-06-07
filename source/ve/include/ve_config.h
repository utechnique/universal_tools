//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_default.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::Config is a class to save/load options and preferences.
template<typename Properties>
class Config
{
public:
	// Inheritance operator, provides access to the owned object.
	Properties* operator -> ()
	{
		return &properties;
	}

	// Inheritance operator, provides read access to the owned object.
	const Properties* operator -> () const
	{
		return &properties;
	}

	// Saves to file.
	ut::Optional<ut::Error> Save()
	{
		// create cfg directory if it doesn't exist
		ut::CreateFolder(directories::skCfg);

		// open file for writing
		ut::File cfg_file;
		ut::Optional<ut::Error> open_error = cfg_file.Open(GetPath(), ut::file_access_write);
		if (open_error)
		{
			return open_error;
		}
		else
		{
			ut::JsonDoc json_doc;
			ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(properties,
																		  skName,
																		  ut::meta::Info::CreatePure());
			cfg_file << (json_doc << cfg_snapshot);
			cfg_file.Close();
			ut::log << "Config file was updated (" << skName << ")" << ut::cret;
		}

		// success
		return ut::Optional<ut::Error>();
	}

	// Loads from file.
	ut::Optional<ut::Error> Load()
	{
		// open file
		ut::File cfg_file;
		ut::Optional<ut::Error> open_cfg_error = cfg_file.Open(GetPath(),
		                                                       ut::file_access_read);
		if (open_cfg_error)
		{
			return ut::Error(ut::error::no_such_file);
		}

		// deserialize cfg from the text document
		ut::JsonDoc json_doc;
		ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(properties, skName);
		cfg_file >> json_doc >> cfg_snapshot;
		cfg_file.Close();

		// success
		return ut::Optional<ut::Error>();
	}

	// Generates full local path to the configuration file.
	static ut::String GetPath()
	{
		return ut::String(directories::skCfg) + ut::fsep + skName + ".json";
	}

	// name of the managed entity
	static const char* skName;

private:
	Properties properties;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//