#include <cstdio>

#include "os_socket/os_socket_server.hpp"



int main()
{
int port = 58002;
	const char* ip_address = "10.0.0.143";

	ServerSocketInfo server{};

	printf("\nServer\n\n");

	if (!os_socket_init())
	{
		printf("socket init failed.\n");
		return -1;
	}

	if (!os_server_open(server, port))
	{
		printf("server open failed.\n");
		return -1;
	}	

	if (!os_server_bind(server))
	{
		printf("server bind failed.\n");
		return -1;
	}

	if (!os_server_listen(server))
	{
		printf("server listen failed.\n");
		return -1;
	}

	printf("Waiting for client to connect on port %d\n", server.port);
	if (!os_server_accept(server))
	{
		printf("client connect failed.\n");
		return -1;
	}

	printf("Client connected\n");

	char message_buffer[50] = "hello from server";

	os_socket_send_buffer(server.client_socket, message_buffer, strlen(message_buffer));

	memset(message_buffer, 0, 50);

	os_socket_receive_buffer(server.client_socket, message_buffer, 50);

	printf("recv: %s\n", message_buffer);

	auto c = getchar();

	os_socket_close(server.client_socket);
	os_socket_close(server.server_socket);
	os_socket_cleanup();
}