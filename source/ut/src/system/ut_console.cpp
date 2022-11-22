//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_console.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
#define UT_INPUT_RECOVERY 20
#define UT_WND_INPUT_PING 100
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// class ut::ConsoleInputJob                                                  //
//----------------------------------------------------------------------------//
// Class ut::ConsoleInputJob processes ut::Console input events, this class
// is declared as a friend of ut::Console, so has access to private members
class ConsoleInputJob : public Job
{
public:
	// Constructor, owning console object must be specified
	//    @param owner - ut::Console object, that launches this job
	ConsoleInputJob(class Console* owner) : console(owner)
	{}

	// Processes input events of the console in separate thread
	void Execute()
	{
#if UT_WINDOWS
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		INPUT_RECORD input_records[50];
		DWORD num_elements = 0;
		DWORD num_chars = 0;
#endif

		// process input
		while (!exit_request.Read())
		{
#if UT_WINDOWS
			// read sequence of characters successfully
			// entered in the last UT_WND_INPUT_PING interval
			ReadConsoleInput(hStdin, input_records, 50, &num_elements);
			bool backspace_pressed = false;

			// process every character (key)
			for (uint32 i = 0; i < num_elements; i++)
			{
				if (input_records[i].EventType == KEY_EVENT)
				{
					KEY_EVENT_RECORD record = input_records[i].Event.KeyEvent;

					// skip alt
					DWORD flags = record.dwControlKeyState;
					if (flags& LEFT_ALT_PRESSED || flags& RIGHT_ALT_PRESSED)
					{
						continue;
					}

					// return current line to user, if key is 'Enter'
					if (record.bKeyDown && record.wVirtualKeyCode == VK_RETURN)
					{
						console->Sync();
					}

					// delete last element for backspace key
					if (record.bKeyDown && record.wVirtualKeyCode == VK_BACK && !backspace_pressed)
					{
						console->PopBackInput();
						backspace_pressed = true;
						continue;
					}

					// print character otherwise
					if (record.bKeyDown && console->IsValidSymbol(record.uChar.AsciiChar))
					{
						console->PutChar(record.uChar.AsciiChar);
					}
				}
			}

			// wait for UT_WND_INPUT_PING milliseconds for the next iteration
			this_thread::Sleep(UT_WND_INPUT_PING);
#elif UT_UNIX
			// get current character
			const char ch = getchar();

			// delete last element for backspace (127) key
			if (ch == 127)
			{
				console->PopBackInput();
			}
			else if (ch == '\n') // return current line to user, if key is 'Enter'
			{
				console->Sync();
			}
			else // print character otherwise
			{
				console->PutChar(ch);
			}
#endif
		}
	}
private:
	// Console, this job belongs to
	class Console* console;
};

//----------------------------------------------------------------------------//
// class ut::Console                                                          //
//----------------------------------------------------------------------------//
// Constructor, console is not initialized here, you must call
// ut::console.Open() before using object of this class
Console::Console() : allocated(false)
{
}

//----------------------------------------------------------------------------->
// Destructor, stops @input_thread, and closes console
Console::~Console()
{
	Close();
}

//----------------------------------------------------------------------------->
// Allocates console, call this function before using instance of the ut::Console
//    @return - 'true' if successfull
bool Console::Open()
{
	// lock @allocated variable
	ScopeSyncRWLock<bool> lock(allocated, access_write);

	// allocate console only once
	if (!lock.Get())
	{
		// allocate data
		data = MakeUnique<Data>(this);

		// set @allocated as 'true' after console was fully
		// allocated and input thread was created and launched
		lock.Set(true);
	}

	// success
	return true;
}

//----------------------------------------------------------------------------->
// Stops @input_thread, and closes console
void Console::Close()
{
	// lock @allocated variable
	ScopeSyncRWLock<bool> lock(allocated, access_write);

	// deallocate console
	if (lock.Get())
	{
		data.Delete();
		lock.Set(false);
	}
}

