//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
// This file contains definition of ut::String<T> class
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_array.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// CodePage describes code pages for encoding/convertion string operations.
// Used in StrConvert<>() function to convert strings with different encoding.
enum CodePage
{
	cp_ascii = 0,
	cp_utf8  = 1
};

//----------------------------------------------------------------------------//
// TString is container class for null-terminated strings, it is derived from
// Array class, and has similar functionality.
template <typename T>
class TString : public Array<T>
{
typedef Array<T> Base;
public:
	// Default constructor, initialized with @str
	//    @param str - null-terminated string
	TString(const T* str = nullptr)
    {
		if (!Set(str))
		{
			ThrowError(error::out_of_memory);
		}
    }

	// Constructor, allocates @size characters, and
	// sets the last one as null terminator
	//    @param size - number of allocated characters
	TString(size_t size) : Base(size + 1)
	{
		Base::arr[size] = 0;
	}

	// Constructor, allocates @size characters, and
	// copies string content from the @ptr
	//    @param ptr - pointer to the first character of the string
	//    @param size - number of allocated characters
	TString(const T* ptr, size_t size) : Base(size + 1)
	{
		memory::Copy(Base::arr, ptr, size);
	}

	// Copy constructor
	TString(const TString& copy) : Base(copy)
    { }

	// Move constructor
#if CPP_STANDARD >= 2011
	TString(TString && copy) : Base(Move(copy))
	{ }
#endif // CPP_STANDARD >= 2011

	// Assignment operator
	TString& operator = (const T* str)
    {
		if (!Set(str))
		{
			ThrowError(error::out_of_memory);
		}
        return *this;
    }
    
    // Assignment operator
	TString& operator = (const TString& str)
    {
		Base::operator = (str);
        return *this;
    }

	// Assignment (move) operator
#if CPP_STANDARD >= 2011
	TString& operator = (TString && str)
	{
		return static_cast<TString&>(Base::operator = (Move(str)));
	}
#endif // CPP_STANDARD >= 2011

	// Comparison operator
	//    @param str - string to compare with
    bool operator == (const TString& r_str) const
    {
        return Compare(r_str) == 0;
    }

	// Comparison operator
	//    @param str - null-terminated string
    bool operator == (const T* r_str) const
    {
        return Compare(r_str) == 0;
    }

	// Comparison operator
	//    @param str - string to compare with
	bool operator != (const TString& r_str) const
	{
		return Compare(r_str) != 0;
	}

	// Comparison operator
	//    @param str - null-terminated string
	bool operator != (const T* r_str) const
	{
		return Compare(r_str) != 0;
	}

	// Comparison operator 'less than'
	bool operator < (const TString& r_str) const
	{
		return Compare(r_str) < 0 ? true : false;
	}

	// Comparison operator 'less than or equal'
	bool operator <= (const TString& r_str) const
	{
		return Compare(r_str) <= 0 ? true : false;
	}

	// Comparison operator 'greater than'
	bool operator > (const TString& r_str) const
	{
		return Compare(r_str) > 0 ? true : false;
	}

	// Comparison operator 'greater than or equal'
	bool operator >= (const TString& r_str) const
	{
		return Compare(r_str) >= 0 ? true : false;
	}

	// Bool conversion operator
	inline operator const T*() const
	{
		return Base::GetAddress();
	}

	// Additive promotion operator
	TString operator +(const TString& str) const
	{
		TString out(Base::arr);
		out.Append(str);
		return out;
	}

	// Addition assignment operator
	TString& operator +=(const TString& str)
	{
		Append(str);
		return *this;
	}

	// Returns the length of the string, without null terminator
	inline size_t Length() const
    {
        return StrLen<T>(Base::arr);
    }

	// Compares self with another string and returns the result
	//    @param str - string to compare with
	//    @return - '0' if strings are equal, or difference value
    inline int Compare(const TString& source) const
    {
        return Compare(source.arr);
    }

	// Compares self with another string (case insensitive) and returns the result
	//    @param str - string to compare with
	//    @return - '0' if strings are equal, or difference value
	inline int CompareCaseInsensitive(const TString& source) const
	{
		TString str0(*this);
		TString str1(source);
		str0.ToLowerCase();
		str1.ToLowerCase();
		return str0.Compare(str1);
	}

