//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
#include "test_task.h"
#include "test_unit.h"
//----------------------------------------------------------------------------//
class StringTestUnit : public TestUnit
{
public:
	StringTestUnit();
};

//----------------------------------------------------------------------------//
class StrLengthTask : public TestTask
{
public:
	StrLengthTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrCmpTask : public TestTask
{
public:
	StrCmpTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrStrTask : public TestTask
{
public:
	StrStrTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrChrTask : public TestTask
{
public:
	StrChrTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrCatTask : public TestTask
{
public:
	StrCatTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrConstructorTask : public TestTask
{
public:
	StrConstructorTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrComparisonOpTask : public TestTask
{
public:
	StrComparisonOpTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrAppendTask : public TestTask
{
public:
	StrAppendTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrValidationTask : public TestTask
{
public:
	StrValidationTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrIsolateFilenameTask : public TestTask
{
public:
	StrIsolateFilenameTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrAbsolutePathTask : public TestTask
{
public:
	StrAbsolutePathTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrExtensionTask : public TestTask
{
public:
	StrExtensionTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrParsingTask : public TestTask
{
public:
	StrParsingTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrSeparatorsTask : public TestTask
{
public:
	StrSeparatorsTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrASCIITask : public TestTask
{
public:
	StrASCIITask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrUTF8Task : public TestTask
{
public:
	StrUTF8Task();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrPrintTask : public TestTask
{
public:
	StrPrintTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrScanTask : public TestTask
{
public:
	StrScanTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class StrMoveTask : public TestTask
{
public:
	StrMoveTask();
	void Execute();
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//