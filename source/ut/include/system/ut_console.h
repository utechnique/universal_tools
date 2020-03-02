//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "containers/ut_singleton.h"
#include "system/ut_thread.h"
#include "system/ut_sync.h"
#include "streams/ut_input_stream.h"
#include "streams/ut_output_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Console is a class for interacting with console, you can display
// information using operator '<<' so if it was std::cout. If you have to wait
// for user input - use ut::Console::GetLine() function. ut::Console is thread
// safe. Object ut::console is created as a singleton instance, you should not
// create any other instances of the ut::Console, but don't forget to call
// ut::console.Open() function before using it.
class Console : public InputStream, public OutputStream, public NonCopyable
{
	// ut::ConsoleInputJob is declared as a friend to have acces to private members
	friend class ConsoleInputJob;
	// Make the constructor available for the singleton container
	friend class Singleton<Console>;
private:
	// Constructor, console is not initialized here, you must call
	// ut::console.Open() before using object of this class
	Console();

public:
	// Destructor, calls Close() internally
	~Console();

	// Allocates console, call this function before using instance of the ut::Console
	//    @return - 'true' if successfull
	bool Open();

	// Stops @input_thread, and closes console.
	// Call it when finished using the console.
	void Close();

	// Reads an array of @count elements, each one with a size of @size bytes,
	// from the file and stores them in the block of memory specified by @ptr.
	//    @param ptr - pointer to a block of memory with a size of
	//                 at least (@size*@count) bytes
	//    @param size - Size, in bytes, of each element to be read
	//    @param count - Number of elements, each one with a size of @size bytes
	//    @return - ut::Error if error occurred
	Optional<Error> Read(void* ptr, size_t size, size_t count);

	// Writes an array of @count elements, each one with a size of @size bytes,
	// from the block of memory pointed by @ptr to the current position
	//    @param ptr - pointer to the array of elements to be written
	//    @param size - size in bytes of each element to be written
	//    @param count - number of elements, each one with a size of @size bytes
	//    @return - ut::Error if error occurred
	Optional<Error> Write(const void* ptr, size_t size, size_t count);

	// Locks output sequence, use it if you don't want another thread to
	// break your output sequence (<< a << b << c) in your current thread
	//    @param status - use 'true' to lock and 'false' to unlock output
	void LockOutputSequence(bool status);
	
#if UT_NO_NATIVE_CONSOLE
	// Returns pending text
	//    @return - null terminated content of the @pending_output
	String FlushOutput();

	// Sets user input buffer, implemented for platforms that don't
	// have native console. Thus, ut::console::GetLine() is called
	// from the main thread, and ut::console::Input() is called from
	// the custom console implementation. 
	//    @param str - new input buffer
	void SetInput(const String& str);
#endif // UT_NO_NATIVE_CONSOLE

private:


	// Removes the last symbol of input string
	void PopBackInput();

	// Moves uncommited (visible) input buffer to the final input buffer
	Optional<Error> Sync();

	// Adds symbol @c to the end of the @ibuf
	void PutChar(const char c);

	// Checks if @c is a correct symbol
	//    @param c - symbol to be checked
	//    @return - 'true' if @c is a valid symbol
	bool IsValidSymbol(char c);

	// Locks output
	void Lock();

	// Unlocks output
	void Unlock();
	
private:
	// ut::Console::Data is a console data container for the temporary
	// use. The need of incapsulating members in the separate structure stems
	// from the fact that ut::Console is allocatable by design. This means that
	// we don't want all these members taking memory space if console is not used.
	struct Data
	{
		// Constructor allocates the console and launches the input thread.
		Data(Console* owner);

		// Destructor deallocates the console, reverts console to the previous state
		// and stops the input thread.
		~Data();

		// Input thread
		UniquePtr<Thread> input_thread;

		RWLock uncommitted_input_lock;
		Array<char> uncommitted_input;

		RWLock input_lock;
		Array<char> input_buffer;

		// Mutex for locking single '<<' statement
		Mutex output_atom_mutex;

		// Output lock (used for multiple '<<' operator calls)
		RWLock output_lock;

		// Indicates if output is currently locked by user for a single thread
		bool output_locked;

		// Holds the Id of the locked thread if @output_locked is 'true'
		ThreadId locked_thread_id;

		// platform-specific console objects
		#if UT_WINDOWS
			// Windows has separate streams for input and output,
			// both are allocatable via freopen_s
			FILE* console_in;
			FILE* console_out;
		#elif UT_UNIX
			#if UT_ANDROID
				// Holds accumulated output. Android java code calls
				// ut::Console::FlushOutput() to grab and display this
				// sequence of characters.
				String pending_output;
			#else
				// UT console has very specific properties
				// (disabled default output, disabled canon mode, etc.).
				// Thus @prev_t variable holds state of the console before
				// ut::Console::Open() was called to be able to revert
				// these attributes after ut::Console::Close().
				struct termios prev_t;
			#endif
		#endif
	};

	// Pointer to the allocatable data.
	// ut::Console::Open() creates data.
	// ut::Console::Close() destructs data.
	UniquePtr<Data> data;

	// Indicates if @data member is already allocated
	SyncRW<bool> allocated;
};

//----------------------------------------------------------------------------//
// Use this singleton for interacting with the console.
// Reason to provide console as a singleton:
// UT console changes attributes of the native os-console, thus it's too
// dangerous to have multiple instances of ut::Console.
static Console& console = Singleton<Console>::Instance();

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//