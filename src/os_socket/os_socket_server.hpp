#pragma once

#include "os_socket_include.hpp"

#include <string.h>


class ServerSocketInfo
{
public:

	socket_t server_socket = 0;
	socket_t client_socket = 0;

	struct sockaddr_in server_addr = { 0 };
	struct sockaddr_in client_addr = { 0 };

	socklen_t client_len;

	bool open = false;
	bool bind = false;
	bool listen = false;

	bool server_running = false;
	bool client_connected = false;

	char ip_address[20];
	int port;
};


static inline bool os_server_open(ServerSocketInfo& server_info, int port)
{
	server_info.open = os_socket_open(server_info.server_socket);

	if (server_info.open)
	{
		server_info.server_addr = { 0 };
		server_info.server_addr.sin_family = AF_INET;
		server_info.server_addr.sin_addr.s_addr = INADDR_ANY;
		server_info.server_addr.sin_port = htons(port);

		server_info.port = port;
	}	

	return server_info.open;
}


static inline bool os_server_bind(ServerSocketInfo& server_info)
{
	auto socket = server_info.server_socket;
	auto addr = (addr_t*)&server_info.server_addr;
	int size = sizeof(server_info.server_addr);

	server_info.bind = bind(socket, addr, size) != SOCKET_ERROR;

	return server_info.bind;
}


static inline bool os_server_listen(ServerSocketInfo& server_info)
{
	auto socket = server_info.server_socket;
	int backlog = 1;

	server_info.listen = listen(socket, backlog) != SOCKET_ERROR;

	return server_info.listen;
}


static inline bool os_server_accept(ServerSocketInfo& server_info)
{
	server_info.client_len = sizeof(server_info.client_addr);

	server_info.client_connected = false;

	auto srv_socket = server_info.server_socket;
	auto cli_addr = (addr_t*)&server_info.client_addr;
	auto cli_len = &server_info.client_len;

	server_info.client_socket = accept(srv_socket, cli_addr, cli_len);

	server_info.client_connected = server_info.client_socket != INVALID_SOCKET;

	return server_info.client_connected;
}


static inline bool os_server_select_ip_address(ServerSocketInfo& server_info, const char* ip)
{
	auto ip_found = os_host_has_ip(ip);

	if (ip_found)
	{
		memcpy(server_info.ip_address, ip, strlen(ip) + 1);
		server_info.server_addr.sin_addr.s_addr = inet_addr(ip);
		//int inet_pton(int af, const char *restrict src, void *restrict dst);
	}

	return ip_found;
}
