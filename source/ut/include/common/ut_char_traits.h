//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if CPP_STANDARD < 2011 && UT_WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#endif
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "preprocessor/ut_array_arguments.h"
//----------------------------------------------------------------------------//
#define UT_STR_MAX 256
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Character types
typedef wchar_t wchar;
typedef unsigned short ucs2char;
typedef unsigned short utf16char;
typedef unsigned long  utf32char;

//----------------------------------------------------------------------------//
// Returns 'true' if @str0 and @str1 are equal
//    @param str0 - first null-terminated string
//    @param str1 - second null-terminated string
template <typename T>
bool StrCmp(const T* str0, const T* str1)
{
	const T* s1 = (const T*)str0;
	const T* s2 = (const T*)str1;
	T c1, c2;
	do
	{
		c1 = (T)*s1++;
		c2 = (T)*s2++;
		if (c1 == '\0')
		{
			return c1 == c2;
		}
	}
	while (c1 == c2);

	return c1 == c2;
}

//----------------------------------------------------------------------------//
// Returns length of the string, in characters, without null terminator
//    @param str - null-terminated string
template <typename T>
size_t StrLen(const T* str)
{
	size_t i;
	for (i = 0; str[i] != '\0'; i++);
	return i;
}

//----------------------------------------------------------------------------//
// Returns a pointer to first occurrence of character @ch
// in @src, or nullptr (if no occurrence was found)
//    @param src - null-terminated source (donor) string
//    @param ch - symbol
template <typename T>
T* StrChr(T* src, int ch)
{
	while (*src != (T)ch)
	{
		if (!*src++)
		{
			return nullptr;
		}
	}
	return (T*)src;
}

template <typename T> // const variant
const T* StrChr(const T* src, int ch)
{
	return StrChr<T>((T*)src, ch);
}

//----------------------------------------------------------------------------//
// Returns a pointer to first occurrence of @str
// in @src, or nullptr (if no occurrence was found)
//    @param src - null-terminated source (donor) string
//    @param str - null-terminated string
template <typename T>
T* StrStr(T* src, const T* str)
{
	size_t len = StrLen<T>(str);
	while (*src)
	{
		for (size_t i = 0; i < len; i++)
		{
			if (src[i] != str[i])
			{
				break;
			}
			else if (i == len - 1)
			{
				return src;
			}
		}
		src++;
	}
	return nullptr;
}

template <typename T> // const variant
const T* StrStr(const T* src, const T* str)
{
	return StrStr<T>((T*)src, str);
}

//----------------------------------------------------------------------------//
// Returns a pointer to last occurrence of @str
// in @src, or nullptr (if no occurrence was found)
//    @param src - null-terminated source (donor) string
//    @param str - null-terminated string
template <typename T>
T* StrStrBack(T* src, const T* str)
{
	T* first_occurrence = StrStr<T>(src, str);
	T* last_occurrence = nullptr;

	if (first_occurrence)
	{
		// calculate string length before loop
		size_t str_len = StrLen<T>(str);

		// init last occurence
		last_occurrence = first_occurrence;

		// loop
		while (true)
		{
			// skip current string occurrence and search next one
			T* occurrence = last_occurrence + str_len;
			T* new_occurrence = StrStr<T>(occurrence, str);

			if (new_occurrence == nullptr)
			{
				break;
			}
			else
			{
				last_occurrence = new_occurrence;
			}
		}
	}
	return last_occurrence;
}

template <typename T> // const variant
const T* StrStrBack(const T* src, const T* str)
{
	return StrStrBack<T>((T*)src, str);
}

//----------------------------------------------------------------------------//
// Appends @src string to @dst string. Returns pointer to the @dst
//    @param dst - null-terminated string
//    @param src - null-terminated string
template <typename T>
T* StrCat(T* dst, const T* src)
{
	T* ret = dst;
	while (*dst)
	{
		dst++;
	}
	while (*dst++ = *src++)
	{
		// void
	}
	return ret;
}

//----------------------------------------------------------------------------//
// Appends @src string to @dst string. Returns pointer to the @dst
// Takes into account @max_size parameter.
//    @param dst - null-terminated string
//    @param src - null-terminated string
//    @param max_size - maximum number of characters in @dst string
template <typename T>
T* StrCat(T* dst, const T* src, size_t max_size)
{
	T* ret = dst;
	while (*dst)
	{
		dst++;
	}
	while ((*dst++ = *src++) && ((size_t)(dst - ret + 1) < max_size))
	{
		// void
	}
	return ret;
}

//----------------------------------------------------------------------------//
// Returns 'true' if @c is a control symbol
//    @param c - symbol
template <typename T>
bool ChControl(T c)
{
	return (c == '\a' || c == '\b' ||
	        c == '\t' || c == '\t' ||
	        c == '\n' || c == '\v' ||
	        c == '\f' || c == '\r');
}

//----------------------------------------------------------------------------//
// Returns 'true' if @c is literal symbol
//    @param c - symbol
template <typename T>
bool ChLiteral(T c)
{
	return c != ' ' && !ChControl<T>(c);
}

