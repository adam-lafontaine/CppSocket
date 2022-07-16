#include "../Server/SocketServer.hpp"
#include "../Client/SocketClient.hpp"
#include "win32_leak_check.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>

#if defined(_WIN32)

#include "win32_leak_check.h"

#define sprintf sprintf_s

#endif



constexpr int PORT_NUMBER = 57884;
constexpr auto IP_ADDRESS = "127.0.0.1";
constexpr auto STOP_FLAG = "KILL";
constexpr auto CLIENT_DISCONNECT_FLAG = "END";

std::mutex console_mtx;
std::mutex network_mtx;

void print_line(cstring msg)
{
	std::lock_guard<std::mutex> lk(console_mtx);

	std::cout << msg << '\n';
}


void print_line(cstring msg, cstring concat)
{
	std::lock_guard<std::mutex> lk(console_mtx);

	std::cout << msg << concat << '\n';

}

void read_console(std::string& buffer)
{
	std::lock_guard<std::mutex> lk(console_mtx);

	std::getline(std::cin, buffer);
}


void server_send(SocketServer& server, std::string const& msg)
{
	std::lock_guard<std::mutex> lk(network_mtx);

	server.send_text(msg.data());
}


void client_send(SocketClient& client, cstring msg)
{
	std::lock_guard<std::mutex> lk(network_mtx);

	client.send_text(msg);
}



void process_server_send(bool send_result, cstring server_msg)
{
	if (!send_result)
	{
		print_line("server send: FAIL");
		return;
	}

	print_line("server send: SUCCESS | ", server_msg);
}


void process_server_receive(bool receive_result, cstring client_msg)
{
	if (!receive_result)
	{
		print_line("server recv: FAIL");
		return;
	}

	print_line("server recv: SUCCESS | ", client_msg);	
}


void echo_message(SocketServer& server, cstring msg)
{
	auto echo = std::string("echo: ") + msg;
	
	server_send(server, echo);
}


void check_stop_server(SocketServer& server, cstring msg)
{
	if (strcmp(msg, STOP_FLAG) == 0)
	{
		print_line("stopping server");
		server.disconnect();
		server.close();
	}
}


void process_client_send(bool send_result, cstring client_msg)
{
	if (!send_result)
	{
		print_line("client send: FAIL");
		return;
	}

	print_line("client send: SUCCESS | ", client_msg);
}


void process_client_receive(bool receive_result, cstring server_msg)
{
	if (!receive_result)
	{
		print_line("client recv: FAIL\n");
		return;
	}

	print_line("client recv: SUCCESS | ", server_msg);
}


void check_stop_client(SocketClient& client, cstring msg)
{
	if (strcmp(msg, STOP_FLAG) == 0)
	{
		print_line("stopping client");

	}
}


void run_server()
{
	SocketServer server;
	server.on_send = process_server_send;
	server.on_receive = [&](bool r, cstring m) 
	{		
		process_server_receive(r, m);
		if (!r)
		{
			print_line("server disconnecting");
			server.disconnect();
			return;
		}
		echo_message(server, m);
		check_stop_server(server, m);
	};

	if (!server.open(PORT_NUMBER))
	{
		print_line("Error server open");
		return;
	}

	if (!server.start())
	{
		print_line("Error server start");
		return;
	}

	while (server.is_running())
	{
		print_line("Server waiting for connection");
		server.connect();

		while (server.is_connected())
		{
			server.receive_text();
		}
	}

	print_line("Server done");
}


void run_client()
{
	bool disconnect = false;
	bool stop = false;

	SocketClient client;
	client.on_receive = process_client_receive;
	client.on_send = [&](bool r, cstring m) 
	{
		process_client_send(r, m);

		if (strcmp(m, CLIENT_DISCONNECT_FLAG) == 0)
		{
			print_line("client disconnecting");
			client.disconnect();
		}
		else if (strcmp(m, STOP_FLAG) == 0)
		{
			print_line("stopping client");
			client.disconnect();
			stop = true;
		}
	};

	if (!client.open(IP_ADDRESS, PORT_NUMBER))
	{
		print_line("Error client open");
		return;
	}

	std::string message_buffer;

	while (!stop)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		print_line("Client waiting for server");
		while (!client.connect())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		disconnect = false;

		print_line("Client connected");
		while (client.is_connected())
		{
			read_console(message_buffer);
			client_send(client, message_buffer.data());

			if (client.is_connected())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				client.receive_text();
			}
		}

		if (!client.open(IP_ADDRESS, PORT_NUMBER))
		{
			print_line("Error client open");
			stop = true;
		}
	}	

	print_line("Client done\n");
}


int main()
{
#if defined(_WIN32) && defined(_DEBUG)
	int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;   // check block integrity
	dbgFlags |= _CRTDBG_DELAY_FREE_MEM_DF; // don't recycle memory
	dbgFlags |= _CRTDBG_LEAK_CHECK_DF;     // leak report on exit
	_CrtSetDbgFlag(dbgFlags);
#endif

	std::thread t_server(run_server);
	std::thread t_client(run_client);

	t_server.join();
	t_client.join();
}