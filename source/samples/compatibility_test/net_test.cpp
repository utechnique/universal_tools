//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net_test.h"
//----------------------------------------------------------------------------//
// Unit
NetTestUnit::NetTestUnit() : TestUnit("NETWORK")
{
	tasks.Add(new HostNameTask);
	tasks.Add(new IpFromNameLHTask);
	//tasks.Add(new GoogleIpTask);
	tasks.Add(new ClientServerCommTask);
}

//----------------------------------------------------------------------------//
// Host name
HostNameTask::HostNameTask() : TestTask("Host name") {}

void HostNameTask::Execute()
{
	report += "current machine network name: ";
	ut::Result<ut::String, ut::Error> hostname = ut::net::GetHostName();
	if (hostname)
	{
		report += hostname.GetResult();
	}
	else
	{
		report += " failed\n";
		report += hostname.GetAlt().GetDesc();
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Host name
IpFromNameLHTask::IpFromNameLHTask() : TestTask("Ip from name - localhost") {}

void IpFromNameLHTask::Execute()
{
	report += "from domain name (localhost): ";
	ut::Result<ut::String, ut::Error> localhost = ut::net::GetHostByName("localhost");
	if (localhost)
	{
		report += localhost.GetResult();
	}
	else
	{
		report += " failed\n";
		report += localhost.GetAlt().GetDesc();
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
// Google ip
GoogleIpTask::GoogleIpTask() : TestTask("Google IP") {}

void GoogleIpTask::Execute()
{
	report += "from domain name (www.google.com): ";
	ut::Result<ut::String, ut::Error> hostip = ut::net::GetHostByName("www.google.com");
	if (hostip)
	{
		report += hostip.GetResult();
	}
	else
	{
		report += "failed, not considered fatal error (no internet connection?)\n";
		report += hostip.GetAlt().GetDesc();
	}
}

//----------------------------------------------------------------------------//
// Launcher
ClientServerCommTask::ClientServerCommTask() : TestTask("Client-server communication") {}

void ClientServerCommTask::Execute()
{
	report += "performing client-server communication via localhost:";
	ut::UniquePtr<ut::Job> client_job(new ClientTestJob(*this));
	ut::UniquePtr<ut::Job> server_job(new ServerTestJob(*this));
	ut::ThreadPtr client_thread(new ut::Thread(Move(client_job)));
	ut::ThreadPtr server_thread(new ut::Thread(Move(server_job)));

	// wait for both threads
	client_thread.Delete();
	server_thread.Delete();
}

void ClientServerCommTask::AddReport(const ut::String& str)
{
	ut::ScopeLock sl(report_mutex);
	report += "\n";
	report += str;
}

//----------------------------------------------------------------------------//
// Client job
ClientTestJob::ClientTestJob(ClientServerCommTask& in_owner) : owner(in_owner)
{ }

void ClientTestJob::Execute()
{
	ut::String report;
	try
	{
		socket = new ut::net::Socket("127.0.0.1", 50000);

		bool connected = false;

		for (ut::uint32 iteration = 0; iteration < 10; iteration++)
		{
			ut::Optional<ut::Error> connect_error = socket->Connect();
			if (!connect_error)
			{
				report += "    client socket connected to the server\n";

				// send integer
				ut::int32 integer = 1844;
				ut::Result<int, ut::Error> send_result_0 = socket->SendElement<ut::int32>(integer);
				if (send_result_0)
				{
					report += "    client socket sent integer value\n";
				}
				else
				{
					ut::String err_str("    client socket failed to send integer value:\n");
					err_str += send_result_0.GetAlt().GetDesc() + "\n";
					report += err_str;
					failed_test_counter.Increment();
				}

				// recv unsigned int array
				ut::Array<ut::uint32> arr;
				ut::Result<int, ut::Error> recv_result_0 = socket->RecvArray<ut::uint32>(arr);
				if (recv_result_0)
				{
					ut::String str("    client socket received the array of unsigned integers: ");
					for (size_t i = 0; i < arr.GetNum(); i++)
					{
						ut::String int_str;
						int_str.Print("%u", arr[i]);
						str += int_str;
						if (i != arr.GetNum() - 1)
						{
							str += ", ";
						}
					}
					report += str;
				}
				else
				{
					ut::String err_str("    client socket failed to receive the array of unsigned integers:\n");
					err_str += recv_result_0.GetAlt().GetDesc() + "\n";
					report += err_str;
					failed_test_counter.Increment();
				}

				// break the loop
				connected = true;
				break;
			}
			else
			{
				ut::String err_str;
				err_str.Print("    client connect iteration %u failed\n", iteration);
				report += err_str;
				ut::Sleep(250);
			}
		}

		if (!connected)
		{
			ut::String err_str;
			err_str.Print("    client connection failed\n");
			report += err_str;
			failed_test_counter.Increment();
		}
	}
	catch(const ut::Error& error)
	{
		ut::String err_str("\nException handled:\n");
		err_str += error.GetDesc() + "\nThread exited.\n";
		report += err_str;
		failed_test_counter.Increment();
	}

	owner.AddReport(report);
}


//----------------------------------------------------------------------------//
// Server job
ServerTestJob::ServerTestJob(ClientServerCommTask& in_owner) : owner(in_owner)
{ }

void ServerTestJob::Execute()
{
	ut::String report;
	try
	{
		socket = new ut::net::Socket("127.0.0.1", 50000);

		// bind local address
		ut::Optional<ut::Error> bind_error = socket->Bind();
		if (bind_error)
		{
			ut::String err_str("    server socket bind failed:\n");
			err_str += bind_error.Get().GetDesc() + "\n";
			report += err_str;
			failed_test_counter.Increment();
		}
		else
		{
			report += "    server socket binded successfully\n";
		}

		// start listening clients
		ut::Optional<ut::Error> listen_error = socket->Listen();
		if (listen_error)
		{
			ut::String err_str("    server socket listening failed:\n");
			err_str += listen_error.Get().GetDesc() + "\n";
			report += err_str;
			failed_test_counter.Increment();
		}
		else
		{
			report += "    server socket started listening\n";
		}

		for (ut::uint32 iteration = 0; iteration < 5; iteration++)
		{
			ut::Optional<ut::Error> poll_error = socket->Poll(500);
			if (poll_error)
			{
				ut::String err_str;
				err_str.Print("    server poll iteration %u failed\n", iteration);
				report += err_str;
			}
			else
			{
				ut::Result<ut::UniquePtr<ut::net::Socket>, ut::Error> accept_result = socket->Accept();
				if (accept_result)
				{
					ut::UniquePtr<ut::net::Socket> client_socket(accept_result.MoveResult());
					report += "    server socket accepted client\n";

					// recv int value
					ut::int32 integer;
					ut::Result<int, ut::Error> recv_result_0 = client_socket->RecvElement<ut::int32>(integer);
					if (recv_result_0)
					{
						ut::String str;
						str.Print("    server socket received integer value: %i\n", integer);
						report += str;
					}
					else
					{
						ut::String err_str("    server socket failed to receive integer value:\n");
						err_str += recv_result_0.GetAlt().GetDesc() + "\n";
						report += err_str;
						failed_test_counter.Increment();
					}

					// send uint array
					ut::Array<ut::uint32> arr;
					arr.Add(0);
					arr.Add(1);
					arr.Add(2);
					arr.Add(3);
					ut::Result<int, ut::Error> send_result_0 = client_socket->SendArray<ut::uint32>(arr);
					if (send_result_0)
					{
						report += "    server socket sent the array of unsigned integers";
					}
					else
					{
						ut::String err_str("    server socket failed to send the array of unsigned integers:\n");
						err_str += send_result_0.GetAlt().GetDesc() + "\n";
						report += err_str;
						failed_test_counter.Increment();
					}

					// break the loop
					break;
				}
				else
				{
					ut::String err_str("    server socket failed to accept client:\n");
					err_str += accept_result.GetAlt().GetDesc() + "\n";
					report += err_str;
					failed_test_counter.Increment();
				}
			}
		}
	}
	catch(const ut::Error& error)
	{
		ut::String err_str("\nException handled:\n");
		err_str += error.GetDesc() + "\nThread exited.\n";
		report += err_str;
		failed_test_counter.Increment();
	}

	owner.AddReport(report);
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//