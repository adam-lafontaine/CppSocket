#include "../Server/SocketServer.hpp"

#include <cstdio>


#if defined(_WIN32)

#define sprintf sprintf_s

#endif


constexpr int PORT_NUMBER = 57884;
constexpr auto SERVER_END_FLAG = "KILL";
constexpr auto CLIENT_DISCONNECT_FLAG = "END";


void process_server_send(bool send_result, cstring server_msg)
{
	auto send = send_result ? "SUCCESS" : "FAIL";
	printf("server send: %s | %s\n", send, server_msg);
}


void process_server_receive(bool receive_result, cstring client_msg)
{
	if (!receive_result)
	{
		printf("server recv: FAIL\n");
		return;
	}

	printf("server recv: SUCCESS | %s\n", client_msg);

	
}


void echo_message(SocketServer& server, cstring msg)
{
	char buffer[50];
	sprintf(buffer, "echo: %s\n", msg);
	server.send_text(buffer);
}


void check_stop_server(SocketServer& server, cstring msg)
{
	if (strcmp(msg, SERVER_END_FLAG) == 0)
	{
		printf("stopping server\n");
		server.disconnect();
		server.close();
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
		printf("Error server open\n");
		return;
	}

	if (!server.start())
	{
		printf("Error server start\n");
		return;
	}

	while (server.is_running())
	{
		printf("Server waiting for connection\n");
		server.connect();

		while (server.is_connected())
		{
			server.receive_text();
		}
	}
}


int main()
{
	run_server();


}