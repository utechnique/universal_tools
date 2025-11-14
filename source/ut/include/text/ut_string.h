//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
// This file contains definition of ut::String<T> class
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_array.h"
#include "templates/ut_optional.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// CodePage describes code pages for encoding/convertion string operations.
// Used in StrConvert<>() function to convert strings with different encoding.
enum class CodePage
{
	ascii = 0,
	utf8  = 1
};

//----------------------------------------------------------------------------//
// TString is container class for null-terminated strings.
template <typename T, class Allocator = DefaultAllocator<T> >
class TString
{
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
	// adds the null terminator to the end.
	//    @param size - number of allocated characters
	TString(size_t size)
	{
		length = size + 1;

		// sso case
		if (length <= sso_size)
		{
			sso[size] = '\0';
			return;
		}

		// heap case
		heap.Resize(length);
		heap[size] = '\0';
	}

	// Constructor, allocates @size characters, and
	// copies string content from the @ptr
	//    @param ptr - pointer to the first character of the string
	//    @param size - number of allocated characters
	TString(const T* ptr, size_t size)
	{
		UT_ASSERT(ptr != nullptr);
		if (size == 0)
		{
			length = 1;
			sso[0] = '\0';
			return;
		}

		const bool null_terminated = ptr[size - 1] == '\0';
		const size_t null_terminated_size = size + (null_terminated ? 0 : 1);
		length = null_terminated_size;

		// sso case
		if (null_terminated_size <= sso_size) 
		{
			memory::Copy(sso, ptr, size * sizeof(T));
			sso[null_terminated_size - 1] = '\0';
			return;
		}

		// heap case
		heap.Resize(null_terminated_size);
		memory::Copy(heap.GetAddress(), ptr, size * sizeof(T));
		heap[null_terminated_size - 1] = '\0';
	}

	// Copy constructor
	TString(const TString& copy) : length(copy.length)
    {
		// sso case
		if (length <= sso_size) 
		{
			memory::Copy(sso, copy.sso, length * sizeof(T));
			return;
		}

		// heap case
		heap = copy.heap;
	}

	// Move constructor
	TString(TString&& rval) noexcept : length(rval.length)
	                                 , heap(Move(rval.heap))
	{
		if (length <= sso_size)
		{
			memory::Copy(sso, rval.sso, length * sizeof(T));
		}
		rval.length = 1;
		rval.sso[0] = '\0';
	}

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
	TString& operator = (const TString& copy)
    {
		length = copy.length;

		// sso case
		if (length <= sso_size)
		{
			memory::Copy(sso, copy.sso, length * sizeof(T));
			return *this;
		}

		// heap case
		heap = copy.heap;
        return *this;
    }

	// Assignment (move) operator
	TString& operator = (TString&& rval) noexcept
	{
		// move data
		length = rval.length;
		heap = Move(rval.heap);
		if (length <= sso_size)
		{
			memory::Copy(sso, rval.sso, length * sizeof(T));
		}

		// reset r-value
		rval.length = 1;
		rval.sso[0] = '\0';

		return *this;
	}

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

	// Additive promotion operator
	TString operator +(const TString& str) const
	{
		TString out(*this);
		out.Append(str);
		return out;
	}

	// Additive promotion operator (single character)
	TString operator +(T chr) const
	{
		TString out(*this);
		out.Append(chr);
		return out;
	}

	// Addition assignment operator
	TString& operator +=(const TString& str)
	{
		Append(str);
		return *this;
	}

	// Addition assignment operator (single character)
	TString& operator +=(T chr)
	{
		Append(chr);
		return *this;
	}

	// Returns desired element
	T& operator [] (const size_t id)
	{
		UT_ASSERT(id < length);
		return length <= sso_size ? sso[id] : heap[id];
	}

	// Returns desired element
	const T& operator [] (const size_t id) const
	{
		UT_ASSERT(id < length);
		return length <= sso_size ? sso[id] : heap[id];
	}

