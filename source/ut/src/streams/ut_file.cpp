//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "streams/ut_file.h"
#include "integrity/ut_adler32.h"
//----------------------------------------------------------------------------//
#define UT_FILE_CURSOR_END SEEK_END
#define UT_FILE_CURSOR_SET SEEK_SET
#define UT_FILE_CURSOR_CUR SEEK_CUR
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
#if UT_WINDOWS
typedef __int64 FileCursor;
#elif UT_UNIX
typedef off_t FileCursor;
#endif
//----------------------------------------------------------------------------//

inline int ConvertFileCursor(stream::Position position)
{
	switch(position)
	{
		case stream::cursor: return UT_FILE_CURSOR_CUR;
		case stream::start: return UT_FILE_CURSOR_SET;
		case stream::end: return UT_FILE_CURSOR_END;
	}
	return UT_FILE_CURSOR_SET;
}

//----------------------------------------------------------------------------//
// Composes suitable for fopen() 'mode string' from UT parameters
//    @param access - file access mode (see FileAccess enumeration)
//    @return - fopen()-friendly 'mode string'
String FileAccessStr(FileAccess access)
{
	String out;

	switch(access)
	{
		case file_access_read:   out += "rb"; break;
		case file_access_write:  out += "wb"; break;
		case file_access_append: out += "ab"; break;
	}

	return out;
}

//----------------------------------------------------------------------------//
// Renames a file
//    @param old_fn - old filename
//    @param new_fn - new filename
//    @return - ut::Error if encountered an error
Optional<Error> RenameFile(const String& old_fn, const String& new_fn)
{
	// remove file with the same name (if exists)
	RemoveFile(new_fn);

	// rename file
#if UT_WINDOWS
	WString wold = StrConvert<char, wchar, cp_utf8>(old_fn);
	WString wnew = StrConvert<char, wchar, cp_utf8>(new_fn);
	int result = _wrename(wold.ToCStr(), wnew.ToCStr());
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}
#elif UT_UNIX
	int result = rename(old_fn.GetAddress(), new_fn.GetAddress());
	if (result != 0)
	{
		return Error(ConvertErrno(errno));
	}
#else
#error ut::RenameFile() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Removes a file
//    @param filename - file to remove
//    @return - ut::Error if encountered an error
Optional<Error> RemoveFile(const String& filename)
{
#if UT_WINDOWS
	WString wfn = StrConvert<char, wchar, cp_utf8>(filename);
	int result = _wremove(wfn.ToCStr());
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}
#elif UT_UNIX
	int result = remove(filename.GetAddress());
	if (result != 0)
	{
		return Error(ConvertErrno(errno));
	}
#else
#error ut::RemoveFile() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Removes a folder
//    @param folder - folder to remove
//    @param delete_subdirectories - if 'true', folder's content
//                                   will be deleted too
//    @return - ut::Error if encountered an error
Optional<Error> RemoveFolder(const String& folder, bool delete_subdirectories)
{
#if UT_WINDOWS
	HANDLE          directory_handle;            // Handle to directory
	WString         file_path;                   // Filepath
	WString         pattern_str;                 // Pattern
	WIN32_FIND_DATA file_info;                   // File information
	bool            has_subdirectory = false;    // Flag, indicating whether
												 // subdirectories have been found

												 // delete all subdirectories
	WString wfolder = StrConvert<char, wchar, cp_utf8>(folder);
	pattern_str = wfolder;
	pattern_str += L"\\*.*";
	directory_handle = ::FindFirstFile(pattern_str.ToCStr(), &file_info);
	if (directory_handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (file_info.cFileName[0] != '.')
			{
				file_path.Empty();
				file_path = wfolder;
				file_path += L"\\";
				file_path += file_info.cFileName;

				if (file_info.dwFileAttributes& FILE_ATTRIBUTE_DIRECTORY)
				{
					if (delete_subdirectories)
					{
						// Delete subdirectory
						String wsubdir = StrConvert<wchar, char, cp_utf8>(file_path);
						Optional<Error> error = RemoveFolder(wsubdir, delete_subdirectories);
						if (error)
						{
							return error;
						}
					}
					else
					{
						has_subdirectory = true;
					}
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributes(file_path.GetAddress(), FILE_ATTRIBUTE_NORMAL) == FALSE)
					{
						return Error(ConvertWinSysErr(GetLastError()));
					}

					// Delete file
					if (::DeleteFile(file_path.GetAddress()) == FALSE)
					{
						return Error(ConvertWinSysErr(GetLastError()));
					}
				}
			}
		} while (::FindNextFile(directory_handle, &file_info));

		// close handle
		::FindClose(directory_handle);

		// delete directory
		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
		{
			return error::fail;
		}
		else
		{
			if (!has_subdirectory)
			{
				// Set directory attributes
				if (::SetFileAttributes(wfolder.GetAddress(), FILE_ATTRIBUTE_NORMAL) == FALSE)
				{
					return Error(ConvertWinSysErr(GetLastError()));
				}

				// Delete directory
				if (::RemoveDirectory(wfolder.GetAddress()) == FALSE)
				{
					return Error(ConvertWinSysErr(GetLastError()));
				}
			}
		}
	}

