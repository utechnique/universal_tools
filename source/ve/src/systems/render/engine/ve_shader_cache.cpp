//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_shader_cache.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
const char* ShaderCache::skMetaName = "shader_cache";
#if VE_VULKAN
const char* ShaderCache::skFileName = "shaders/vulkan_cache";
#elif VE_DX11
const char* ShaderCache::skFileName = "shaders/dx11_cache";
#endif

//----------------------------------------------------------------------------//
// Constructor.
ShaderCache::ShaderCache() : path(ut::String(directories::skRc) + ut::skFileSeparator + skFileName)
                           , shader_directory(ut::String("shaders") + ut::skFileSeparator)
{}

//----------------------------------------------------------------------------->
// Registers children into reflection tree.
//    @param snapshot - reference to the reflection tree
void ShaderCache::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(cache, "shaders");
}

//----------------------------------------------------------------------------->
// Loads cache from the @path file.
ut::Optional<ut::Error> ShaderCache::Load()
{
	// capture meta snapshot
	ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(*this, skMetaName,
	                                                          ut::meta::Info::CreateComplete());

	// open file for reading
	ut::File file;
	ut::Optional<ut::Error> open_error = file.Open(path, ut::file_access_read);
	if (open_error)
	{
		ut::log.Lock() << "Shader cache: failed to open file " << path << ut::cret;
		return open_error;
	}

	// save to cache to file
	return snapshot.Load(file);
}

//----------------------------------------------------------------------------->
// Save cache to the @path file.
ut::Optional<ut::Error> ShaderCache::Save()
{
	// capture meta snapshot
	ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(*this, skMetaName,
	                                                          ut::meta::Info::CreateComplete());

	// create directory if it doesn't exist
	ut::CreateDirectories(path.GetIsolatedLocation(false));

	// open file for writing
	ut::File file;
	ut::Optional<ut::Error> open_error = file.Open(path, ut::file_access_write);
	if (open_error)
	{
		ut::log.Lock() << "Shader cache: failed to save file " << path << ut::cret;
		return open_error;
	}

	// save to cache to file
	return snapshot.Save(file);
}

//----------------------------------------------------------------------------->
// Compiles a shader from file. Returns cached version if source didn't
// change or compiles it from the scratch otherwise.
//    @param stage - type of the shader (vertex/pixel/geometry etc.)
//    @param shader_name - string with the name of this
//                         particular shader build.
//    @param entry_point - string with a name of entry point.
//    @param filename - const string with the name of shader file, can be
//                      relative to ve::directories::skRc directory.
//    @param macros - preprocessor macros to build shader with.
//    @return - Shader::Info object or ut::Error if failed.
ut::Result<Shader::Info, ut::Error> ShaderCache::CompileFromFile(Shader::Stage stage,
	                                                             ut::String shader_name,
	                                                             ut::String entry_point,
	                                                             const ut::String& filename,
	                                                             Shader::Macros macros)
{
	// read shader file
	ut::Result<ut::String, ut::Error> shader_text = Reader::ReadShaderFileText(shader_directory + filename);

	// search for this shader in the cache
	ut::Optional<Shader::Info&> find_result = Find(stage, shader_name, entry_point, macros);
	if (find_result)
	{
		// check if shader file was opened
		if (!shader_text)
		{
			// file wasn't found, but there is a shader in the cache, it's ok
			ut::log.Lock() << "Shader cache: \"" << shader_name
			               << "\" can\'t be loaded from file \"" << filename
			               << "\", loading cached version."<< ut::cret;
			return Shader::Info(find_result.Get()); // copy
		}

		// check hash
		if (find_result->hash == ut::Sha256(shader_text.GetResult()))
		{
			ut::log.Lock() << "Shader cache: full match for \"" << shader_name
			               << "\", loaded cached version." << ut::cret;
			return Shader::Info(find_result.Get()); // copy
		}	
		else
		{
			ut::log.Lock() << "Shader cache: invalid hash for \"" << shader_name
			               << "\", must be recompiled." << ut::cret;
		}
	}

	// check if shader file was opened successfully - otherwise it's an error
	if (!shader_text)
	{
		ut::log.Lock() << "Shader cache: failed to load \"" << shader_name
			           << "\" - there is no file \"" << filename
			           << "\" and nothing in cache."<< ut::cret;
		return ut::MakeError(shader_text.MoveAlt());
	}

	// if found nothing in the cache - compile
	ut::log.Lock() << "Shader cache: compiling \"" << shader_name
	               << "\" from file \"" << filename << "\"..." << ut::cret;
	ut::Result<Shader::Info, ut::Error> compile_result = compiler.Compile(stage,
	                                                                      ut::Move(shader_name),
	                                                                      ut::Move(entry_point),
	                                                                      shader_text.GetResult(),
	                                                                      ut::Move(macros));
	if (!compile_result)
	{
		return compile_result;
	}

	// calculate hash
	Shader::Info& info = compile_result.GetResult();
	info.hash = ut::Sha256(shader_text.GetResult());

	// remove previous version
	cache.Remove(info.name);

	// add to the cache
	if (!cache.Insert(info.name, info))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// success
	return compile_result.MoveResult();
}

