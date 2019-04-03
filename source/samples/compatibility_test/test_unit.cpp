//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "test_unit.h"
//----------------------------------------------------------------------------//

TestUnit::TestUnit(const ut::String& unit_name) : name(unit_name)
{ }

ut::String TestUnit::GetName()
{
	return name;
}

ut::String TestUnit::GetReport()
{
	return report;
}

void TestUnit::Execute()
{
	for (size_t i = 0; i < tasks.GetNum(); i++)
	{
		tasks[i]->Execute();
		
		ut::String task_id_str;
		task_id_str.Print("[%u] ", (ut::uint32)i);

		report += task_id_str;
		report += ut::String("(") + tasks[i]->GetName() + ut::String(") ");
		report += tasks[i]->GetReport() + ut::String("\n");
	}
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//