#elif UT_UNIX
	DIR* d = opendir(folder.GetAddress());
	size_t path_len = StrLen<char>(folder.GetAddress());
	int r = -1;

	if (d)
	{
		struct dirent* p;

		r = 0;

		while (!r && (p = readdir(d)))
		{
			int r2 = ENOMEM;
			char* buf;
			size_t len;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (StrCmp<char>(p->d_name, ".") || StrCmp<char>(p->d_name, ".."))
			{
				continue;
			}

			len = path_len + StrLen<char>(p->d_name) + 2;
			buf = (char*)malloc(len);

			if (buf)
			{
				struct stat statbuf;

				snprintf(buf, len, "%s/%s", folder.GetAddress(), p->d_name);

				if (!stat(buf, &statbuf))
				{
					if (S_ISDIR(statbuf.st_mode))
					{
						Optional<Error> error = RemoveFolder(buf);
						if (error)
						{
							r2 = EPERM;
						}
						else
						{
							r2 = 0;
						}
					}
					else
					{
						r2 = unlink(buf);
					}
				}

				free(buf);
			}

			r = r2;
		}

		closedir(d);
	}
	else
	{
		// success if no directory
		return Optional<Error>();
	}

	if (!r)
	{
		r = rmdir(folder.GetAddress());
	}

	return r == 0 ? Optional<Error>() : Error(ConvertErrno(r));
