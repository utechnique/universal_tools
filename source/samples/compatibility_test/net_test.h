//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
#include "test_task.h"
#include "test_unit.h"
//----------------------------------------------------------------------------//
class NetTestUnit : public TestUnit
{
public:
	NetTestUnit();
};

//----------------------------------------------------------------------------//
class HostNameTask : public TestTask
{
public:
	HostNameTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class IpFromNameLHTask : public TestTask
{
public:
	IpFromNameLHTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class GoogleIpTask : public TestTask
{
public:
	GoogleIpTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class ClientServerCommTask : public TestTask
{
public:
	ClientServerCommTask();
	void Execute();
	void AddReport(const ut::String& str);
private:
	ut::Mutex report_mutex;
};

//----------------------------------------------------------------------------//
class ClientTestJob : public ut::Job
{
public:
	ClientTestJob(ClientServerCommTask& owner);
	void Execute();
private:
	ut::UniquePtr<ut::net::Socket> socket;
	ClientServerCommTask& owner;
};

//----------------------------------------------------------------------------//
class ServerTestJob : public ut::Job
{
public:
	ServerTestJob(ClientServerCommTask& owner);
	void Execute();
private:
	ut::UniquePtr<ut::net::Socket> socket;
	ClientServerCommTask& owner;
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//