//----------------------------------------------------------------------------->
// Searches if given shader is present in cache.
//    @param stage - type of the shader (vertex/pixel/geometry etc.).
//    @param shader_name - unique name of the shader.
//    @param entry_point - string with a name of entry point.
//    @param macros - preprocessor macros used to build shader.
//    @return - optional reference to the ve::Shader::Info object.
ut::Optional<Shader::Info&> ShaderCache::Find(Shader::Stage stage,
                                              const ut::String& shader_name,
                                              const ut::String& entry_point,
                                              const Shader::Macros& macros)
{
	// search by name
	ut::Optional<Shader::Info&> info = cache.Find(shader_name);
	if (!info)
	{
		ut::log.Lock() << "Shader cache: \"" << shader_name << "\" wasn't found in the cache." << ut::cret;
		return ut::Optional<Shader::Info&>();
	}

	// check stage
	if (info->stage != stage)
	{
		ut::log.Lock() << "Shader cache: cached version of \"" << shader_name << "\" has another shader type." << ut::cret;
		return ut::Optional<Shader::Info&>();
	}

	// check entry point
	if (info->entry_point != entry_point)
	{
		ut::log.Lock() << "Shader cache: cached version of \"" << shader_name << "\" has another entry point." << ut::cret;
		return ut::Optional<Shader::Info&>();
	}

	// check if macros number match
	const size_t macros_count = info->macros.GetNum();
	if (macros_count != macros.GetNum())
	{
		ut::log.Lock() << "Shader cache: cached version of \"" << shader_name << "\" has macros count mismatch." << ut::cret;
		return ut::Optional<Shader::Info&>();
	}

	// check if macros match
	for (size_t i = 0; i < macros_count; i++)
	{
		if (info->macros[i].name != macros[i].name || info->macros[i].value != macros[i].value)
		{
			ut::log.Lock() << "Shader cache: cached version of \"" << shader_name << "\" has at least one mismatched macro value." << ut::cret;
			return ut::Optional<Shader::Info&>();
		}
	}

	// success
	return info;
}

