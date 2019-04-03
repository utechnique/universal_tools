//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
#include "test_task.h"
#include "test_unit.h"
//----------------------------------------------------------------------------//
extern ut::String g_test_dir;
//----------------------------------------------------------------------------//
class FileTestUnit : public TestUnit
{
public:
	FileTestUnit();
};

//----------------------------------------------------------------------------//
class CreateFolderTask : public TestTask
{
public:
	CreateFolderTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class CreateFileTask : public TestTask
{
public:
	CreateFileTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class CopyFileTask : public TestTask
{
public:
	CopyFileTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class RenameFileTask : public TestTask
{
public:
	RenameFileTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class RemoveDirectoryTask : public TestTask
{
public:
	RemoveDirectoryTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class BinaryFileOpsTask : public TestTask
{
public:
	BinaryFileOpsTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class FileTestCleanupTask : public TestTask
{
public:
	FileTestCleanupTask();
	void Execute();
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//