	// Compares self with another string and returns the result
	//    @param str - string to compare with
	//    @return - '0' if strings are equal, or difference value
	inline int Compare(const T* source) const
	{
		const T* s1 = (const T*)Base::arr;
		const T* s2 = (const T*)source;
		T c1, c2;
		do
		{
			c1 = (T)*s1++;
			c2 = (T)*s2++;
			if (c1 == '\0')
			{
				return c1 - c2;
			}
		} while (c1 == c2);
		return c1 - c2;
	}

	// Starts a new line with @line
	//    @param line - string, should not start with carriage return symbol
	void AddLine(const TString& line)
	{
		Append(CRet() + line);
	}

	// Appends another string
	//    @param str - string to append
	void Append(const TString& str)
	{
		// delete null terminator at the end
		size_t self_length = Length();
		Base::Resize(self_length);

		// add string
		size_t src_length = str.Length();
		for (size_t i = 0; i < src_length; i++)
		{
			Base::Add(str[i]);
		}

		// add null terminator
		Base::Add(0);
	}

	// Appends a character
	//    @param c - character to append
	void Append(T c)
	{
		// skip null-terminator
		if (c == '\0')
		{
			return;
		}

		// add character and null terminator at the end
		size_t length = Length();
		Base::Resize(length + 2);
		Base::arr[length] = c;
		Base::arr[length + 1] = '\0';
	}

	// Replaces self with another string
	//    @param str - null-terminated string
    bool Set(const T* str)
    {
        if (str)
        {
            Base::Empty();
			size_t length = StrLen<T>(str) + 1;
			if (!Base::Resize(length))
			{
				return false;
			}
			memory::Copy(Base::arr, str, length * sizeof(T));
        }
		else
		{
			Base::Empty();
			Base::Add(0);
		}

		return true;
    }

	// Replaces self with empty string
	void SetEmpty()
	{
		Base::Empty();
		Base::Add(0);
	}

	// Crops unused (excess) null terminators at the end,
	// also puts null terminator if none was found
	void Validate()
	{
		T* nt = nullptr;
		size_t nt_id;
		for (nt_id = 0; nt_id < Base::GetNum(); nt_id++)
		{
			if (Base::arr[nt_id] == 0)
			{
				nt = &Base::arr[nt_id];
				break;
			}
		}

		if (nt)
		{
			Base::Resize(nt_id + 1);
		}
		else
		{
			Base::Add(0);
		}
	}

	// Returns isolated filename string (directory is deleted)
	TString GetIsolatedFilename() const
	{
		int i = (int)Length() - 1;
		for ( ; i >= 0; i--)
		{
			if (Base::arr[i] == '\\')
			{
				break;
			}
			if (Base::arr[i] == '/')
			{
				break;
			}
		}
		return TString(&Base::arr[++i]);
	}

	// Leaves only filename, directory is deleted
	//    @return 'true' if success, 'false' otherwise
	bool IsolateFilename()
	{
		TString tmp(GetIsolatedFilename());
		return Set(tmp.GetAddress());
	}

	// Returns isolated location string (filename is deleted)
	//    @param include_separator - whether to leave last separator symbol or not
	TString GetIsolatedLocation(bool include_separator = true) const
	{
		int i = (int)Length() - 1;
		for ( ; i >= 0; i--)
		{
			if (Base::arr[i] == '\\')
			{
				break;
			}
			if (Base::arr[i] == '/')
			{
				break;
			}
		}

		TString str;
		str.Empty();
		int amount = include_separator ? i : i - 1;
		for (i = 0; i <= amount; i++)
		{
			str.Add(Base::arr[i]);
		}
		str.Add(0);

		return str;
	}

	// Leaves only location, filename is deleted
	//    @param include_separator - whether to leave last separator symbol or not
	//    @return 'true' if success, 'false' otherwise
	bool IsolateLocation(bool include_separator = true)
	{
		TString tmp(GetIsolatedLocation(include_separator));
		return Set(tmp.GetAddress());
	}

	// Returns 'true' if has absolute path inside
	bool IsAbsolutePath(void) const
	{
		T clsep_str0[] = { ':', '\\', 0 };
		T clsep_str1[] = { ':', '/', 0 };

		if (StrStr<T>(Base::arr, clsep_str0) ||
		   StrStr<T>(Base::arr, clsep_str1))
		{
			return true;
		}

		return false;
	}

