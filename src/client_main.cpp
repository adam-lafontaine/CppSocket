#include "os_socket/os_socket_client.hpp"

#include <cstdio>

#if defined(_WIN32)

#define sprintf sprintf_s

#endif


int main()
{
	int server_port = 58002;
	const char* server_ip_address = "127.0.0.1";

	ClientSocketInfo client{};

	printf("\nClient\n\n");

	if (!os_socket_init())
	{
		printf("socket init failed.\n");
		return -1;
	}

	if (!os_client_open(client, server_ip_address, server_port))
	{
		printf("client open failed.\n");
		return -1;
	}

	if (!os_client_connect(client))
	{
		printf("client connect failed.\n");
		return -1;
	}

	printf("Client connected.\n");

	char message_buffer[50] = "";

	os_socket_receive_buffer(client.client_socket, message_buffer, 50);

	printf("recv: %s\n", message_buffer);

	memset(message_buffer, 0, 50);

	sprintf(message_buffer, "Hello from client");

	os_socket_send_buffer(client.client_socket, message_buffer, strlen(message_buffer));

	auto c = getchar();

	os_socket_close(client.client_socket);
	os_socket_cleanup();
}