#else
#error ut::RemoveFolder() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Copies a file
//    @param source - path to the source file
//    @param source - destination path
//    @param replace - replaces @dest file if 'true' (if it exists)
//                     or does nothing if 'false' (and @dest file exists)
//    @return - ut::Error if encountered an error
Optional<Error> CopyFile(const String& source, const String& dest, bool replace)
{
#if UT_WINDOWS
	// remove existing file
	if (replace)
	{
		RemoveFile(dest);
	}

	// convert to wide char
	WString wsrc = StrConvert<char, wchar, cp_utf8>(source);
	WString wdst = StrConvert<char, wchar, cp_utf8>(dest);

	// copy file
	BOOL result = ::CopyFileW(wsrc.GetAddress(), wdst.GetAddress(), (BOOL)replace);
	return result ? Optional<Error>() : Error(ConvertWinSysErr(GetLastError()));
#elif UT_UNIX
	int isource = open(source.GetAddress(), O_RDONLY, 0);
	if (isource == -1)
	{
		return Error(ConvertErrno(errno));
	}

	int idest = open(dest.GetAddress(), O_WRONLY | O_CREAT /*| O_TRUNC*/, 0644);
	if (idest == -1)
	{
		return Error(ConvertErrno(errno));
	}

	// struct required, rationale: function stat() exists also
	struct stat stat_source;
	fstat(isource, &stat_source);

	ssize_t bytes = sendfile(idest, isource, 0, stat_source.st_size);

	close(isource);
	close(idest);

	if (bytes < 0)
	{
		close(isource);
		close(idest);
		return Error(ConvertErrno(errno));
	}

	// set permissions
	if (chmod(dest.GetAddress(), S_IRWXU | S_IRWXG | S_IRWXG) != 0)
	{
		// not really an error
		// return Error(ConvertErrno(errno));
	}
#else
#error ut::CopyFile() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Creates a new directory
//    @param folder - the path of the directory to be created
//    @return - 'true' if successfull
Optional<Error> CreateFolder(const String& folder)
{
#if UT_WINDOWS
	WString wfolder = StrConvert<char, wchar, cp_utf8>(folder);
	if (CreateDirectory(wfolder.GetAddress(), NULL) == FALSE)
	{
		return Error(ConvertWinSysErr(GetLastError()));
	}
#elif UT_UNIX
	struct stat st = { 0 };

	if (stat(folder.GetAddress(), &st) == -1)
	{
		if (mkdir(folder.GetAddress(), 0700) != 0)
		{
			return Error(ConvertErrno(errno));
		}

		if (chmod(folder.GetAddress(), S_IRWXU | S_IRWXG | S_IRWXG) != 0)
		{
			// not really an error
			// return Error(ConvertErrno(errno));
		}
	}
#else
#error ut::CreateFolder() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Creates directory tree.
//    @param folder - the path of the directory tree to be created
//    @return - 'true' if successfull
Optional<Error> CreateDirectories(const String& path)
{
	const char* str = path.GetAddress();
	const char* start = str;
	while (*str != '\0')
	{
		if (*str == '\\' || *str == '/')
		{
			ut::String directory(start, str - start);
			Optional<Error> dir_error = CreateFolder(directory);
			if (dir_error)
			{
				return dir_error;
			}
		}
		str++;
	}
	return CreateFolder(path);
}

//----------------------------------------------------------------------------//
// Calculates Adler32 checksum for the file
//    @param filename - path to the file
//    @param out_checksum - checksum will be calculated to this variable
//    @return - 'true' if successfull(file exists)
Result<uint32, Error> FileCheckSumAdler32(const String& filename)
{
	// open file
	File file;
	Optional<Error> open_error = file.Open(filename, file_access_read);
	if (open_error)
	{
		return MakeError(open_error.Get());
	}

	// get file size
	Result<size_t, Error> get_size_result = file.GetSize();
	if (!get_size_result)
	{
		return MakeError(get_size_result.GetAlt());
	}
	size_t file_size = get_size_result.Get();

	// iterate all bytes
	uint32 a = 1, b = 0;
	size_t index;
	for (index = 0; index < file_size; ++index)
	{
		uint8 value;
		file.Read(&value, 1, 1);
		a = (a + value) % UT_ADLER_32_MOD;
		b = (b + a) % UT_ADLER_32_MOD;
	}

	// close item
	file.Close();

	// return checksum
	return (b << 16) | a;
}

//----------------------------------------------------------------------------//
// Reads a file into a byte array.
//    @param filename - path to the file.
//    @return - array of bytes or ut::Error if failed.
Result<Array<ut::byte>, Error> ReadFile(const String& filename)
{
	// open a file
	File file;
	Optional<Error> open_file_error = file.Open(filename, file_access_read);
	if (open_file_error)
	{
		return MakeError(open_file_error.Move());
	}

	// get file size
	Result<size_t, Error> get_size_result = file.GetSize();
	if (!get_size_result)
	{
		return MakeError(get_size_result.MoveAlt());
	}
	size_t file_size = get_size_result.Get();

	// read file contents
	Array<ut::byte> data(file_size);
	Optional<Error> read_error = file.Read(data.GetAddress(), 1, file_size);
	if (read_error)
	{
		return MakeError(read_error.Move());
	}

	// close the file
	Optional<Error> close_error = file.Close();
	if (close_error)
	{
		return MakeError(close_error.Move());
	}

	// success
	return data;
}

//----------------------------------------------------------------------------//
// Writes provided data to file
//    @param filename - path to the file
//    @param data - pointer to the data to be written
//    @param size - size of @data in bytes
//    @return - a string or ut::Error if failed
Optional<Error> WriteFile(const String& filename,
                          const void* data,
                          size_t size)
{
	// open a file
	File file;
	Optional<Error> opt_error = file.Open(filename, file_access_write);
	if (opt_error)
	{
		return opt_error;
	}

	// write data
	opt_error = file.Write(data, 1, size);
	if (opt_error)
	{
		return opt_error;
	}

	// close
	opt_error = file.Close();
	if (opt_error)
	{
		return opt_error;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// class ut::File                                                             //
//----------------------------------------------------------------------------//
// Default constructor
File::File() : f(nullptr)
{}

//----------------------------------------------------------------------------->
// Constructor, opens file @filename
//    @param filename - path to the file
//    @param access - file access mode (see FileAccess enumeration)
File::File(const String& filename, FileAccess access) : f(nullptr)
{
	Optional<Error> open_error = Open(filename, access);
	if (open_error)
	{
		throw open_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Move constructor.
File::File(File&& other) noexcept : f(other.f)
{
	other.f = nullptr;
}

//----------------------------------------------------------------------------->
// Move operator.
File& File::operator = (File&& other) noexcept
{
	Close();
	f = other.f;
	other.f = nullptr;
	return *this;
}

//----------------------------------------------------------------------------->
// Destructor, closes the file (if it was previously opened)
File::~File()
{
	Close();
}

//----------------------------------------------------------------------------->
// Opens file @filename
//    @param filename - path to the file
//    @param access - file access mode (see FileAccess enumeration)
Optional<Error> File::Open(const String& filename, FileAccess access)
{
	String mode_str = FileAccessStr(access);

#if UT_WINDOWS
	WString wfilename = StrConvert<char, wchar, cp_utf8>(filename);
	WString wmode = StrConvert<char, wchar, cp_ascii>(mode_str);
	int result = _wfopen_s(&f, wfilename.GetAddress(), wmode.GetAddress());
	if (result != 0)
	{
		f = nullptr;
		return Error(ConvertErrno(result));
	}
#elif UT_UNIX
    // open file
    f = fopen(filename.GetAddress(), mode_str.GetAddress());
	if (!f)
	{
		return Error(ConvertErrno(errno));
	}
#else
	#error ut::File::Open() is not implemented
#endif
	path = filename;
    return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Closes the file (if it was previously opened), you have no real need
// to close file manually, it will be closed in destructor anyway. However
// you can close the file earlier if you want to use it in another way
// (copy, remove, open in another mode, etc.)
Optional<Error> File::Close()
{
	if (f != nullptr)
	{
		if (fclose(f) != 0)
		{
			return Error(error::fail);
		}
		f = nullptr;
#if UT_UNIX
		chmod(path.GetAddress(), S_IRWXU | S_IRWXG | S_IRWXG);
#elif !UT_WINDOWS
		#error ut::File::Close() is not implemented
#endif
		path.Empty();
	}
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns the path to opened file (or empty string if no file was opened)
String File::GetPath() const
{
	return path;
}

//----------------------------------------------------------------------------->
// Returns 'true' if a file was opened, and hasn't been closed yet
bool File::IsOpened() const
{
	return f != nullptr ? true : false;
}

//----------------------------------------------------------------------------->
// Returns file offset to the current cursor position (in bytes)
//    @return - cursor position if file is opened, or error otherwise
Result<stream::Cursor, Error> File::GetCursor() const
{
	// return error if no file
	if (f == nullptr)
	{
		return MakeError(error::empty);
	}

	// get cursor position
	FileCursor cursor;
#if UT_WINDOWS
	cursor = _ftelli64(f);
#elif UT_UNIX
	cursor = ftello(f);
#else
	#error ut::File::GetCursor() is not implemented
#endif

	// out
	return (stream::Cursor)cursor;
}

//----------------------------------------------------------------------------->
// Sets file offset to the current cursor position (in bytes)
//    @param offset - offset in bytes from @origin
//    @param origin - offset from the beginning of the file
//                    @offset will be added to this parameter
//    @return - error code if failed
Optional<Error> File::MoveCursor(stream::Cursor offset, stream::Position origin)
{
	if (f == nullptr)
	{
		return Error(error::empty);
	}

#if UT_WINDOWS
	if (_fseeki64(f, offset, ConvertFileCursor(origin)) != 0)
	{
		return Error(error::fail);
	}
#elif UT_UNIX
	if (fseeko(f, offset, ConvertFileCursor(origin)) != 0)
	{
		return Error(error::fail);
	}
#else
	#error ut::File::MoveCursor() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes an array of @count elements, each one with a size of @size bytes,
// from the block of memory pointed by @ptr to the current position
//    @param ptr - pointer to the array of elements to be written
//    @param size - size in bytes of each element to be written
//    @param count - number of elements, each one with a size of @size bytes
//    @return - ut::Error if encountered an error
Optional<Error> File::Write(const void* ptr, size_t size, size_t count)
{
	if (size != 0 && count != 0)
	{
		if (f != nullptr)
		{
			size_t result = fwrite(ptr, size, count, f);
			if (result != count)
			{
				return Error(error::fail);
			}
		}
		else
		{
			return Error(error::invalid_arg);
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads an array of @count elements, each one with a size of @size bytes,
// from the file and stores them in the block of memory specified by @ptr.
//    @param ptr - pointer to a block of memory with a size of
//                 at least (@size*@count) bytes
//    @param size - Size, in bytes, of each element to be read
//    @param count - Number of elements, each one with a size of @size bytes
//    @return - ut::Error if encountered an error
Optional<Error> File::Read(void* ptr, size_t size, size_t count)
{
	if (size != 0 && count != 0)
	{
		if (f != nullptr)
		{
			size_t result = fread(ptr, size, count, f);
			if (result != count)
			{
				return Error(error::fail);
			}
		}
		else
		{
			return Error(error::invalid_arg);
		}
	}

	// success
	return Optional<Error>();
}

// Synchronizes the associated stream buffer with file.
//    @return - error code if failed
Optional<Error> File::Flush()
{
	if (f == nullptr)
	{
		return ut::Error(ut::error::empty);
	}

	int flush_error = fflush(f);
	if (flush_error != 0)
	{
		return ut::Error(ConvertErrno(flush_error));
	}

	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns size of the stream buffer or error if failed
Result<size_t, Error> File::GetSize()
{
	// return error if no file
	if (f == nullptr)
	{
		return MakeError(error::empty);
	}

	// intermediate variables
	stream::Cursor cursor_origin;
	stream::Cursor file_size;

	// get initial cursot position
	cursor_origin = GetCursor().Get();

	// handle error
	if (cursor_origin == -1)
	{
		return MakeError(error::fail);
	}

	// move cursor to the end of file
	Optional<Error> move_forward_error = MoveCursor(0, stream::end);
	if (move_forward_error)
	{
		return MakeError(move_forward_error.Get());
	}

	// get file size
	Result<stream::Cursor, Error> get_cursor_result = GetCursor();
	if (get_cursor_result)
	{
		file_size = get_cursor_result.Get();
	}
	else
	{
		return MakeError(get_cursor_result.GetAlt());
	}

	// handle error
	if (file_size == -1)
	{
		return MakeError(error::fail);
	}

	// move cursor back
	Optional<Error> move_back_error = MoveCursor(cursor_origin, stream::start);
	if (move_back_error)
	{
		return MakeError(move_back_error.Get());
	}

	// out
	return (size_t)file_size;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
