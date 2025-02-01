//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "string_test.h"
//----------------------------------------------------------------------------//
// Unit
StringTestUnit::StringTestUnit() : TestUnit("STRING")
{
	tasks.Add(ut::MakeUnique<StrLengthTask>());
	tasks.Add(ut::MakeUnique<StrCmpTask>());
	tasks.Add(ut::MakeUnique<StrStrTask>());
	tasks.Add(ut::MakeUnique<StrChrTask>());
	tasks.Add(ut::MakeUnique<StrCatTask>());
	tasks.Add(ut::MakeUnique<StrConstructorTask>());
	tasks.Add(ut::MakeUnique<StrComparisonOpTask>());
	tasks.Add(ut::MakeUnique<StrAppendTask>());
	tasks.Add(ut::MakeUnique<StrRemoveTask>());
	tasks.Add(ut::MakeUnique<StrValidationTask>());
	tasks.Add(ut::MakeUnique<StrIsolateFilenameTask>());
	tasks.Add(ut::MakeUnique<StrAbsolutePathTask>());
	tasks.Add(ut::MakeUnique<StrParsingTask>());
	tasks.Add(ut::MakeUnique<StrSeparatorsTask>());
	tasks.Add(ut::MakeUnique<StrASCIITask>());
	tasks.Add(ut::MakeUnique<StrUTF8Task>());
	tasks.Add(ut::MakeUnique<StrPrintTask>());
	tasks.Add(ut::MakeUnique<StrScanTask>());
	tasks.Add(ut::MakeUnique<StrMoveTask>());
	tasks.Add(ut::MakeUnique<StrReplaceTask>());
	tasks.Add(ut::MakeUnique<StrFindTask>());
	tasks.Add(ut::MakeUnique<StrSubStrTask>());
	tasks.Add(ut::MakeUnique<StrSplitTask>());
}

//----------------------------------------------------------------------------//
// String length
StrLengthTask::StrLengthTask() : TestTask("String length")
{ }

