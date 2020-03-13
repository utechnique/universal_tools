//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "test_manager.h"
#include "containers_test.h"
#include "string_test.h"
#include "file_test.h"
#include "thread_test.h"
#include "signal_test.h"
#include "net_test.h"
#include "serialization_test.h"
#include "text_format_test.h"
#include "encryption_test.h"
#include "dbg_test.h"
//----------------------------------------------------------------------------//

TestManager::TestManager()
{
	units.Add(ut::MakeUnique<ContainersTestUnit>());
	units.Add(ut::MakeUnique<StringTestUnit>());
	units.Add(ut::MakeUnique<ThreadTestUnit>());
	units.Add(ut::MakeUnique<SignalTestUnit>());
	units.Add(ut::MakeUnique<NetTestUnit>());
	units.Add(ut::MakeUnique<TextFormatUnit>());
	units.Add(ut::MakeUnique<EncryptionTestUnit>());
	units.Add(ut::MakeUnique<SerializationTestUnit>());
	units.Add(ut::MakeUnique<DbgTestUnit>());
}

void TestManager::Execute()
{
	for (size_t i = 0; i < units.GetNum(); i++)
	{
		units[i]->Execute();

		ut::String unit_str;

		unit_str += ut::String("[") + units[i]->GetName() + ut::String("]");
		unit_str += " - Start\n";

		unit_str += units[i]->GetReport();

		unit_str += ut::String("[") + units[i]->GetName() + ut::String("]");
		unit_str += " - End\n\n";

		ut::log << unit_str;
	}

	ut::log << "Tasks failed: " << failed_test_counter.GetNum() << "\n";
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//