	// Returns a pointer to the array of characters representing this string.
	const T* GetAddress() const
	{
		return length <= sso_size ? sso : heap.GetAddress();
	}

	// Returns a pointer to the array of characters representing this string.
	T* GetAddress()
	{
		return length <= sso_size ? sso : heap.GetAddress();
	}

	// Returns the length of the string, without null terminator
	inline size_t Length() const
    {
        return length - 1;
    }

	// Compares self with another string and returns the result
	//    @param str - string to compare with
	//    @return - '0' if strings are equal, or difference value
    inline int Compare(const TString& str) const
    {
		const T* left = GetAddress();
		const T* right = str.GetAddress();
		const size_t right_n = str.length;
		const size_t n = right_n < length ? right_n : length;

		for (size_t i = 0; i < n; i++)
		{
			const T c1 = left[i];
			const T c2 = right[i];

			if (c1 != c2 || c1 == '\0')
			{
				return c1 - c2;
			}
		}

		return 0;
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
		const T* s1 = GetAddress();
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
		const size_t original_size = Length();
		const size_t append_size = str.Length();
		const size_t total_length = original_size + append_size;
		if (total_length == 0 || append_size == 0)
		{
			return;
		}

		// sso->sso case
		length = total_length + 1;
		const T* right = str.GetAddress();
		if (length <= sso_size)
		{
			memory::Copy(sso + original_size, right, append_size * sizeof(T));
			sso[total_length] = '\0';
			return;
		}

		// heap->heap and sso->heap case
		heap.Resize(length);
		memory::Copy(heap.GetAddress() + original_size, right, append_size * sizeof(T));
		heap[total_length] = '\0';
		if (original_size + 1 <= sso_size)
		{
			memory::Copy(heap.GetAddress(), sso, original_size * sizeof(T));
		}
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

		length++;

		// sso case
		if (length <= sso_size)
		{
			sso[length - 2] = c;
			sso[length - 1] = '\0';
			return;
		}

		// heap case
		heap.Resize(length);
		if (length == sso_size + 1)
		{
			memory::Copy(heap.GetAddress(), sso, sso_size * sizeof(T));
		}
		heap[length - 2] = c;
		heap[length - 1] = '\0';
	}

	// Inserts a character into the desired position.
	//    @param pos - insertion position (index).
	//    @param c - character to insert.
	void Insert(size_t pos, T c)
	{
		// skip null-terminator
		if (c == '\0')
		{
			return;
		}

		length++;

		// sso->sso case
		if (length <= sso_size)
		{
			for (size_t i = length; i-- > pos + 1;)
			{
				sso[i] = sso[i - 1];
			}
			sso[pos] = c;
			return;
		}

		// heap case
		heap.Resize(length);
		if (length == sso_size + 1)
		{
			memory::Copy(heap.GetAddress(), sso, sso_size * sizeof(T));
		}
		for (size_t i = length; i-- > pos + 1;)
		{
			heap[i] = heap[i - 1];
		}
		heap[pos] = c;
	}

	// Removes the character.
	//    @param pos - position (index) of the first character to be removed.
	//    @param count - how many characters to be removed, this parameter must
	//                   not be gteater than Length() - @pos.
	void Remove(size_t pos, size_t count)
	{
		UT_ASSERT(pos + count < length);

		const size_t final_size = length - count;

		// sso->sso case
		if (length <= sso_size)
		{
			for (size_t i = pos + count; i < length; i++)
			{
				sso[i - count] = sso[i];
			}
			length = final_size;
			return;
		}

		// heap->heap case
		if (final_size > sso_size)
		{
			for (size_t i = pos + count; i < length; i++)
			{
				heap[i - count] = heap[i];
			}
			heap.Resize(final_size);
			length = final_size;
			return;
		}

		// heap->sso case
		memory::Copy(sso, heap.GetAddress(), pos * sizeof(T));
		memory::Copy(sso + pos,
		             heap.GetAddress() + pos + count,
		             (length - pos - count) * sizeof(T));
		heap.Reset();
		length = final_size;
	}

