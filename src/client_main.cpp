#include "os_socket_client.hpp"

#include <cstdio>


int main()
{
	int server_port = 58002;
	const char* server_ip_address = "192.168.137.1";

	ClientSocketInfo client{};

	if (!os_socket_init())
	{
		printf("socket init failed.\n");
		return EXIT_FAILURE;
	}

	if (!os_client_open(client, server_ip_address, server_port))
	{
		printf("client open failed.\n");
		return EXIT_FAILURE;
	}

	if (!os_client_connect(client))
	{
		printf("client connect failed.\n");
		return EXIT_FAILURE;
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