//----------------------------------------------------------------------------->
// Reads shader file and processes "#include" directives.
//    @param filename - const pointer to the shader filename, it can be 
//                      relative, absolute or relative to ve::directories::skRc
//                      and ve::directories::skRcAlt directories.
//    @return - string with the final shader text or ut::Error if failed.
ut::Result<ut::String, ut::Error> ShaderCache::Reader::ReadShaderFileText(const ut::String& filename)
{
	// open a file
	ut::File file;

	// try file path as is
	ut::String path = filename;
	ut::Optional<ut::Error> open_file_error = file.Open(path, ut::file_access_read);
	if (open_file_error)
	{
		// if didn't work - try default resources directory
		path = ut::String(directories::skRc) + ut::skFileSeparator + filename;
		open_file_error = file.Open(path, ut::file_access_read);
		if (open_file_error)
		{
			// finally try alternative resources directory
			path = ut::String(directories::skRcAlt) + ut::skFileSeparator + filename;
			open_file_error = file.Open(path, ut::file_access_read);
			if (open_file_error)
			{
				ut::log.Lock() << "Shader cache: file " << filename << " not found." << ut::cret;
				return ut::MakeError(open_file_error.Move());
			}
		}
	}

	// get file size
	ut::Result<size_t, ut::Error> get_size_result = file.GetSize();
	if (!get_size_result)
	{
		ut::log.Lock() << "Shader cache: can't read shader file size " << filename << ut::cret;
		return ut::MakeError(get_size_result.MoveAlt());
	}
	size_t file_size = get_size_result.GetResult();

	// read file content to the string
	ut::String text(file_size);
	ut::Optional<ut::Error> read_error = file.Read(text.GetAddress(), 1, file_size);
	if (read_error)
	{
		ut::log.Lock() << "Shader cache: can't read shader " << filename << ut::cret;
		return ut::MakeError(read_error.Move());
	}

	// close the file
	file.Close();

	// include all files to the final shader code
	ut::Result<ut::String, ut::Error> final_code = ProcessIncludes(ut::Move(text), path.GetIsolatedLocation(true));
	if (!final_code)
	{
		ut::log.Lock() << "Shader cache: error parsing includes in shader " << filename << ut::cret;
		return ut::MakeError(final_code.MoveAlt());
	}

	// success
	return final_code.MoveResult();
}

//----------------------------------------------------------------------------->
// Searches for the "#include" directives in a shader code and replaces
// them with the code from correspoding files.
//    @param text - shader text.
//    @param directory - const pointer to the parent directory where
//                       included files can be found.
//    @return - shader text with included files or ut::Error if failed.
ut::Result<ut::String, ut::Error> ShaderCache::Reader::ProcessIncludes(ut::String text,
                                                                       const ut::String& directory)
{
	ut::String out(ut::Move(text));
	const char* cursor = out.GetAddress();
	const char* next_line;

	// parse text line by line
	while (true)
	{
		// calculate a number of characters in the current line
		ut::Result<size_t, ut::Error> read_line_result = ReadLine(cursor);
		if (!read_line_result)
		{
			return ut::MakeError(read_line_result.MoveAlt());
		}

		// calculate offset to the next line
		next_line = cursor + read_line_result.GetResult();
		if (next_line == cursor)
		{
			// end of the file
			break;
		}

		// remember pointer to the current line
		const char* const line_start = cursor;

		// skip whitespaces in the beginning of the line
		cursor += SkipWhitespaces(cursor, next_line - cursor);

		// '#' char indicates a start of preprocessor directive
		bool preprocessor = false;
		if (*cursor++ == '#')
		{
			cursor += SkipWhitespaces(cursor, next_line - cursor);
			preprocessor = true;
		}

		// read include statement (it can occur only in the beginning of the line)
		const char* include_keyword = "include";
		const size_t keyword_size = ut::StrLen(include_keyword);

		// check if include directive present
		bool has_include = preprocessor && (static_cast<size_t>(next_line - cursor) > keyword_size);
		for (size_t i = 0; has_include && (i < keyword_size); i++)
		{
			if (*cursor++ != include_keyword[i])
			{
				has_include = false;
				break;
			}
		}

		// replace include directive with a text from included file
		if (has_include)
		{
			// skip whitespaces before '"' or '<' opening character
			cursor += SkipWhitespaces(cursor, next_line - cursor);

			// read included filename
			ut::Result<ut::String, ut::Error> include_file = ReadIncludeFilename(cursor, next_line - cursor);
			if (!include_file)
			{
				ut::String invalid_statement(cursor, next_line - cursor);
				ut::log.Lock() << "Shader cache: can't parse " << invalid_statement << ut::cret;
				return ut::MakeError(ut::error::fail);
			}

			// read include file
			ut::Result<ut::String, ut::Error> include_result = ReadShaderFileText(directory + include_file.GetResult());
			if (!include_result)
			{
				return ut::MakeError(include_result.MoveAlt());
			}

			// replace current line with a code from the include file
			// start with the text that was before the include directive
			ut::String temp(out.GetAddress(), line_start - out.GetAddress());

			// add included file text
			temp += include_result.GetResult();
			temp += ut::cret;

			// remember an offset to the the next line
			const size_t next_line_offset = temp.Length();

			// add all text that was after the include directive
			temp += next_line;
			out = ut::Move(temp);

			// update the pointer to the next line
			next_line = out.GetAddress() + next_line_offset;
		}

		// moving to the next line
		cursor = next_line;
	}

	// success
	return out;
}