	// Replaces self with another string
	//    @param str - null-terminated string
    bool Set(const T* str)
    {
		if (str == nullptr)
		{
			heap.Reset();
			length = 1;
			sso[0] = '\0';
			return true;
		}

		// sso case
		length = StrLen<T>(str) + 1;
		if (length <= sso_size)
		{
			heap.Reset();
			memory::Copy(sso, str, length * sizeof(T));
			return true;
		}

		// heap case
		heap.Resize(length);
		memory::Copy(heap.GetAddress(), str, length * sizeof(T));
		return true;
    }

	// Resets this string.
	void Reset()
	{
		T empty[1] = {'\0'};
		Set(empty);
	}

	// Returns isolated filename string (directory is deleted)
	TString GetIsolatedFilename() const
	{
		const T* src = GetAddress();
		size_t i = Length();
		for (; i-- > 0;)
		{
			if (src[i] == '\\')
			{
				break;
			}
			if (src[i] == '/')
			{
				break;
			}
		}
		return TString(&src[++i]);
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
		size_t i = Length();
		const T* src = GetAddress();
		for (; i-- > 0;)
		{
			if (src[i] == '\\' || src[i] == '/')
			{
				break;
			}
		}
		return TString(src, include_separator ? i + 1 : i);
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
		const T clsep_str0[] = { ':', '\\', 0 };
		const T clsep_str1[] = { ':', '/', 0 };

		if (StrStr<T>(GetAddress(), clsep_str0) ||
		    StrStr<T>(GetAddress(), clsep_str1))
		{
			return true;
		}

		return false;
	}

	// Returns true if has any literal characters
	bool HasLiterals() const
	{
		const T* src = GetAddress();
		const size_t len = Length();
		for (size_t i = 0; i < len; i++)
		{
			if (ChLiteral<T>(src[i]))
			{
				return true;
			}
		}
		return false;
	}

	// Returns true if has only numeric characters
	bool IsNumber() const
	{
		const T* src = GetAddress();
		const size_t len = Length();
		for (size_t i = 0; i < len; i++)
		{
			if (!ChIsNumber<T>(src[i]))
			{
				return false;
			}
		}
		return true;
	}

	// Converts all characters to the lower case
	void ToLowerCase()
	{
		T* src = GetAddress();
		for (size_t i = 0; i < length; i++)
		{
			src[i] = ChToLower<T>(src[i]);
		}
	}

	// Searches the string for the first occurrence of the sequence specified
	// by its arguments.
	//    @param str - searched null-terminated string.
	//    @param pos - when specified, the search only includes characters
	//                 at or after position pos, ignoring any possible
	//                 occurrences that include characters before it.
	//    @return - the position of the first character of the first match.
	//              If no matches were found, the function returns nothing.
	Optional<size_t> Find(const T* str, size_t pos = 0) const
	{
		UT_ASSERT(pos < length);
		const T* src = GetAddress();
		const T* occurrence = StrStr<T>(src + pos, str);
		if (occurrence == nullptr)
		{
			return Optional<size_t>();
		}

		return occurrence - src;
	}

	// Searches the string for the first occurrence of the sequence specified
	// by its arguments.
	//    @param str - the reference to the searched string.
	//    @param pos - when specified, the search only includes characters
	//                 at or after position pos, ignoring any possible
	//                 occurrences that include characters before it.
	//    @return - the position of the first character of the first match.
	//              If no matches were found, the function returns nothing.
	Optional<size_t> Find(const TString& str, size_t pos = 0) const
	{
		return Find(str.GetAddress(), pos);
	}

	// Searches the string for the first occurrence of the character specified
	// by its argument.
	//    @param c - the reference to the searched character.
	//    @param pos - when specified, the search only includes characters
	//                 at or after position pos, ignoring any possible
	//                 occurrences that include characters before it.
	//    @return - the position of the first character of the first match.
	//              If no matches were found, the function returns nothing.
	Optional<size_t> Find(T c, size_t pos = 0) const
	{
		UT_ASSERT(pos < length);
		const T* src = GetAddress();
		const T* occurrence = StrChr<T>(src + pos, c);
		if (occurrence == nullptr)
		{
			return Optional<size_t>();
		}

		return occurrence - src;
	}

