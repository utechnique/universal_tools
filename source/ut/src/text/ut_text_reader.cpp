//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "text/ut_text_reader.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(text)
//----------------------------------------------------------------------------//
// Constructor
Reader::Reader(const char* text) : cursor(text)
{ }

// Constructor
Reader::Reader(const Reader & copy) : cursor(copy.cursor)
{ }

// Assignment operator
Reader& Reader::operator = (const Reader& copy)
{
	cursor = copy.cursor;
	return *this;
}

// Assignment operator
Reader& Reader::operator = (const char* new_cursor)
{
	cursor = new_cursor;
	return *this;
}

// Shifts pointer forward (not modifying it) and returns the result
//    @param offset - offset in characters
char Reader::operator [](size_t offset) const
{
	return cursor[offset];
}

// Comparison operator
//    @param str - null-terminated string
bool Reader::operator == (char c) const
{
	return *cursor == c;
}

// Comparison operator
//    @param str - string to compare with
bool Reader::operator != (char c) const
{
	return *cursor != c;
}

// Increment operator
char Reader::operator ++()
{
	return *(cursor++);
}

// Post increment operator
char Reader::operator++(int)
{
	char tmp = *cursor;
	cursor++;
	return tmp;
}

// Additive promotion operator
const char * Reader::operator +(size_t offset) const
{
	return cursor + offset;
}

// Addition assignment operator
Reader& Reader::operator +=(size_t offset)
{
	cursor += offset;
	return *this;
}

// Subtraction promotion operator
const char * Reader::operator -(size_t offset) const
{
	return cursor - offset;
}

// Subtraction assignment operator
Reader& Reader::operator -=(size_t offset)
{
	cursor -= offset;
	return *this;
}

// Returns the current pointer
const char * Reader::Get() const
{
	return cursor;
}

// Checks if the string has enough characters
//    @param length - number of characters to be checked
//    @return - true if has @length or more characters
bool Reader::CheckLength(size_t length) const
{
	if (cursor == nullptr)
	{
		return false;
	}

	if (length == 0)
	{
		return true;
	}

	const char* save = cursor;
	while (length-- > 0)
	{
		if (*(save++) == '\0') return false;
	}

	return true;
}

// Checks if current char sequence starts with the specified string
//    @param str - string to be compared with
//    @param case_sensitive - indicates if the comparison must be case sensitive
//    @return - true if @str is equal to the current char sequence
bool Reader::Compare(const String & str, bool case_sensitive) const
{
	const char* s0 = str.GetAddress();
	const char* s1 = cursor;

	while (*s0 != '\0')
	{
		char c0 = case_sensitive ? *s0 : ChToLower<char>(*s0);
		char c1 = case_sensitive ? *s1 : ChToLower<char>(*s1);
		if (c0 != c1)
		{
			return false;
		}
		s0++;
		s1++;
	}

	return true;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(text)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//