	// Deletes extension (ending)
	void CropExtension()
	{
		for (size_t i = Length() - 1; i > 0; i--)
		{
			if (Base::arr[i] == '.')
			{
				Base::arr[i] = 0;
			}
		}
		Validate();
	}

	// If has an extension - puts it to the @out_str and returns 'true'
	// otherwise - just returns 'false'
	//    @param out_str - extension will be put inside this string object
	bool GetExtension(TString& out_str) const
	{
		for (size_t i = Length() - 1; i > 0; i--)
		{
			if (Base::arr[i] == '.')
			{
				out_str = &Base::arr[i+1];
				return true;
			}
		}
		return false;
	}

	// Returns true if has any literal characters
	bool HasLiterals() const
	{
		size_t len = Length();
		for (size_t i = 0; i < len; i++)
		{
			if (ChLiteral<T>(Base::arr[i]))
			{
				return true;
			}
		}
		return false;
	}

	// Converts all characters to the lower case
	void ToLowerCase()
	{
		for (size_t i = 0; i < Base::num; i++)
		{
			Base::arr[i] = ChToLower<T>(Base::arr[i]);
		}
	}

	// Parses self into set of words separated by spaces, tabs, etc.
	// If something is concluded into quates - it is perceived as a single word
	//    @param words - array of strings, every parsed word will be appended to it
	void Parse(Array<TString>& words) const
	{
		// string length
		size_t len = Length() + 1;

		// word start index
		int start = -1;

		// if word is quoted
		bool has_quotes = false;

		// iterate every symbol
		for (size_t i = 0; i < len; i++)
		{
			if (ChLiteral<T>(Base::arr[i]))
			{
				if (start < 0)
				{
					start = (int)i;
					has_quotes = Base::arr[i] == '\"';
				}
			}
			else
			{
				// create word
				if (start >= 0 && (!has_quotes || (i > 0 && Base::arr[i-1] == '\"')))
				{
					// get word length
					size_t word_len = i - (size_t)start;
					if (has_quotes)
					{
						word_len -= 2;
					}

					// create new word
					TString new_word;
					new_word.Resize(word_len);
					memory::Copy(new_word.GetAddress(), &Base::arr[has_quotes ? start + 1 : start], sizeof(T)*(has_quotes ? word_len : word_len));
					new_word.Add(0);

					// add to the array
					words.Add(new_word);

					// reset start value
					start = -1;
					has_quotes = false;
				}
			}
		}
	}

	// Replaces separator characters with platform-specific ones:
	// for Windows all '/' symbols will be replaced with '\', vice versa for Linux
	void ReplacePlatformSeparators()
	{
		size_t size = Base::GetSize();
		for (size_t i = 0; i < size; i++)
		{
		#if UT_WINDOWS
			if (Base::arr[i] == '/')
			{
				Base::arr[i] = '\\';
			}
		#else
			if (Base::arr[i] == '\\')
			{
				Base::arr[i] = '/';
			}
		#endif
		}
	}

	// Replaces \r\n with \n for Linux and \n with \r\n for Windows
	void FixCarriageReturn()
	{
		size_t size = Base::GetNum();
		for (size_t i = size; i-- > 0;)
		{
#if UT_WINDOWS
			if (Base::arr[i] == '\n')
			{
				if (i == 0 || Base::arr[i - 1] != '\r')
				{
					Base::Insert(i, '\r');
				}
			}
#else
			if (Base::arr[i] == '\n')
			{
				if (i != 0 && Base::arr[i - 1] == '\r')
				{
					Base::Remove(i - 1);
				}
			}
#endif
		}
		Validate();
	}

	// Writes the C string pointed by @format. If @format includes format 
	// specifiers (subsequences beginning with %), the additional arguments
	// following format are formatted and inserted in the resulting string
	// replacing their respective specifiers.
	//    @param max_size - size capacity for string buffer
	//    @param format - classic printf() format string
	//    @return - number of printed characters, negative if error occurred
	int Print(int max_size, const T* format, ...)
	{
		va_list args;
		va_start(args, format);
		Base::Resize(max_size);
		int result = VStrPrint<T>(Base::arr, max_size, format, args);
		Validate();
		return result;
	}

