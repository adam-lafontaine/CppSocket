#pragma once

#include "os_socket_include.hpp"


class ClientSocketInfo
{
public:

	sockaddr_in server_addr = { 0 };
	socket_t client_socket = NULL;

	bool open = false;
	bool connected = false;	
};


static inline bool os_client_open(ClientSocketInfo& client_info, const char* server_ip, int server_port)
{
	client_info.open = os_socket_open(client_info.client_socket);

	if (client_info.open)
	{
		client_info.server_addr = { 0 };
		client_info.server_addr.sin_family = AF_INET;
		client_info.server_addr.sin_addr.s_addr = inet_addr(server_ip);
		client_info.server_addr.sin_port = htons(server_port);
	}

	return client_info.open;
}


static inline bool os_client_connect(ClientSocketInfo& client_info)
{
	auto socket = client_info.client_socket;
	auto addr = (addr_t*)&client_info.server_addr;
	int size = sizeof(client_info.server_addr);

	client_info.connected = connect(socket, addr, size) != SOCKET_ERROR;

	return client_info.connected;
}