//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "streams/ut_input_stream.h"
#include "streams/ut_output_stream.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::FileAccess describes access to the data of the opened file
//    @fa_read - file's data is preserved, and can only be read,
//               writing is forbidden
//    @fa_write - file's data is erased, writing is allowed
//    @fa_append - file's data is preserved, writing is allowed
enum FileAccess
{
	file_access_read   = 0,
	file_access_write  = 1,
	file_access_append = 2
};

//----------------------------------------------------------------------------//
// Renames a file
//    @param old_fn - old filename
//    @param new_fn - new filename
//    @return - ut::Error if encountered an error
Optional<Error> RenameFile(const String& old_fn, const String& new_fn);

//----------------------------------------------------------------------------//
// Removes a file
//    @param filename - file to remove
//    @return - ut::Error if encountered an error
Optional<Error> RemoveFile(const String& filename);

//----------------------------------------------------------------------------//
// Removes a folder
//    @param folder - folder to remove
//    @param delete_subdirectories - if 'true', folder's content
//                                   will be deleted too
//    @return - ut::Error if encountered an error
Optional<Error> RemoveFolder(const String& folder, bool delete_subdirectories = true);

//----------------------------------------------------------------------------//
// Copies a file
//    @param source - path to the source file
//    @param source - destination path
//    @param replace - replaces @dest file if 'true' (if it exists)
//                     or does nothing if 'false' (and @dest file exists)
//    @return - ut::Error if encountered an error
Optional<Error> CopyFile(const String& source, const String& dest, bool replace = true);

//----------------------------------------------------------------------------//
// Creates a new directory
//    @param folder - the path of the directory to be created
//    @return - 'true' if successfull
Optional<Error> CreateFolder(const String& folder);

//----------------------------------------------------------------------------//
// Creates directory tree.
//    @param folder - the path of the directory tree to be created
//    @return - 'true' if successfull
Optional<Error> CreateDirectories(const String& path);

//----------------------------------------------------------------------------//
// Calculates Adler32 checksum for the file
//    @param filename - path to the file
//    @param out_checksum - checksum will be calculated to this variable
//    @return - checksum if successfull(file exists), or error code otherwise
Result<uint32, Error> FileCheckSumAdler32(const String& filename);

//----------------------------------------------------------------------------//
// Reads a file into a byte array
//    @param filename - path to the file
//    @return - array of bytes or ut::Error if failed
Result<Array<ut::byte>, Error> ReadFile(const String& filename);

//----------------------------------------------------------------------------//
// Writes provided data to file
//    @param filename - path to the file
//    @param data - pointer to the data to be written
//    @param size - size of @data in bytes
//    @return - a string or ut::Error if failed
Optional<Error> WriteFile(const String& filename,
                          const void* data,
                          size_t size);

//----------------------------------------------------------------------------//
// Writes provided array to file
//    @param filename - path to the file
//    @param data - const reference to the array to be written
template<typename ElementType>
Optional<Error> WriteFile(const String& filename,
                          const Array<ElementType>& data)
{
	return WriteFile(filename, data.GetAddress(), data.GetNum() * sizeof(ElementType));
}


//----------------------------------------------------------------------------//
// ut::File is a wrapper around LibC file stream, data can be read / written
// after the file is opened (using constructor or Open() function), file is
// closed in destructor or Close() function. Use Close() function to release
// the file (if you want to use it in another way) before the end of scope.
class File : public InputStream, public OutputStream
{
public:
	// Default constructor
    File();

	// Constructor, opens file @filename
	//    @param filename - path to the file
	//    @param access - file access mode (see FileAccess enumeration)
	File(const String& filename, FileAccess access);

	// Move constructor.
	File(File&& other) noexcept;

	// Move operator.
	File& operator = (File&& other) noexcept;

	// Copying is prohibited.
	File(const File&) = delete;
	File& operator = (const File&) = delete;

	// Destructor, closes the file (if it was previously opened)
	~File();

	// Opens file @filename
	//    @param filename - path to the file
	//    @param access - file access mode (see FileAccess enumeration)
	Optional<Error> Open(const String& filename, FileAccess access);

	// Closes the file (if it was previously opened), you have no real need
	// to close file manually, it will be closed in destructor anyway. However
	// you can close the file earlier if you want to use it in another way
	// (copy, remove, open in another mode, etc.)
	Optional<Error> Close();

	// Returns the path to opened file (or empty string if no file was opened)
	String GetPath() const;

	// Returns 'true' if a file was opened, and hasn't been closed yet
	bool IsOpened() const;

	// Returns file offset to the current cursor position (in bytes)
	//    @return - cursor position if file is opened, or error otherwise
	Result<stream::Cursor, Error> GetCursor() const;

	// Sets file offset to the current cursor position (in bytes)
	//    @param offset - offset in bytes from @origin
	//    @param origin - offset from the beginning of the file
	//                    @offset will be added to this parameter
	//    @return - error code if failed
	Optional<Error> MoveCursor(stream::Cursor offset, stream::Position origin);

	// Writes an array of @count elements, each one with a size of @size bytes,
	// from the block of memory pointed by @ptr to the current position
	//    @param ptr - pointer to the array of elements to be written
	//    @param size - size in bytes of each element to be written
	//    @param count - number of elements, each one with a size of @size bytes
	//    @return - ut::Error if encountered an error
	Optional<Error> Write(const void* ptr, size_t size, size_t count);

	// Reads an array of @count elements, each one with a size of @size bytes,
	// from the file and stores them in the block of memory specified by @ptr.
	//    @param ptr - pointer to a block of memory with a size of
	//                 at least (@size*@count) bytes
	//    @param size - Size, in bytes, of each element to be read
	//    @param count - Number of elements, each one with a size of @size bytes
	//    @return - ut::Error if encountered an error
	Optional<Error> Read(void* ptr, size_t size, size_t count);

	// Synchronizes the associated stream buffer with file.
	//    @return - error code if failed
	Optional<Error> Flush();

	// Returns size of the stream buffer or error if failed
	Result<size_t, Error> GetSize();

private:
	String path; // path to the file
    FILE* f; // stream pointer
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