//----------------------------------------------------------------------------->
// Processes comments, reads a line and returns it's length in characters.
//    @param str - pointer to the text to read a line from.
//    @return - length of the line, or ut::Error if parsing error occurred.
ut::Result<size_t, ut::Error> ShaderCache::Reader::ReadLine(const char* str)
{
	// indicates line having '//' comment
	bool inline_comment = false;

	// indicates line having '/**/' comment
	bool comment_block = false;

	// remember pointer to the original line
	const char* const start = str;

	// iterate characters one by one
	while (true)
	{
		// exit if null-terminator occurs
		const char c = *str;
		if (c == '\0')
		{
			break;
		}

		// don't care processing comments if already inside '//' comment
		if (!inline_comment)
		{
			if (!comment_block && c == '/' && str[1] == '/')
			{
				inline_comment = true;
			}
			else if (!comment_block && c == '/' && str[1] == '*')
			{
				comment_block = true;
			}
			else if (comment_block && c == '*' && str[1] == '/')
			{
				comment_block = false;
			}
		}		

		// go to the next character
		str++;

		// line ends with '\n' in all cases except if inside a '/**/' comment block
		if (!comment_block && c == '\n')
		{
			break;
		}
	}

	// if comment block has no ending ('*/') - it's an error
	if (comment_block)
	{
		ut::log.Lock() << "Shader cache: comment block has no ending." << ut::cret;
		return ut::MakeError(ut::error::fail);
	}

	// return length of the line
	return str - start;
}

//----------------------------------------------------------------------------->
// Returns a number of whitespace characters in the beginning of the text.
//    @param str - pointer to the text containing whitespaces.
//    @param size - size of the text in characters.
//    @return - number of whitespace characters in front of the @str.
size_t ShaderCache::Reader::SkipWhitespaces(const char* str, size_t size)
{
	size_t i = 0;
	for (; i < size; i++)
	{
		const char c = str[i];
		if (c != '\t' && c != ' ')
		{
			break;
		}
	}
	return i;
}

//----------------------------------------------------------------------------->
// Reads a name of the file inside '""' or '<>' parentheses.
//    @param str - pointer to the string.
//    @param size - size of the @str in characters.
//    @return - filename string or ut::Error if parsing error occurred.
ut::Result<ut::String, ut::Error> ShaderCache::Reader::ReadIncludeFilename(const char* str,
                                                                           size_t size)
{
	// check size, statement must have one opening char, one closing char,
	// and at least one character for the filename
	if (size < 3)
	{
		ut::log.Lock() << "Shader cache: invalid include statement." << ut::cret;
		return ut::MakeError(ut::Error(ut::error::fail));
	}

	// read opening character
	char open = str[0];

	// detect a type of closing character
	char close;
	if (open == '\"')
	{
		close = '\"';
	}
	else if (open == '<')
	{
		close = '>';
	}
	else
	{
		ut::log.Lock() << "Shader cache: invalid opening character for include statement." << ut::cret;
		return ut::MakeError(ut::Error(ut::error::fail));
	}

	// find closing character
	for (size_t i = 1; i < size; i++)
	{
		if (str[i] == close)
		{
			// filename is a text between opening and closing characters
			const size_t filename_length = i - 1;
			ut::String filename(filename_length);
			ut::memory::Copy(filename.GetAddress(), str + 1, filename_length);

			// success
			return filename;
		}
	}

	// error, closing character is absent
	ut::log.Lock() << "Shader cache: closing character for include statement is absent." << ut::cret;
	return ut::MakeError(ut::Error(ut::error::fail));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//