	// Returns a newly constructed string object with its value initialized
	// to a copy of a substring of this object.
	//    @param pos - position of the first character to be copied as a
	//                 substring.
	//    @param len - optional number of characters to include in the
	//                 substring, if the string is shorter, as many characters
	//                 as possible are used, if this parameter is not set then
	//                 all remaining characters are included.
	//    @return - a string object with a substring of this object.
	TString SubStr(size_t pos = 0, Optional<size_t> len = Optional<size_t>()) const
	{
		UT_ASSERT(pos < length);
		return TString(GetAddress() + pos, len ? len.Get() : (length - pos));
	}

	// Checks if the string begins with the given prefix.
	//    @param str - null-terminated c-string.
	//    @return - true if the string begins with the provided prefix,
	//              false otherwise. 
	bool StartsWith(const T* str) const
	{
		const T* s1 = (const T*)GetAddress();
		const T* s2 = (const T*)str;
		T c1, c2;
		do
		{
			c1 = (T)*s1++;
			c2 = (T)*s2++;
			if (c1 == '\0')
			{
				return c1 == c2;
			}
		} while (c1 == c2);

		return c2 == '\0';
	}

	// Checks if the string begins with the given prefix.
	//    @param str - a reference to the prefix string.
	//    @return - true if the string begins with the provided prefix,
	//              false otherwise. 
	bool StartsWith(const TString& str) const
	{
		return StartsWith(str.GetAddress());
	}

	// Checks if the string begins with the given prefix.
	//    @param c - prefix character.
	//    @return - true if the string begins with the provided prefix,
	//              false otherwise. 
	bool StartsWith(T c) const
	{
		return *GetAddress() == c;
	}

	// Checks if the string ends with the given suffix.
	//    @param str - null-terminated c-string.
	//    @return - true if the string ends with the provided suffix,
	//              false otherwise. 
	bool EndsWith(const T* str) const
	{
		const size_t own_length = Length();
		const size_t suffix_length = StrLen<T>(str);
		if (suffix_length > own_length)
		{
			return false;
		}

		return StrCmp<T>(GetAddress() + own_length - suffix_length, str);
	}

	// Checks if the string ends with the given suffix.
	//    @param str - a reference to the suffix string.
	//    @return - true if the string ends with the provided suffix,
	//              false otherwise. 
	bool EndsWith(const TString& str) const
	{
		const size_t own_length = length;
		const size_t suffix_length = str.length;
		if (suffix_length > own_length)
		{
			return false;
		}

		return StrCmp<T>(GetAddress() + own_length - suffix_length, str.GetAddress());
	}

	// Checks if the string ends with the given suffix.
	//    @param c - suffix character.
	//    @return - true if the string ends with the provided suffix,
	//              false otherwise. 
	bool EndsWith(T c) const
	{
		const size_t own_length = Length();
		if (own_length == 0)
		{
			return false;
		}

		return GetAddress()[own_length - 1] == c;
	}

	// Parses self into set of words separated by spaces, tabs, etc.
	// If something is concluded into quates - it is perceived as a single word
	//    @param words - array of strings, every parsed word will be appended to it
	void Parse(Array<TString>& words) const
	{
		const T* src = GetAddress();

		// word start index
		int start = -1;

		// if word is quoted
		bool has_quotes = false;

		// iterate every symbol
		for (size_t i = 0; i < length; i++)
		{
			if (ChLiteral<T>(src[i]))
			{
				if (start < 0)
				{
					start = (int)i;
					has_quotes = src[i] == '\"';
				}
			}
			else
			{
				// create word
				if (start >= 0 && (!has_quotes || (i > 0 && src[i-1] == '\"')))
				{
					// get word length
					size_t word_len = i - (size_t)start;
					if (has_quotes)
					{
						word_len -= 2;
					}

					// create new word
					words.Add(TString(&src[has_quotes ? start + 1 : start], sizeof(T) * word_len));

					// reset start value
					start = -1;
					has_quotes = false;
				}
			}
		}
	}