//----------------------------------------------------------------------------//
// Returns 'true' if @c is a number or part of a number
//    @param c - symbol
template <typename T>
bool ChIsNumber(T c)
{
	return (c == '0' || c == '1' ||
	        c == '2' || c == '3' ||
	        c == '4' || c == '5' ||
	        c == '6' || c == '7' ||
	        c == '8' || c == '9' ||
	        c == '-' || c == '+' ||
	        c == '.') ? true : false;
}

//----------------------------------------------------------------------------//
// Converts specified character to lower case
//    @param c - character to be converted
//    @return - converted character
template<typename T>
T ChToLower(T c)
{
	switch (c)
	{
	case 'A': return 'a';
	case 'B': return 'b';
	case 'C': return 'c';
	case 'D': return 'd';
	case 'E': return 'e';
	case 'F': return 'f';
	case 'G': return 'g';
	case 'H': return 'h';
	case 'I': return 'i';
	case 'J': return 'j';
	case 'K': return 'k';
	case 'L': return 'l';
	case 'M': return 'm';
	case 'N': return 'n';
	case 'O': return 'o';
	case 'P': return 'p';
	case 'Q': return 'q';
	case 'R': return 'r';
	case 'S': return 's';
	case 'T': return 't';
	case 'U': return 'u';
	case 'V': return 'v';
	case 'W': return 'w';
	case 'X': return 'x';
	case 'Y': return 'y';
	case 'Z': return 'z';
	default: return c;
	}
}

//----------------------------------------------------------------------------//
// Loads the data from the locations, defined by @arg_list, converts them to
// character string equivalents and writes the results to @buffer string.
template <typename T>
inline int VStrPrint(T* buffer, size_t buffer_size, const T* format, va_list arg_list)
{
	return 0;
}

// StrPrint spec for 'char' type
template<> inline int VStrPrint<char>(char* buffer, size_t buffer_size, const char* format, va_list arg_list)
{
#if UT_WINDOWS
	return vsprintf_s(buffer, buffer_size, format, arg_list);
#else
	return vsprintf(buffer, format, arg_list);
#endif
}

// StrPrint spec for 'wchar' type
template<> inline int VStrPrint<wchar>(wchar* buffer, size_t buffer_size, const wchar* format, va_list arg_list)
{
#if UT_WINDOWS
	return vswprintf_s(buffer, buffer_size, format, arg_list);
#else
	return vswprintf(buffer, buffer_size, format, arg_list);
#endif
}

//----------------------------------------------------------------------------//
// Reads data from the a variety of sources, interprets it according to @format
// and stores the results into string @buffer.
template <typename T>
inline int VStrScan(const T* buffer, const T* format, va_list arg_list)
{
	return 0;
}

#if (CPP_STANDARD >= 2011 || !UT_WINDOWS) && !UT_ANDROID
//----------------------------------------------------------------------------//
// StrScan spec for 'char' type
template<> inline int VStrScan<char>(const char* buffer, const char* format, va_list arg_list)
{
#if UT_WINDOWS
	return vsscanf_s(buffer, format, arg_list);
#else
	return vsscanf(buffer, format, arg_list);
#endif
}

// StrScan spec for 'wchar' type
template<> inline int VStrScan<wchar>(const wchar* buffer, const wchar* format, va_list arg_list)
{
#if UT_WINDOWS
	return vswscanf_s(buffer, format, arg_list);
#else
	return vswscanf(buffer, format, arg_list);
#endif
}
#else
// StrScanfT() is a replacement for vsscanf(), but pointer is used instead of va_list
//    @param arg_num - number of arguments('%' parameters)
//    @param p - pointer to the array of parameters
template <typename T>
inline int StrScanfT(const T* str, int arg_num, const T* format, void** p);

// StrScanfT() spec for 'char' type
template<> inline int StrScanfT<char>(const char* str, int arg_num, const char* format, void** p)
{
	switch (arg_num)
	{
	case  0: return 0;
	case  1: return sscanf(str, format, UT_ARGS_1(p));
	case  2: return sscanf(str, format, UT_ARGS_2(p));
	case  3: return sscanf(str, format, UT_ARGS_3(p));
	case  4: return sscanf(str, format, UT_ARGS_4(p));
	case  5: return sscanf(str, format, UT_ARGS_5(p));
	case  6: return sscanf(str, format, UT_ARGS_6(p));
	case  7: return sscanf(str, format, UT_ARGS_7(p));
	case  8: return sscanf(str, format, UT_ARGS_8(p));
	case  9: return sscanf(str, format, UT_ARGS_9(p));
	case 10: return sscanf(str, format, UT_ARGS_10(p));
	case 11: return sscanf(str, format, UT_ARGS_11(p));
	case 12: return sscanf(str, format, UT_ARGS_12(p));
	case 13: return sscanf(str, format, UT_ARGS_13(p));
	case 14: return sscanf(str, format, UT_ARGS_14(p));
	case 15: return sscanf(str, format, UT_ARGS_15(p));
	case 16: return sscanf(str, format, UT_ARGS_16(p));
	case 17: return sscanf(str, format, UT_ARGS_17(p));
	case 18: return sscanf(str, format, UT_ARGS_18(p));
	case 19: return sscanf(str, format, UT_ARGS_19(p));
	case 20: return sscanf(str, format, UT_ARGS_20(p));
	}
	return 0;
}

