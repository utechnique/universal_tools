//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// v::render::ShaderCache class stores compiled shaders in a cache so that
// the next time shader will be loaded no recompilation would be needed.
class ShaderCache : public ut::meta::Reflective
{
public:
	// Constructor.
	ShaderCache();

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	// Loads cache from the @path file.
	ut::Optional<ut::Error> Load();

	// Save cache to the @path file.
	ut::Optional<ut::Error> Save();

	// Compiles a shader from file. Returns cached version if source didn't
	// change or compiles it from the scratch otherwise.
	//    @param stage - type of the shader (vertex/pixel/geometry etc.).
	//    @param shader_name - string with the name of this
	//                         particular shader build.
	//    @param entry_point - string with a name of entry point.
	//    @param filename - const string with the name of shader file, can be
	//                      relative to ve::directories::skRc directory.
	//    @param macros - preprocessor macros to build shader with.
	//    @return - Shader::Info object or ut::Error if failed.
	ut::Result<Shader::Info, ut::Error> CompileFromFile(Shader::Stage stage,
	                                                    ut::String shader_name,
	                                                    ut::String entry_point,
	                                                    const ut::String& filename,
	                                                    Shader::Macros macros = Shader::Macros());

	// Searches if given shader is present in cache.
	//    @param stage - type of the shader (vertex/pixel/geometry etc.).
	//    @param shader_name - unique name of the shader.
	//    @param entry_point - string with a name of entry point.
	//    @param macros - preprocessor macros used to build shader.
	//    @return - optional reference to the ve::Shader::Info object.
	ut::Optional<Shader::Info&> Find(Shader::Stage stage,
	                                 const ut::String& shader_name,
	                                 const ut::String& entry_point,
	                                 const Shader::Macros& macros);

private:
	// cache name
	static const char* skMetaName;

	// cache filename
	static const char* skFileName;

	// protects Find() and CompileFromFile() methods
	ut::Mutex mutex;

	// path to the cache file
	ut::String path;

	// default relative resource directory path to search shaders in
	ut::String shader_directory;

	// cached shaders, key is a name of the shader
	ut::HashMap<ut::String, Shader::Info> cache;

	// compiler recompiles shaders if hash changed
	ShaderCompiler compiler;

	// ve::ShaderCache::Reader is a helper struct to read shader files
	// and processs '#include' directives.
	struct Reader
	{
		// Reads shader file and processes "#include" directives.
		//    @param filename - const pointer to the shader filename, it can be 
		//                      relative, absolute or relative to ve::directories::skRc
		//                      and ve::directories::skRcAlt directories.
		//    @return - string with the final shader text or ut::Error if failed.
		static ut::Result<ut::String, ut::Error> ReadShaderFileText(const ut::String& filename);

		// Searches for the "#include" directives in a shader code and replaces
		// them with the code from correspoding files.
		//    @param text - shader text.
		//    @param directory - const pointer to the parent directory where
		//                       included files can be found.
		//    @return - shader text with included files or ut::Error if failed.
		static ut::Result<ut::String, ut::Error> ProcessIncludes(ut::String text,
																 const ut::String& directory);

		// Processes comments, reads a line and returns it's length in characters.
		//    @param str - a pointer to the text to read a line from.
		//    @return - length of the line or ut::Error if parsing error occurred.
		static ut::Result<size_t, ut::Error> ReadLine(const char* str);

		// Returns a number of whitespace characters in the beginning of the text.
		//    @param str - a pointer to the text containing whitespaces.
		//    @param size - size of the text in characters.
		//    @return - number of whitespace characters in front of the @str.
		static size_t SkipWhitespaces(const char* str, size_t size);

		// Reads a name of the file inside '""' or '<>' parentheses.
		//    @param str - a pointer to the string.
		//    @param size - size of the @str in characters.
		//    @return - filename string or ut::Error if parsing error occurred.
		static ut::Result<ut::String, ut::Error> ReadIncludeFilename(const char* str,
																	 size_t size);
	};
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
