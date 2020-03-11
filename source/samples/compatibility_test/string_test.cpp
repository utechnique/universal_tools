//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "string_test.h"
//----------------------------------------------------------------------------//
// Unit
StringTestUnit::StringTestUnit() : TestUnit("STRING")
{
	tasks.Add(new StrLengthTask);
	tasks.Add(new StrCmpTask);
	tasks.Add(new StrStrTask);
	tasks.Add(new StrChrTask);
	tasks.Add(new StrCatTask);
	tasks.Add(new StrConstructorTask);
	tasks.Add(new StrComparisonOpTask);
	tasks.Add(new StrAppendTask);
	tasks.Add(new StrValidationTask);
	tasks.Add(new StrIsolateFilenameTask);
	tasks.Add(new StrAbsolutePathTask);
	tasks.Add(new StrExtensionTask);
	tasks.Add(new StrParsingTask);
	tasks.Add(new StrSeparatorsTask);
	tasks.Add(new StrASCIITask);
	tasks.Add(new StrUTF8Task);
	tasks.Add(new StrPrintTask);
	tasks.Add(new StrScanTask);
	tasks.Add(new StrMoveTask);
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
	ut::String str_0("hello");
	ut::String str_1(str_0);
	ut::String str_a = str_0 + str_1;
	str_a += str_1;
	str_0.Append(str_1);
	report += ut::String("append test: ") + str_0 + "|" + str_a;
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
	str_0.Add(0);
	str_0.Add(0);
	str_0.Validate();
	report.Print("validation.. container size must be 6: %u", (ut::uint32)str_0.GetNum());
	if (str_0.GetNum() != 6)
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
// Extension
StrExtensionTask::StrExtensionTask() : TestTask("Filename Extension")
{ }

void StrExtensionTask::Execute()
{
	ut::String str_0("taskmgr.exe");
	ut::String str_1(str_0);
	str_0.GetExtension(str_1);
	str_0.CropExtension();
	bool has_literals = str_0.HasLiterals();
	report += "extension test: ";
	report += str_1 + " " + str_0;
	if (str_1 != "exe" || str_0 != "taskmgr")
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
	for (size_t i = 0; i < strarr.GetNum(); i++)
	{
		report += strarr[i] + (i != strarr.GetNum() - 1 ? " | " : "");
	}

	if (strarr.GetNum() != 5)
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
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//