	// The same as Print(int max_size, const T* format, ...),
	// but @max_size is set to UT_STR_MAX
	int Print(const T* format, ...)
	{
		va_list args;
		va_start(args, format);
		Base::Resize(UT_STR_MAX);
		int result = VStrPrint<T>(Base::arr, UT_STR_MAX, format, args);
		Validate();
		return result;
	}

	// Reads data from own data and stores them according to parameter format
	// into the locations given by the additional arguments, as if scanf was used,
	// but reading from @arr base member instead of the standard input (stdin).
	int Scan(const T* format, ...) const
	{
		va_list args;
		va_start(args, format);
		int result = VStrScan<T>(Base::arr, format, args);
		return result;
	}

	// Returns platform-specific carriage return sequense
	static TString<T> CRet()
	{
		TString<T> str;
#if UT_WINDOWS
		str.Append((T)'\r');
		str.Append((T)'\n');
#elif UT_UNIX
		str.Append((T)'\n');
#else
#error ut::CarriageReturn<>() is not implemented
#endif
		return str;
	}
};

// Short TString types
typedef TString<char> String;
typedef TString<wchar> WString;
typedef TString<utf16char> UTF16String;

//----------------------------------------------------------------------------//
// returns platform-specific carriage return sequense
template<typename T>
TString<T> CarriageReturn()
{
	static const TString<T> cret = TString<T>::CRet();
	return cret;
}
typedef String(*CarriageReturnType)();
CarriageReturnType const CRet = &CarriageReturn<char>;
static const String cret = CRet();

//----------------------------------------------------------------------------//
// Converts @src string(type @T0) and returns result string(type @T1)
//    @param src - source null-terminated string
template <typename T0, typename T1, CodePage cp>
TString<T1> StrConvert(const T0* src)
{
	if (src == nullptr)
	{
		return TString<T1>();
	}

	size_t length = StrLen<T0>(src);

	if (length > 0)
	{
		TString<T1> out_str;
		out_str.Empty();
		for (size_t i = 0; i < length; i++)
		{
			T1 c = (T1)src[i];
			out_str.Add(c);
		}
		out_str.Add(0);
		return out_str;
	}
	else
	{
		return TString<T1>();
	}
}

// StrConvert spec for utf8: wchar -> char
template<> String StrConvert<wchar, char, cp_utf8>(const wchar* src);

// StrConvert spec for utf8: char -> wchar
template<> WString StrConvert<char, wchar, cp_utf8>(const char* src);

// StrConvert variant with @src parameter as a string object
template <typename T0, typename T1, CodePage cp>
TString<T1> StrConvert(const TString<T0> &src)
{
	return StrConvert<T0, T1, cp>(src.GetAddress());
}

//----------------------------------------------------------------------------//
// Converts @t value to the text form and returns resulting string
//    @param t - custom value
//    @return - text version of the @t value
template <typename T>
String Print(T t)
{
	return String();
}

// Specializations of ut::Print<>()
template<> String Print<bool>(bool);
template<> String Print<int8>(int8);
template<> String Print<byte>(byte);
template<> String Print<int16>(int16);
template<> String Print<uint16>(uint16);
template<> String Print<int32>(int32);
template<> String Print<uint32>(uint32);
template<> String Print<int64>(int64);
template<> String Print<uint64>(uint64);
template<> String Print<float>(float);
template<> String Print<double>(double);
template<> String Print<long double>(long double);
template<> String Print<void*>(void*);
template<> String Print<const char*>(const char*);
template<> String Print<const String &>(const String &);

//----------------------------------------------------------------------------//
// Converts a string to the native binary type
//    @param str - string to be scanned
//    @return - binary version of @str text value
template <typename T>
T Scan(const String& str)
{
	return 0;
}

// Specializations of ut::Print<>()
template<> bool Scan<bool>(const String& str);
template<> int8 Scan<int8>(const String& str);
template<> byte Scan<byte>(const String& str);
template<> int16 Scan<int16>(const String& str);
template<> uint16 Scan<uint16>(const String& str);
template<> int32 Scan<int32>(const String& str);
template<> uint32 Scan<uint32>(const String& str);
template<> int64 Scan<int64>(const String& str);
template<> uint64 Scan<uint64>(const String& str);
template<> float Scan<float>(const String& str);
template<> double Scan<double>(const String& str);
template<> long double Scan<long double>(const String& str);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