	// Takes a pattern and divides this string into an ordered list of substrings
	// by searching for the pattern, puts these substrings into an array, and
	// returns the array.
	//    @param delimiter - the pattern describing where each split should occur.
	//    @return - an Array of strings, split at each point where the delimiter
	//              occurs in the given string.
	ut::Array<TString> Split(const TString& delimiter) const
	{
		size_t pos_start = 0;
		Optional<size_t> pos_end;
		size_t delim_len = delimiter.Length();
		TString token;
		ut::Array<TString> result;

		while (pos_end = Find(delimiter, pos_start))
		{
			token = SubStr(pos_start, pos_end.Get() - pos_start);
			pos_start = pos_end.Get() + delim_len;
			result.Add(token);
		}

		result.Add(SubStr(pos_start));
		return result;
	}

	// Takes a character and divides this string into an ordered list of
	// substrings by searching for the pattern, puts these substrings into
	// an array, and returns the array.
	//    @param delimiter - the character where each split should occur.
	//    @return - an Array of strings, split at each point where the delimiter
	//              occurs in the given string.
	ut::Array<TString> Split(T delimiter) const
	{
		size_t pos_start = 0;
		Optional<size_t> pos_end;
		TString token;
		ut::Array<TString> result;

		while (pos_end = Find(delimiter, pos_start))
		{
			token = SubStr(pos_start, pos_end.Get() - pos_start);
			pos_start = pos_end.Get() + 1;
			result.Add(token);
		}

		result.Add(SubStr(pos_start));
		return result;
	}

	// Replaces separator characters with platform-specific ones:
	// for Windows all '/' symbols will be replaced with '\', vice versa for Linux
	void ReplacePlatformSeparators()
	{
		T* src = GetAddress();
		const size_t length = Length();
		for (size_t i = 0; i < length; i++)
		{
		#if UT_WINDOWS
			if (src[i] == '/')
			{
				src[i] = '\\';
			}
		#else
			if (src[i] == '\\')
			{
				src[i] = '/';
			}
		#endif
		}
	}

	// Replaces the string.
	//    @param src - desired string.
	//    @param str - another string object, whose value is copied.
	//    @param first - index of the first occurence of the string @src,
	//                   where replacing must take place.
	//    @param count - how many times to replace @src string, 0 means all.
	void Replace(const TString& src,
	             const TString& str,
	             uint32 first = 0,
	             uint32 count = 0)
	{
		const size_t src_len = src.Length();
		if (src_len == 0)
		{
			return;
		}

		const size_t str_len = str.Length();

		uint32 i = 0;
		size_t offset = 0;
		while (true)
		{
			const T* start = GetAddress();
			const T* addr = StrStr<T>(start + offset, src.GetAddress());
			if (!addr)
			{
				break;
			}
			
			const size_t prefix_len = addr - start;
			offset = prefix_len + str_len;

			if (i++ < first)
			{
				continue;
			}
			else if (count != 0 && i > first + count)
			{
				break;
			}
			
			const TString prefix(start, prefix_len);
			const TString sufix(addr + src_len);

			*this = prefix + str + sufix;
		}
	}

	// Replaces the character.
	//    @param src - desired character.
	//    @param str - another character, whose value is copied.
	//    @param first - index of the first occurence of the character @src,
	//                   where replacing must take place.
	//    @param count - how many times to replace @src character, 0 means all.
	void Replace(const T src,
	             const T chr,
	             uint32 first = 0,
	             uint32 count = 0)
	{
		uint32 i = 0;
		size_t offset = 0;
		while (true)
		{
			const T* start = GetAddress();
			const T* addr = StrChr<T>(start + offset, src);
			if (!addr)
			{
				break;
			}

			const size_t prefix_len = addr - start;
			offset = prefix_len + 1;

			if (i++ < first)
			{
				continue;
			}
			else if (count != 0 && i > first + count)
			{
				break;
			}

			const TString prefix(start, prefix_len);
			const TString sufix(addr + 1);

			*this = prefix + chr + sufix;
		}
	}