// StrScanfT() spec for 'wchar' type
template<> inline int StrScanfT<wchar>(const wchar* str, int arg_num, const wchar* format, void** p)
{
	switch (arg_num)
	{
	case  0: return 0;
	case  1: return swscanf(str, format, UT_ARGS_1(p));
	case  2: return swscanf(str, format, UT_ARGS_2(p));
	case  3: return swscanf(str, format, UT_ARGS_3(p));
	case  4: return swscanf(str, format, UT_ARGS_4(p));
	case  5: return swscanf(str, format, UT_ARGS_5(p));
	case  6: return swscanf(str, format, UT_ARGS_6(p));
	case  7: return swscanf(str, format, UT_ARGS_7(p));
	case  8: return swscanf(str, format, UT_ARGS_8(p));
	case  9: return swscanf(str, format, UT_ARGS_9(p));
	case 10: return swscanf(str, format, UT_ARGS_10(p));
	case 11: return swscanf(str, format, UT_ARGS_11(p));
	case 12: return swscanf(str, format, UT_ARGS_12(p));
	case 13: return swscanf(str, format, UT_ARGS_13(p));
	case 14: return swscanf(str, format, UT_ARGS_14(p));
	case 15: return swscanf(str, format, UT_ARGS_15(p));
	case 16: return swscanf(str, format, UT_ARGS_16(p));
	case 17: return swscanf(str, format, UT_ARGS_17(p));
	case 18: return swscanf(str, format, UT_ARGS_18(p));
	case 19: return swscanf(str, format, UT_ARGS_19(p));
	case 20: return swscanf(str, format, UT_ARGS_20(p));
	}
	return 0;
}

//----------------------------------------------------------------------------//
// VStrScanT simulates vsscanf() function, parses @buffer string into separate
// parameters, so that StrScanfT<>() could be called instead of vsscanf()
template <typename T>
inline int VStrScanT(const T* buffer, const T* format, va_list arg_list)
{
	int   arg_num = 0;
	void* arg_ptr[UT_ARG_MAX];
	const T* curr_buff = StrChr<T>(format, '%');
	T curr_char;

	if (curr_buff == nullptr)
	{
		// no valid format specifier!
		return 0;
	}

	do
	{
		// Move pointer to next character
		curr_buff++;
		curr_char = (T)(*curr_buff);

		if (curr_char == 0)
		{
			// End of string
			//      -> processing will stop!
		}
		else if (curr_char == '*')
		{
			// "%*" suppresses argument assignment
			//      -> do not get argument from stack!
		}
		else if (curr_char == '%')
		{
			// "%%" substitutes "%" character!
			//      -> do not get argument from stack!
			//      -> Increment to next character
			curr_buff++;
		}
		else
		{
			if (arg_num >= UT_ARG_MAX)
			{
				// This function can only handle UT_ARG_MAX arguments!
				return 0;
			}
			arg_ptr[arg_num++] = va_arg(arg_list, void*);
		}
		curr_buff = StrChr<T>(curr_buff, '%');
	} while (curr_buff != nullptr);

	va_end(arg_list);

	return StrScanfT(buffer, arg_num, format, arg_ptr);
}

//----------------------------------------------------------------------------//
// StrScan spec for 'char' type
template<> inline int VStrScan<char>(const char* buffer, const char* format, va_list arg_list)
{
	return VStrScanT<char>(buffer, format, arg_list);
}

// StrScan spec for 'wchar' type
template<> inline int VStrScan<wchar>(const wchar* buffer, const wchar* format, va_list arg_list)
{
	return VStrScanT<wchar>(buffer, format, arg_list);
}

#endif //CPP_STANDARD >= 2011

//----------------------------------------------------------------------------//
// Writes the C string pointed by @format to string @buffer. If format includes
// format specifiers (subsequences beginning with %), the additional arguments
// following format are formatted and inserted in the resulting string replacing
// their respective specifiers.
template <typename T>
int StrPrint(const T* buffer, size_t buffer_size, const T* format, ...)
{
	va_list args;
	va_start(args, format);
	return VStrPrint<T>(buffer, buffer_size, format, args);
}

//----------------------------------------------------------------------------//
// Reads data from string @buffer and stores them according to parameter format
// into the locations given by the additional arguments, as if scanf was used,
// but reading from @buffer instead of the standard input (stdin).
template <typename T>
int StrScan(const T* buffer, const T* format, ...)
{
	va_list args;
	va_start(args, format);
	return VStrScan<T>(buffer, format, args);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
