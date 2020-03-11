//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
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
