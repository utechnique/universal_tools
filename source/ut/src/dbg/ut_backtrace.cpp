//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "dbg/ut_backtrace.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
#define UT_TRACE_FUNC_MAX_NAME_LENGTH 1024
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Returns current call stack
//    @return - array of addresses of the functions in the callstack
Array<void*> Backtrace()
{
	void* stack[UT_CALL_STACK_DEPTH];
#if UT_WINDOWS
	uint16 frames_num = CaptureStackBackTrace(0, UT_CALL_STACK_DEPTH, stack, nullptr);
#elif UT_UNIX
	int frames_num = backtrace(stack, UT_CALL_STACK_DEPTH);
#else
	#error ut::Backtrace() is not implemented
#endif
	Array<void*> out(frames_num);
	memory::Copy(out.GetAddress(), stack, frames_num * sizeof(void*));
	return out;
}

//----------------------------------------------------------------------------//
// Returns current call stack with symbols
//    @return - array of functions' names in the callstack
Array<String> SymbolsBacktrace()
{
#if DEBUG
	// function names
	Array<String> out;

	// get callstack addresses
	Array<void*> addresses = Backtrace();
	#if UT_WINDOWS
		// get process handle and initialize debug symbols
		HANDLE process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);

		// allocate memory
		SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + (UT_TRACE_FUNC_MAX_NAME_LENGTH - 1) * sizeof(wchar));
		IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*)malloc(sizeof(IMAGEHLP_LINE64));

		// set name length and size if the first element of SYMBOL_INFO array
		symbol->MaxNameLen = UT_TRACE_FUNC_MAX_NAME_LENGTH;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		// set size of the struct in the first element of IMAGEHLP_LINE64 array
		line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		// collect all stack frames
		for (size_t i = 1; i < addresses.Count(); i++)
		{
			DWORD64 address = (DWORD64)(addresses[i]);
			DWORD line_displacement;
			SymFromAddr(process, address, NULL, symbol);
			if (SymGetLineFromAddr64(process, address, &line_displacement, line))
			{
				String filename(line->FileName);
				filename.IsolateFilename();

				String line_str;
				line_str.Print("%lu(%u)", line->LineNumber, line_displacement);

				String addr_str;
				addr_str.Print("0x%0X", symbol->Address);

				String frame(symbol->Name);
				frame += String("(") + addr_str + String(")");
				frame += String(" ") + filename;
				frame += String(" line: ") + line_str;

				out.Add(frame);
			}
			else
			{
				String addr_str;
				addr_str.Print("unknown_function(0x%0X)", addresses[i]);

				out.Add(addr_str);
			}
		}

		// release resources
		free(symbol);
		free(line);
		SymCleanup(process);
	#elif UT_UNIX
		char** symbols = backtrace_symbols(addresses.GetAddress(), addresses.Count());

		for (size_t i = 0; i < addresses.Count(); i++)
		{
			String addr_str;
			addr_str.Print("0x%0X", addresses[i]);

			Dl_info info;
			if (dladdr(addresses[i], &info))
			{
				char* demangled = nullptr;
				int status;
				demangled = abi::__cxa_demangle(info.dli_sname, nullptr, 0, &status);
				String name(status == 0 ? demangled : info.dli_sname);
				out.Add(name + String("(") + addr_str + String(")"));
				free(demangled);
			}
			else
			{
				out.Add(String("unknown_function(") + addr_str + String(")"));
			}
		}

		free(symbols);
	#else
		#error ut::SymbolsBacktrace() is not implemented
	#endif // PLATFORM

	// return array of strings
	// each string is a stack frame
	return out;
#else // DEBUG
	Array<void*> addresses = Backtrace();
	Array<String> out;
	for (size_t i = 1; i < addresses.Count(); i++)
	{
		String addr_str;
		addr_str.Print("unknown_function(0x%0X)", addresses[i]);
		out.Add(addr_str);
	}
	return out;
#endif // DEBUG
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//