//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_cmdline.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//

Result<Array<String>, Error> GetCmdLine()
{
	Array<String> arguments;

#if UT_WINDOWS
	int arg_num;
	LPWSTR* arg_list = CommandLineToArgvW(GetCommandLineW(), &arg_num);
	if (nullptr == arg_list)
	{
		return MakeError(ConvertWinSysErr(GetLastError()));
	}

	for (int i = 0; i < arg_num; i++)
	{
		String argument = StrConvert<ut::wchar, char, ut::cp_utf8>(arg_list[i]);
		arguments.Add(Move(argument));
	}
#elif UT_UNIX
	const int buffer_size = 4096; // should really get PAGESIZE or something instead...
	Array<char> buffer(buffer_size);
	int fd = open("/proc/self/cmdline", O_RDONLY);
	int nbytesread = read(fd, buffer.GetAddress(), buffer_size);
	char* end = buffer.GetAddress() + nbytesread;
	for (char* p = buffer.GetAddress(); p < end; /**/)
	{
		arguments.Add(p);
		while (*p++); // skip until start of next 0-terminated section
	}
	close(fd);
#else
#error ut::GetCmdLine() is not implemented
#endif

	return arguments;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//