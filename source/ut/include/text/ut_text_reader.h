//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(text)
//----------------------------------------------------------------------------//
// ut::text::Reader class represents an interface for
// comfortable parsing of the input data.
class Reader
{
public:
	// Constructor
	Reader(const char* text);

	// Constructor
	Reader(const Reader & copy);

	// Assignment operator
	Reader& operator = (const Reader& copy);

	// Assignment operator
	Reader& operator = (const char* new_cursor);

	// Shifts pointer forward (not modifying it) and returns the result
	//    @param offset - offset in characters
	char operator [](size_t offset) const;

	// Comparison operator
	//    @param str - null-terminated string
	bool operator == (char c) const;

	// Comparison operator
	//    @param str - string to compare with
	bool operator != (char c) const;

	// Increment operator
	char operator ++();

	// Post increment operator
	char operator++(int);

	// Additive promotion operator
	const char * operator +(size_t offset) const;

	// Addition assignment operator
	Reader& operator +=(size_t offset);

	// Subtraction promotion operator
	const char * operator -(size_t offset) const;

	// Subtraction assignment operator
	Reader& operator -=(size_t offset);

	// Returns the current pointer
	const char * Get() const;

	// Checks if the string has enough characters
	//    @param length - number of characters to be checked
	//    @return - true if has @length or more characters
	bool CheckLength(size_t length) const;

	// Checks if current char sequence starts with the specified string
	//    @param str - string to be compared with
	//    @param case_sensitive - indicates if the comparison must be case sensitive
	//    @return - true if @str is equal to the current char sequence
	bool Compare(const String & str, bool case_sensitive = true) const;

private:
	const char* cursor;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(text)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//