#include "../Server/SocketServer.hpp"
#include "../Client/SocketClient.hpp"

#include <cstdio>
#include <thread>
#include <mutex>
#include <chrono>


#if defined(_WIN32)

#define sprintf sprintf_s
#define scanf scanf_s

#endif


constexpr int PORT_NUMBER = 57884;
constexpr auto IP_ADDRESS = "127.0.0.1";
constexpr auto STOP_FLAG = "KILL";
constexpr auto CLIENT_DISCONNECT_FLAG = "END";

std::mutex console_mtx;

void print(cstring fmt, ...)
{
	std::lock_guard<std::mutex> lk(console_mtx);

	printf(fmt);
}


void read_console(cstring buffer)
{
	std::lock_guard<std::mutex> lk(console_mtx);

	scanf("%s", buffer);
}


void process_server_send(bool send_result, cstring server_msg)
{
	auto send = send_result ? "SUCCESS" : "FAIL";
	print("server send: %s | %s\n", send, server_msg);
}


void process_server_receive(bool receive_result, cstring client_msg)
{
	if (!receive_result)
	{
		print("server recv: FAIL\n");
		return;
	}

	print("server recv: SUCCESS | %s\n", client_msg);	
}


void echo_message(SocketServer& server, cstring msg)
{
	char buffer[50];
	sprintf(buffer, "echo: %s\n", msg);
	server.send_text(buffer);
}


void check_stop_server(SocketServer& server, cstring msg)
{
	if (strcmp(msg, STOP_FLAG) == 0)
	{
		print("stopping server\n");
		server.disconnect();
		server.close();
	}
}


void process_client_send(bool send_result, cstring client_msg)
{
	auto send = send_result ? "SUCCESS" : "FAIL";
	print("client send: %s | %s\n", send, client_msg);
}


void process_client_receive(bool receive_result, cstring server_msg)
{
	if (!receive_result)
	{
		print("client recv: FAIL\n");
		return;
	}

	print("client recv: SUCCESS | %s\n", server_msg);
}


void check_disconnect_client(SocketClient& client, cstring msg)
{
	if (strcmp(msg, CLIENT_DISCONNECT_FLAG) == 0)
	{
		client.disconnect();
	}
}


void check_stop_client(SocketClient& client, cstring msg)
{
	if (strcmp(msg, STOP_FLAG) == 0)
	{
		print("stopping client\n");

	}
}


void run_server()
{
	SocketServer server;
	server.on_send = process_server_send;
	server.on_receive = [&](bool r, cstring m) 
	{
		process_server_receive(r, m);
		echo_message(server, m);
		check_stop_server(server, m);
	};

	if (!server.open(PORT_NUMBER))
	{
		print("Error server open\n");
		return;
	}

	if (!server.start())
	{
		print("Error server start\n");
		return;
	}

	while (server.is_running())
	{
		print("Server waiting for connection\n");
		server.connect();

		while (server.is_connected())
		{
			server.receive_text();
		}
	}

	print("Server done\n");
}


void run_client()
{
	bool try_connect = true;

	SocketClient client;
	client.on_send = process_client_send;
	client.on_receive = [&](bool r, cstring m) 
	{
		process_client_receive(r, m);
		check_disconnect_client(client, m);
		if (strcmp(m, STOP_FLAG) == 0)
		{
			try_connect = false;
		}
	};

	if (!client.open(IP_ADDRESS, PORT_NUMBER))
	{
		print("Error client open\n");
		return;
	}

	char buffer[50];

	while (try_connect)
	{
		print("Client waiting for server");
		while (!client.connect())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		print("Client connected");
		while (client.is_connected())
		{
			memset(buffer, 0, 50);
			read_console(buffer);
			client.send_text(buffer);
		}
	}

	print("Client done");
}


int main()
{
	//run_server();

	run_client();
}