	// Replaces \r\n with \n for Linux and \n with \r\n for Windows
	void FixCarriageReturn()
	{
		T* src = GetAddress();
		for (size_t i = length; i-- > 0;)
		{
#if UT_WINDOWS
			if (src[i] == '\n')
			{
				if (i == 0 || src[i - 1] != '\r')
				{
					Insert(i, '\r');
				}
			}
#else
			if (src[i] == '\n')
			{
				if (i != 0 && src[i - 1] == '\r')
				{
					Remove(i - 1, 1);
				}
			}
#endif
		}
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
		int result;
		if (max_size <= sso_size)
		{
			result = VStrPrint<T>(sso, max_size, format, args);
			length = StrLen<T>(sso);
		}
		else
		{
			Array<T> buffer(max_size);
			result = VStrPrint<T>(buffer.GetAddress(), max_size, format, args);
			Set(buffer.GetAddress());
		}
		return result;
	}

	// The same as Print(int max_size, const T* format, ...),
	// but @max_size is set to UT_STR_MAX
	int Print(const T* format, ...)
	{
		va_list args;
		va_start(args, format);
		T buffer[256];
		int result = VStrPrint<T>(buffer, 256, format, args);
		Set(buffer);
		return result;
	}

	// Reads data from own data and stores them according to parameter format
	// into the locations given by the additional arguments, as if scanf was used,
	// but reading from @arr base member instead of the standard input (stdin).
	int Scan(const T* format, ...) const
	{
		va_list args;
		va_start(args, format);
		int result = VStrScan<T>(GetAddress(), format, args);
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

private:
	// number of characters in this string (including null-terminator)
	size_t length;

	// small string optimization buffer size
	static constexpr size_t sso_size = 16;
	T sso[sso_size];

	// heap buffer
	Array<T, Allocator> heap;
};

// Short TString types
typedef TString<char> String;
typedef TString<wchar> WString;
typedef TString<utf16char> UTF16String;

// specialize type name function for strings
template<> inline const char* Type<String>::Name() { return "string"; }
template<> inline const char* Type<WString>::Name() { return "wstring"; }

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
#if UT_WINDOWS
static const char* skFileSeparator = "\\";
#else
static const char* skFileSeparator = "/";
#endif
static const String fsep = String(skFileSeparator);

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
		TString<T1> out_str(length);
		for (size_t i = 0; i < length; i++)
		{
			out_str[i] = static_cast<T1>(src[i]);
		}
		return out_str;
	}
	else
	{
		return TString<T1>();
	}
}

// StrConvert spec for utf8: wchar -> char
template<> String StrConvert<wchar, char, CodePage::utf8>(const wchar* src);

// StrConvert spec for utf8: char -> wchar
template<> WString StrConvert<char, wchar, CodePage::utf8>(const char* src);

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
String Print(const T& t)
{
	return String();
}

// Specializations of ut::Print<>()
template<> String Print<bool>(const bool&);
template<> String Print<int8>(const int8&);
template<> String Print<byte>(const byte&);
template<> String Print<int16>(const int16&);
template<> String Print<uint16>(const uint16&);
template<> String Print<int32>(const int32&);
template<> String Print<uint32>(const uint32&);
template<> String Print<int64>(const int64&);
template<> String Print<uint64>(const uint64&);
template<> String Print<float>(const float&);
template<> String Print<double>(const double&);
template<> String Print<long double>(const long double&);
template<> String Print<void*>(void* const&);
template<> String Print<const char*>(const char* const&);
template<> String Print<String>(const String&);

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
template<> String Scan<String>(const String& str);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