//----------------------------------------------------------------------------->
// Reads an array of @count elements, each one with a size of @size bytes,
// from the file and stores them in the block of memory specified by @ptr.
//    @param ptr - pointer to a block of memory with a size of
//                 at least (@size*@count) bytes
//    @param size - Size, in bytes, of each element to be read
//    @param count - Number of elements, each one with a size of @size bytes
//    @return - ut::Error if encountered an error
Optional<Error> Console::Read(void* ptr, size_t size, size_t count)
{
	// exit if console is not allocated
	ScopeSyncRWLock<bool> lock(allocated, access_read);
	if (!lock.Get())
	{
		return Optional<Error>();
	}

	// calculate the size requested for reading
	size_t requested_size = size * count;

	// wait until input buffer is big enough
	Array<char> temp_buffer;
	while (true)
	{
		this_thread::Sleep(UT_INPUT_RECOVERY);

		ScopeRWLock lock(data->input_lock, access_write);

		size_t buffer_size = data->input_buffer.GetSize();
		if (buffer_size >= requested_size)
		{
			// copy buffer content
			memory::Copy(ptr, data->input_buffer.GetAddress(), requested_size);

			// crop input buffer
			for (size_t i = requested_size; i-- > 0;)
			{
				data->input_buffer.Remove(i);
			}

			// exit
			break;
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes an array of @count elements, each one with a size of @size bytes,
// from the block of memory pointed by @ptr to the current position
//    @param ptr - pointer to the array of elements to be written
//    @param size - size in bytes of each element to be written
//    @param count - number of elements, each one with a size of @size bytes
//    @return - ut::Error if encountered an error
Optional<Error> Console::Write(const void* ptr, size_t size, size_t count)
{
	// exit if console is not allocated
	ScopeSyncRWLock<bool> lock(allocated, access_read);
	if (!lock.Get())
	{
		return Optional<Error>();
	}

	// wait if output is locked by another thread
	ThreadId thread_id = ut::this_thread::GetId();
	bool loop_out = false;
	while (!loop_out)
	{
		data->output_lock.Lock(access_read);
		if (data->output_locked)
		{
			if (data->locked_thread_id == thread_id)
			{
				loop_out = true;
			}
		}
		else
		{
			loop_out = true;
		}
		data->output_lock.Unlock(access_read);
		ut::this_thread::Sleep(1);
	}

	// start critical section
	Lock();

#if UT_NO_NATIVE_CONSOLE
	// convert data to text and append to the @obuf
	ut::Array<char> txt_buf(size * count + 1);
	memory::Copy(txt_buf.GetAddress(), ptr, size* count);
	txt_buf.GetLast() = '\0';
	data->pending_output += txt_buf.GetAddress();
#else

	// lock input_buffer for reading
	data->uncommitted_input_lock.Lock(access_read);

	// erase string, typed by user
	for (size_t i = 0; i < data->uncommitted_input.Count(); i++) std::cout << '\b';
	for (size_t i = 0; i < data->uncommitted_input.Count(); i++) std::cout << ' ';
	for (size_t i = 0; i < data->uncommitted_input.Count(); i++) std::cout << '\b';

	// write data
	std::cout.write((const char*)ptr, size * count);

	// print user input again
	for (size_t i = 0; i < data->uncommitted_input.Count(); i++)
	{
		const char c = data->uncommitted_input[i];
		std::cout << c;
	}

	// lock input_buffer for reading
	data->uncommitted_input_lock.Unlock(access_read);

	// flush std::cout stream
	std::cout.flush();
#endif // else UT_NO_NATIVE_CONSOLE

	// end critical section
	Unlock();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Locks output sequence, use it if you don't want another thread to
// break your output sequence (<< a << b << c) in your current thread
//    @param status - use 'true' to lock and 'false' to unlock output
void Console::LockOutputSequence(bool status)
{
	ScopeSyncRWLock<bool> lock(allocated, access_read);
	if (lock.Get())
	{
		ThreadId thread_id = ut::this_thread::GetId();
		ScopeRWLock scope_lock(data->output_lock, access_write);
		if (status && !data->output_locked)
		{
			data->output_locked = true;
			data->locked_thread_id = thread_id;
		}
		if (!status && data->output_locked && thread_id == data->locked_thread_id)
		{
			data->output_locked = false;
		}
	}
}

//----------------------------------------------------------------------------->
#if UT_NO_NATIVE_CONSOLE
// Returns pending text
//    @return - null terminated content of the @pending_output
String Console::FlushOutput()
{
	// exit if console is not allocated
	ScopeSyncRWLock<bool> lock(allocated, access_read);
	if (!lock.Get())
	{
		return String();
	}

	Lock();
	String out = data->pending_output;
	data->pending_output.Reset();
	Unlock();
	return out;
}
#endif // UT_NO_NATIVE_CONSOLE
//----------------------------------------------------------------------------->
// Sets user input buffer, implemented for platforms that don't
// have native console. Thus, ut::console::GetLine() is called
// from the main thread, and ut::console::Input() is called from
// custom console implementation. 
//    @param str - new input buffer
#if UT_NO_NATIVE_CONSOLE
void Console::SetInput(const String& str)
{
	ScopeSyncRWLock<bool> lock(allocated, access_read);
	if (allocated.Get())
	{
		const String copy = str + CarriageReturn<char>();
		const size_t len = copy.Length();
		data->input_lock.Lock(access_write);
		data->input_buffer.Reset();
		data->input_buffer.Resize(len);
		memory::Copy(data->input_buffer.GetAddress(), copy.GetAddress(), len);
		data->input_lock.Unlock(access_write);
	}
}
#endif

//----------------------------------------------------------------------------->
// Removes the last symbol of input string
void Console::PopBackInput(void)
{
	// start of the critical section
	Lock();

	// lock console input buffer for writing
	data->uncommitted_input_lock.Lock(access_write);

	// erase string typed by user from output
	// and remove the last symbol of the @ibuf
	if (data->uncommitted_input.Count() != 0)
	{
		std::cout << '\b';
		std::cout << ' ';
		std::cout << '\b';

		data->uncommitted_input.PopBack();
	}

	// unlock console input buffer for writing
	data->uncommitted_input_lock.Unlock(access_write);

	// end of the critical section
	Unlock();
}

//----------------------------------------------------------------------------->
// Moves uncommited (visible) input buffer to the final input buffer
Optional<Error> Console::Sync()
{
	ScopeSyncRWLock<bool> lock(allocated, access_read);
	if (allocated.Get())
	{
		// lock output
		Lock();

		// copy uncommited input
		data->uncommitted_input_lock.Lock(access_write);
		Array<char> temp_input = data->uncommitted_input;
		data->uncommitted_input.Reset();
		data->uncommitted_input_lock.Unlock(access_write);

		// erase input string from output
		for (size_t i = 0; i < temp_input.Count(); i++) std::cout << '\b';
		for (size_t i = 0; i < temp_input.Count(); i++) std::cout << ' ';
		for (size_t i = 0; i < temp_input.Count(); i++) std::cout << '\b';

		// copy previous uncommitted input text to the output buffer
		std::cout.write(temp_input.GetAddress(), temp_input.Count());
		std::cout << std::endl;

		// unlock output
		Unlock();

		// save final input
		data->input_lock.Lock(access_write);
		data->input_buffer = temp_input;
		String cret = CarriageReturn<char>();
		for (uint32 i = 0; i < cret.Length(); i++)
		{
			data->input_buffer.Add(cret[i]);
		}
		data->input_lock.Unlock(access_write);
	}

	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Adds symbol @c to the end of the @ibuf
void Console::PutChar(const char c)
{
	Lock();
	std::cout << c;
	Unlock();

	data->uncommitted_input_lock.Lock(access_write);
	data->uncommitted_input.Add(c);
	data->uncommitted_input_lock.Unlock(access_write);
}

//----------------------------------------------------------------------------->
// Checks if @c is a correct symbol
//    @param c - symbol to be checked
//    @return - 'true' if @c is a valid symbol
bool Console::IsValidSymbol(char c)
{
	switch(c)
	{
		case 'a': return true; case 'A': return true;
		case 'b': return true; case 'B': return true;
		case 'c': return true; case 'C': return true;
		case 'd': return true; case 'D': return true;
		case 'e': return true; case 'E': return true;
		case 'f': return true; case 'F': return true;
		case 'g': return true; case 'G': return true;
		case 'h': return true; case 'H': return true;
		case 'i': return true; case 'I': return true;
		case 'j': return true; case 'J': return true;
		case 'k': return true; case 'K': return true;
		case 'l': return true; case 'L': return true;
		case 'm': return true; case 'M': return true;
		case 'n': return true; case 'N': return true;
		case 'o': return true; case 'O': return true;
		case 'p': return true; case 'P': return true;
		case 'q': return true; case 'Q': return true;
		case 'r': return true; case 'R': return true;
		case 's': return true; case 'S': return true;
		case 't': return true; case 'T': return true;
		case 'u': return true; case 'U': return true;
		case 'v': return true; case 'V': return true;
		case 'w': return true; case 'W': return true;
		case 'x': return true; case 'X': return true;
		case 'y': return true; case 'Y': return true;
		case 'z': return true; case 'Z': return true;
		case ' ': return true;
		case '_': return true;
		case ',': return true; case '.': return true;
		case '0': return true;
		case '1': return true;
		case '2': return true;
		case '3': return true;
		case '4': return true;
		case '5': return true;
		case '6': return true;
		case '7': return true;
		case '8': return true;
		case '9': return true;
		case '-': return true; case '+': return true;
		case '=': return true;
		case '/': return true; case '\\': return true;
		case '*': return true;
		case '`': return true; case '"': return true;
		case '~': return true;
		case '|': return true;
		case '(': return true; case ')': return true;
		case '<': return true; case '>': return true;
		case '!': return true; case '?': return true;
		case '@': return true;
		case '#': return true;
		case ':': return true;
		case '%': return true;
		case '$': return true;
		case '&': return true;
	}
	return false;
}

//----------------------------------------------------------------------------->
// Locks output
void Console::Lock(void)
{
	data->output_atom_mutex.Lock();
}

//----------------------------------------------------------------------------->
// Unlocks output
void Console::Unlock(void)
{
	data->output_atom_mutex.Unlock();
}

//----------------------------------------------------------------------------//
// struct ut::Console::Data                                                   //
//----------------------------------------------------------------------------//
// ut::Console::Data is a console data container for the temporary
// use. The need of incapsulating members in the separate structure stems
// from the fact that ut::Console is allocatable by design. This means that
// we don't want all these members taking memory space if console is not used.
// Constructor allocates the console and launches the input thread.
Console::Data::Data(Console* owner) : output_locked(false)
{
#if UT_WINDOWS
	// open additional window for console
	::AllocConsole();

	// reopen console streams
	freopen_s(&console_in, "CONIN$", "r", stdin);
	freopen_s(&console_out, "CONOUT$", "w", stdout);

	// save console mode
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(hStdin, &mode);
#elif UT_UNIX && !UT_NO_NATIVE_CONSOLE
	// save current attributes
	memory::Set(&prev_t, 0, sizeof(prev_t));
	if (tcgetattr(0, &prev_t) < 0)
	{
		// err
	}

	// disable output
	struct termios new_t = prev_t;
	new_t.c_lflag &= ~ICANON;
	new_t.c_lflag &= ~ECHO;
	new_t.c_cc[VMIN] = 1;
	new_t.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &new_t) < 0)
	{
		// err
	}
#endif

	// Android doesn't have native console, thus std::cout is useless..
	// All characters will be accumulated into the @pending_output, and
	// Java code will be responsible for grabbing and displaying this buffer.
#if !UT_NO_NATIVE_CONSOLE
	// launch input thread
	UniquePtr<Job> input_job(MakeUnique<ConsoleInputJob>(owner));
	input_thread = MakeUnique<Thread>(Move(input_job));
#endif
}

//----------------------------------------------------------------------------//
// Destructor deallocates the console, reverts console to the previous state
// and stops the input thread.
Console::Data::~Data()
{
#if !UT_NO_NATIVE_CONSOLE
	// enable this scope to wait untill thread stops
	// instead of just killing it
	#if 0
		// pass job thread a command to stop
		input_thread->Exit();

		// wait till job finishes
		input_thread.Delete();
	#else
		// the only way  to finish input thread - violently kill it
		// othewise will be waiting getchar() to the end of days
		if (input_thread.Get())
		{
			input_thread->Kill();
		}
	#endif

	#if UT_UNIX
		tcsetattr(STDIN_FILENO, TCSANOW, &prev_t);
	#endif
#endif // !UT_NO_NATIVE_CONSOLE
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//