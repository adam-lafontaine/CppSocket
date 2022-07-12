//define DO_SERVER
#define DO_CLIENT

#include <cstdio>

#ifdef DO_SERVER

#include "os_socket_server.hpp"

void run_server()
{
	int port = 58002;
	const char* ip_address = "192.168.137.1";

	ServerSocketInfo server{};

	printf("\nServer\n\n");

	if (!os_socket_init())
	{
		printf("socket init failed.\n");
		return;
	}

	if (!os_server_open(server, port))
	{
		printf("server open failed.\n");
		return;
	}	

	if (!os_server_bind(server))
	{
		printf("server bind failed.\n");
		return;
	}

	if (!os_server_listen(server))
	{
		printf("server listen failed.\n");
		return;
	}

	printf("Waiting for client to connect on port %d\n", server.port);
	if (!os_server_accept(server))
	{
		printf("client connect failed.\n");
		return;
	}

	printf("Client connected\n");

	char message_buffer[50] = "hello from server";

	os_socket_send_buffer(server.client_socket, message_buffer, strlen(message_buffer));

	memset(message_buffer, 0, 50);

	os_socket_receive_buffer(server.client_socket, message_buffer, 50);

	printf("recv: %s\n", message_buffer);

	os_socket_receive_buffer(server.client_socket, message_buffer, 50);

	auto c = getchar();

	os_socket_close(server.client_socket);
	os_socket_close(server.server_socket);
	os_socket_cleanup();
}

#endif // DO_SERVER


#ifdef DO_CLIENT

#include "os_socket_client.hpp"

void run_client()
{
	int server_port = 58002;
	const char* server_ip_address = "192.168.137.1";

	ClientSocketInfo client{};

	printf("\nClient\n\n");

	if (!os_socket_init())
	{
		printf("socket init failed.\n");
		return;
	}

	if (!os_client_open(client, server_ip_address, server_port))
	{
		printf("client open failed.\n");
		return;
	}

	if (!os_client_connect(client))
	{
		printf("client connect failed.\n");
		return;
	}

	printf("Client connected.\n");

	char message_buffer[50] = "";

	os_socket_receive_buffer(client.client_socket, message_buffer, 50);

	printf("recv: %s\n", message_buffer);

	memset(message_buffer, 0, 50);

	sprintf_s(message_buffer, "Hello from client");

	os_socket_send_buffer(client.client_socket, message_buffer, strlen(message_buffer));

	auto c = getchar();

	os_socket_close(client.client_socket);
	os_socket_cleanup();
}

#endif // DO_CLIENT

int main()
{
#ifdef DO_SERVER

	run_server();

#endif

#ifdef DO_CLIENT

	run_client();

#endif


}