void StrLengthTask::Execute()
{
	const char* str = "test";
	ut::uint32 strsize = (ut::uint32)ut::StrLen<char>(str);
	report.Print("calculating length (must be 4): %u", strsize);
	if (strsize != 4)
	{
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// String comparison
StrCmpTask::StrCmpTask() : TestTask("String comparison")
{ }

void StrCmpTask::Execute()
{
	const ut::wchar* wcs00 = L"one 0";
	const ut::wchar* wcs01 = L"one 0";
	const ut::wchar* wcs02 = L"on1 0";
	bool cmp01 = ut::StrCmp(wcs00, wcs01);
	bool cmp02 = ut::StrCmp(wcs00, wcs02);
	report += "comparison test (must be 'true' 'false'): ";
	report += (cmp01 ? "true " : "false ");
	report += (cmp02 ? "true " : "false ");

	if (cmp01 != true && cmp02 != false)
	{
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// String inclusion
StrStrTask::StrStrTask() : TestTask("String inclusion")
{ }

void StrStrTask::Execute()
{
	const ut::wchar* str1 = L"french cookies";
	const ut::wchar* str2 = L"cookies";
	const ut::wchar* st = ut::StrStr<ut::wchar>(str1, str2);
	ut::uint32 offset = st ? (ut::uint32)(st - str1) : 0;
	report.Print("inclusion symbol (must be 7): %u", offset);
	if (offset != 7)
	{
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Search for a character
StrChrTask::StrChrTask() : TestTask("Search for a character")
{ }

void StrChrTask::Execute()
{
	const ut::wchar* str1 = L"french cookies";
	const ut::wchar* ct = ut::StrChr<ut::wchar>(str1, L'c');
	ut::uint32 coffset = ct ? (ut::uint32)(ct - str1) : 0;
	report.Print("searching.. (must be 4): %u", coffset);
	if (coffset != 4)
	{
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Strings concatenation
StrCatTask::StrCatTask() : TestTask("Strings concatenation")
{ }

void StrCatTask::Execute()
{
	const ut::wchar* wcs0 = L"one ";
	const ut::wchar* wcs1 = L"two ";
	const ut::wchar* wcs2 = L"three 0123456789";
	ut::wchar wcs3[16] = { 0 };
	ut::StrCat<ut::wchar>(wcs3, wcs0, 16);
	ut::StrCat<ut::wchar>(wcs3, wcs1, 16);
	ut::StrCat<ut::wchar>(wcs3, wcs2, 16);
	ut::String cv_str = ut::StrConvert<ut::wchar, char, ut::cp_ascii>(wcs3);
	report += "concatenation test: ";
	report += cv_str;
	if (!ut::StrCmp<ut::wchar>(wcs3, L"one two three 0"))
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// String constructors
StrConstructorTask::StrConstructorTask() : TestTask("String constructors")
{ }

void StrConstructorTask::Execute()
{
	ut::String str_0("hello");
	ut::String str_1(str_0);
	ut::String str_2;
	str_2 = str_0;
	report += "testing constructors: ";
	report += str_0 + " " + str_1 + " " + str_2;
	if (!ut::StrCmp<char>(str_2.GetAddress(), "hello"))
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Comparison operator
StrComparisonOpTask::StrComparisonOpTask() : TestTask("Comparison operator")
{ }

void StrComparisonOpTask::Execute()
{
	ut::String str_0("hello");
	ut::String str_1(str_0);
	bool cmp_01 = str_0 == str_1;
	report += ut::String("string test (must be true): ") + (cmp_01 ? "true" : "false");
	if (!cmp_01)
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Append String
StrAppendTask::StrAppendTask() : TestTask("Append String")
{ }

void StrAppendTask::Execute()
{
	report += ut::String("append test: ");

	ut::String str_concat("0123456789AB");
	str_concat += ut::String("QolckzjmV2ajakjsfowfoqwfo");
	if (!ut::StrCmp<char>(str_concat.GetAddress(), "0123456789ABQolckzjmV2ajakjsfowfoqwfo"))
	{
		report += " failed";
		failed_test_counter.Increment();
		return;
	}

	ut::String str_c("0123456789ABCD");
	str_c.Append('E');
	if (!ut::StrCmp<char>(str_c.GetAddress(), "0123456789ABCDE"))
	{
		report += " failed";
		failed_test_counter.Increment();
		return;
	}

	str_c.Append('F');
	if (!ut::StrCmp<char>(str_c.GetAddress(), "0123456789ABCDEF"))
	{
		report += " failed";
		failed_test_counter.Increment();
		return;
	}

	str_c.Append('G');
	if (!ut::StrCmp<char>(str_c.GetAddress(), "0123456789ABCDEFG"))
	{
		report += " failed";
		failed_test_counter.Increment();
		return;
	}


	ut::String str_0("hello");
	ut::String str_1(str_0);
	ut::String str_a = str_0 + str_1;
	str_a += str_1;
	str_0.Append(str_1);
	if (str_a != "hellohellohello")
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Remove characters
StrRemoveTask::StrRemoveTask() : TestTask("Remove characters")
{ }

void StrRemoveTask::Execute()
{
	report += ut::String("remove test: ");

	ut::String str("0123456789ABCDEFGqsrdtBhJkoPzsd76");
	str.Remove(30, 1);
	if (!ut::StrCmp<char>(str.GetAddress(), "0123456789ABCDEFGqsrdtBhJkoPzs76"))
	{
		report += " failed";
		failed_test_counter.Increment();
		return;
	}

	str.Remove(12, 20);
	if (!ut::StrCmp<char>(str.GetAddress(), "0123456789AB"))
	{
		report += " failed";
		failed_test_counter.Increment();
		return;
	}

	str.Remove(4, 2);
	if (!ut::StrCmp<char>(str.GetAddress(), "01236789AB"))
	{
		report += " failed";
		failed_test_counter.Increment();
		return;
	}


	ut::String str_0("hello");
	ut::String str_1(str_0);
	ut::String str_a = str_0 + str_1;
	str_a += str_1;
	str_0.Append(str_1);
	if (str_a != "hellohellohello")
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Validate String
StrValidationTask::StrValidationTask() : TestTask("Validate String")
{ }

void StrValidationTask::Execute()
{
	ut::String str_0("hello");
	str_0.Append(0);
	str_0.Append(0);
	report.Print("validation.. container size must be 5: %u", (ut::uint32)str_0.Length());
	if (str_0.Length() != 5)
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Isolate path and filename
StrIsolateFilenameTask::StrIsolateFilenameTask() : TestTask("Filename isolation")
{ }

void StrIsolateFilenameTask::Execute()
{
	ut::String str_0("C:\\Windows\\system32\\taskmgr.exe");
	ut::String str_1(str_0);
	str_0.IsolateFilename();
	str_1.IsolateLocation();
	report += "result: ";
	report += str_0 + " and " + str_1;
	if (str_0 != "taskmgr.exe" || str_1 != "C:\\Windows\\system32\\")
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Absolute path
StrAbsolutePathTask::StrAbsolutePathTask() : TestTask("Absolute path")
{ }

void StrAbsolutePathTask::Execute()
{
	ut::String str_0("C:\\Windows\\system32\\taskmgr.exe");
	ut::String str_1(str_0);
	str_0.IsolateFilename();
	str_1.IsolateLocation();
	bool abspath0 = str_0.IsAbsolutePath();
	bool abspath1 = str_1.IsAbsolutePath();
	report += "result (must be 'false true'): ";
	report += (abspath0 ? "true " : "false ");
	report += (abspath1 ? "true" : "false");
	if (abspath0 != false || abspath1 != true)
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Parsing
StrParsingTask::StrParsingTask() : TestTask("Parsing")
{ }

void StrParsingTask::Execute()
{
	ut::String strl("one two three \"four\"  32 ");
	ut::Array<ut::String> strarr;
	strl.Parse(strarr);
	report += "parsing test: ";
	for (size_t i = 0; i < strarr.Count(); i++)
	{
		report += strarr[i] + (i != strarr.Count() - 1 ? " | " : "");
	}

	if (strarr.Count() != 5)
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Separators
StrSeparatorsTask::StrSeparatorsTask() : TestTask("Separators")
{ }

void StrSeparatorsTask::Execute()
{
	ut::String str_0("C:\\Windows/system32\\taskmgr.exe");
	str_0.ReplacePlatformSeparators();
	report += "replacing separators: ";
	report += str_0;
#if UT_WINDOWS
	if (str_0 != "C:\\Windows\\system32\\taskmgr.exe")
#elif UT_UNIX
	if (str_0 != "C:/Windows/system32/taskmgr.exe")
#else
	if (false)
#endif
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// ASCII convertion
StrASCIITask::StrASCIITask() : TestTask("ASCII convertion")
{ }

void StrASCIITask::Execute()
{
	ut::String str_0 = "french cookies";
	ut::WString wstr_0 = ut::StrConvert<char, ut::wchar, ut::cp_ascii>(str_0);
	ut::String cv_str = ut::StrConvert<ut::wchar, char, ut::cp_ascii>(wstr_0);
	report += ut::String("converting: ") + cv_str;
	if (wstr_0 != L"french cookies")
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// UTF8 convertion
StrUTF8Task::StrUTF8Task() : TestTask("UTF8 convertion")
{ }

void StrUTF8Task::Execute()
{
	ut::WString wstr_0 = L"french cookies è áóëî÷êè. ABCDEF!";
	ut::String str_0 = ut::StrConvert<ut::wchar, char, ut::cp_utf8>(wstr_0);
	wstr_0 = ut::StrConvert<char, ut::wchar, ut::cp_utf8>(str_0);
	ut::String cv_str = ut::StrConvert<ut::wchar, char, ut::cp_ascii>(wstr_0);
	report += cv_str;
	if (wstr_0 != L"french cookies è áóëî÷êè. ABCDEF!")
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Print test
StrPrintTask::StrPrintTask() : TestTask("Print functionality")
{ }

void StrPrintTask::Execute()
{
	ut::String str_0;
	str_0.Print("test (must be 24, 0.3): %u, %.1f", (ut::uint32)24, (float)0.311f);
	report += str_0;
	if (str_0 != "test (must be 24, 0.3): 24, 0.3")
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Scan test
StrScanTask::StrScanTask() : TestTask("Scan functionality")
{ }

void StrScanTask::Execute()
{
	ut::String str_0 = "12 31; 0.025";
	ut::uint32 u0 = 0;
	ut::uint32 u1 = 0;
	float f0 = 0.0f;
	str_0.Scan("%u %u; %f", &u0, &u1, &f0);
	report.Print("test (must be 12 31 0.025): %u, %u, %f", u0, u1, f0);
	if (u0 != 12 || u1 != 31)
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Move constructor
StrMoveTask::StrMoveTask() : TestTask("Move constructor")
{ }

void StrMoveTask::Execute()
{
	ut::String str_0 = "move test";
	ut::String str_1 = ut::Move(str_0);
	report += ut::String("move test (must be\"move test\"): ") + str_1;
	if (str_1 != "move test")
	{
		report += " failed";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success";
	}
}

//----------------------------------------------------------------------------//
// Replace string
StrReplaceTask::StrReplaceTask() : TestTask("Replace")
{ }

void StrReplaceTask::Execute()
{
	const ut::String original = "123 abc 123 bca 123 ddf";
	const ut::String src = "123";
	const ut::String replace = "321";

	report += ut::String("original string: ") + original + ut::cret;

	ut::String test = original;
	test.Replace(src, replace);
	report += ut::String("     replace all: ") + test + ut::cret;
	if (test != "321 abc 321 bca 321 ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	test = original;
	test.Replace(src, replace, 0, 1);
	report += ut::String("     replace first: ") + test + ut::cret;
	if (test != "321 abc 123 bca 123 ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	test = original;
	test.Replace(src, replace, 1, 1);
	report += ut::String("     replace second: ") + test + ut::cret;
	if (test != "123 abc 321 bca 123 ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	test = original;
	test.Replace(src, replace, 0, 2);
	report += ut::String("     replace first two: ") + test + ut::cret;
	if (test != "321 abc 321 bca 123 ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	test = original;
	test.Replace(src, replace, 1, 0);
	report += ut::String("     replace all except first: ") + test + ut::cret;
	if (test != "123 abc 321 bca 321 ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	test = "";
	test.Replace(src, "", 0, 0);
	report += ut::String("     replace empty: ") + test + ut::cret;
	if (test != "")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	test = ut::String(" ") + original;
	test.Replace(src, replace, 0, 1);
	report += ut::String("     replace first (2): ") + test + ut::cret;
	if (test != " 321 abc 123 bca 123 ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	test = original;
	test.Replace('2', '@');
	report += ut::String("     replace character 2 with @: ") + test + ut::cret;
	if (test != "1@3 abc 1@3 bca 1@3 ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	report += "     Success.";
}

//----------------------------------------------------------------------------//
// Find string
StrFindTask::StrFindTask() : TestTask("Find")
{ }

void StrFindTask::Execute()
{
	const ut::String src = "123 abc 123 bca 123 ddf";
	const ut::String search = "23";
	const ut::String non_existent = "32";

	ut::Optional<size_t> occurrence = src.Find(search);
	if (!occurrence || occurrence.Get() != 1)
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	occurrence = src.Find(search, 11);
	if (!occurrence || occurrence.Get() != 17)
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	occurrence = src.Find(non_existent);
	if (occurrence)
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	occurrence = src.Find('c');
	if (!occurrence || occurrence.Get() != 6)
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	occurrence = src.Find('c', 8);
	if (!occurrence || occurrence.Get() != 13)
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	report += "Success.";
}

//----------------------------------------------------------------------------//
// Substring
StrSubStrTask::StrSubStrTask() : TestTask("Substring")
{ }

void StrSubStrTask::Execute()
{
	const ut::String src = "123 abc 123 bca 123 ddf";
	
	ut::String substr = src.SubStr(2);
	if (substr != "3 abc 123 bca 123 ddf")
	{
		report += ut::String("Failed. ") + substr + "instead of 3 abc 123 bca 123 ddf";
		failed_test_counter.Increment();
		return;
	}

	substr = src.SubStr(4, 5);
	if (substr != "abc 1")
	{
		report += ut::String("Failed. ") + substr + "instead of bc 12";
		failed_test_counter.Increment();
		return;
	}

	substr = src.SubStr();
	if (substr != src)
	{
		report += ut::String("Failed. ") + substr + "instead of " + src;
		failed_test_counter.Increment();
		return;
	}

	report += "Success.";
}

//----------------------------------------------------------------------------//
// Split
StrSplitTask::StrSplitTask() : TestTask("Split")
{ }

void StrSplitTask::Execute()
{
	const ut::String src = "123-=@abc-=@123-=@bca-=@123-=@ddf";
	ut::Array<ut::String> substrs = src.Split("-=@");

	if (substrs.Count() != 6 ||
		substrs[0] != "123" ||
		substrs[1] != "abc" || 
		substrs[2] != "123" || 
		substrs[3] != "bca" || 
		substrs[4] != "123" || 
		substrs[5] != "ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	substrs = src.Split('=');
	if (substrs.Count() != 6 ||
		substrs[0] != "123-" ||
		substrs[1] != "@abc-" ||
		substrs[2] != "@123-" ||
		substrs[3] != "@bca-" ||
		substrs[4] != "@123-" ||
		substrs[5] != "@ddf")
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	substrs = src.Split("#");
	if (substrs.Count() != 1 ||
		substrs[0] != src)
	{
		report += "Failed.";
		failed_test_counter.Increment();
		return;
	}

	report += "Success.";
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//