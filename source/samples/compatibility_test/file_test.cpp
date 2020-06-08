//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "file_test.h"
//----------------------------------------------------------------------------//
ut::String g_test_dir("");

//----------------------------------------------------------------------------//
// Unit
FileTestUnit::FileTestUnit() : TestUnit("FILE")
{
	tasks.Add(ut::MakeUnique<CreateFolderTask>());
	tasks.Add(ut::MakeUnique<CreateFileTask>());
	tasks.Add(ut::MakeUnique<CopyFileTask>());
	tasks.Add(ut::MakeUnique<RenameFileTask>());
	tasks.Add(ut::MakeUnique<RemoveDirectoryTask>());
	tasks.Add(ut::MakeUnique<BinaryFileOpsTask>());
	tasks.Add(ut::MakeUnique<FileTestCleanupTask>());
}

//----------------------------------------------------------------------------//
// Create folder
CreateFolderTask::CreateFolderTask() : TestTask("Create folder")
{ }

void CreateFolderTask::Execute()
{
	// remove existing test directory
	ut::RemoveFolder(g_test_dir + "test");

	// create folders
	report += "creating different folders: ";
	ut::Optional<ut::Error> create_dir_error_0 = ut::CreateFolder(g_test_dir + "test");
	ut::Optional<ut::Error> create_dir_error_1 = ut::CreateFolder(g_test_dir + "test/testfolder");
	ut::Optional<ut::Error> create_dir_error_2 = ut::CreateFolder(g_test_dir + "test/testfolder1");
	ut::Optional<ut::Error> create_dir_error_3 = ut::CreateFolder(g_test_dir + "test/testfolder1/testfolder2");
	if (!create_dir_error_0 && !create_dir_error_1 && !create_dir_error_2 && !create_dir_error_3)
	{
		report += "success";
	}
	else
	{
		if (create_dir_error_0)
		{
			report += ut::String("failed 0:\n") + create_dir_error_0->GetDesc();
		}
		else if (create_dir_error_1)
		{
			report += ut::String("failed 1:\n") + create_dir_error_1->GetDesc();
		}
		else if (create_dir_error_2)
		{
			report += ut::String("failed 2:\n") + create_dir_error_2->GetDesc();
		}
		else
		{
			report += ut::String("failed 3:\n") + create_dir_error_3->GetDesc();
		}

		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Create File
CreateFileTask::CreateFileTask() : TestTask("Create test file")
{ }

void CreateFileTask::Execute()
{
	// create test file
	report += "creating \"test/test.txt\": ";
	const ut::wchar wfn[] = { 't','e','s','t','/','t','e','s','t', 0442, 0435, 0441, 0442,'.','t','x','t',0 };
	ut::WString fn(wfn);
	ut::String mbfn = ut::StrConvert<ut::wchar, char, ut::cp_utf8>(fn);
	ut::File file;
	ut::Optional<ut::Error> open_error = file.Open(g_test_dir + mbfn, ut::file_access_write);
	if (open_error)
	{
		report += ut::String("failed to open a file:\n") + open_error->GetDesc();
	}
	file << "hello";

	// close file
	file.Close();
}

//----------------------------------------------------------------------------//
// Copy file
CopyFileTask::CopyFileTask() : TestTask("Copy test file")
{ }

void CopyFileTask::Execute()
{
	report += "copying file: ";

	const ut::wchar wfn[] = { 't','e','s','t','/','t','e','s','t', 0442, 0435, 0441, 0442,'.','t','x','t',0 };
	ut::WString fn(wfn);
	ut::String mbfn = ut::StrConvert<ut::wchar, char, ut::cp_utf8>(fn);
	
	ut::Optional<ut::Error> copy_error_0 = ut::CopyFile(g_test_dir + mbfn, g_test_dir + "test/copy.txt");
	ut::Optional<ut::Error> copy_error_1 = ut::CopyFile(g_test_dir + mbfn, g_test_dir + "test/testfolder1/copy.txt");
	ut::Optional<ut::Error> copy_error_2 = ut::CopyFile(g_test_dir + mbfn, g_test_dir + "test/testfolder1/testfolder2/copy.txt");
	if (!copy_error_0 && !copy_error_1 && !copy_error_2)
	{
		report += "success";
	}
	else
	{
		if (copy_error_0)
		{
			report += ut::String("failed:\n") + copy_error_0->GetDesc();
		}
		else if (copy_error_1)
		{
			report += ut::String("failed:\n") + copy_error_1->GetDesc();
		}
		else
		{
			report += ut::String("failed:\n") + copy_error_2->GetDesc();
		}
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Rename file
RenameFileTask::RenameFileTask() : TestTask("Rename test file")
{ }

void RenameFileTask::Execute()
{
	report += "renaming a file: ";
	ut::Optional<ut::Error> rename_error = ut::RenameFile(g_test_dir + "test/copy.txt", g_test_dir + "test/copy_renamed.txt");
	if (!rename_error)
	{
		report += "success";
	}
	else
	{
		report += ut::String("failed:\n") + rename_error->GetDesc();
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Remove directory
RemoveDirectoryTask::RemoveDirectoryTask() : TestTask("Remove directory")
{ }

void RemoveDirectoryTask::Execute()
{
	report += "removing a folder: ";
	ut::Optional<ut::Error> remove_error = ut::RemoveFolder(g_test_dir + "test/testfolder1");
	if (!remove_error)
	{
		report += "success";
	}
	else
	{
		report += ut::String("failed:\n") + remove_error->GetDesc();
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Binary file (read/write)
BinaryFileOpsTask::BinaryFileOpsTask() : TestTask("Binary file read/write")
{ }

void BinaryFileOpsTask::Execute()
{
	// binary file
	report += "testing: ";
	ut::File bfile;
	ut::Optional<ut::Error> open_error = bfile.Open(g_test_dir + "test/binary.txt", ut::file_access_write);
	ut::uint32 ua = 100, ub = 255;
	ut::Optional<ut::Error> bytes_written0_error = bfile.Write(&ua, sizeof(ut::uint32), 1);
	ut::Optional<ut::Error> bytes_written1_error = bfile.Write(&ub, sizeof(ut::uint32), 1);
	if (!open_error && !bytes_written0_error && !bytes_written1_error)
	{
		report += "success";
	}
	else
	{
		if (open_error)
		{
			report += ut::String("failed:\n") + open_error->GetDesc();
		}
		else
		{
			report += "failed to write data";
		}
		failed_test_counter.Increment();
	}
	bfile.Close();
}

//----------------------------------------------------------------------------//
// Cleanup
FileTestCleanupTask::FileTestCleanupTask() : TestTask("Cleanup")
{ }

void FileTestCleanupTask::Execute()
{
	report += "removing test folder: ";
	ut::Optional<ut::Error> remove_error = ut::RemoveFolder(g_test_dir + "test");
	if (!remove_error)
	{
		report += "success";
	}
	else
	{
		report += ut::String("failed:\n") + remove_error->GetDesc();
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//