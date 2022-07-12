#pragma once

#include "os_socket_include.hpp"

#include <string.h>


class ServerSocketInfo
{
public:

	socket_t server_socket = NULL;
	socket_t client_socket = NULL;

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


static inline bool os_set_ip_address(ServerSocketInfo& server_info, const char* ip)
{
	bool found = false;

#if defined(_WIN32)

	char host_name[255];
	PHOSTENT host_info;

	if (gethostname(host_name, sizeof(host_name)) != 0 || (host_info = gethostbyname(host_name)) == NULL)
	{
		return false;
	}

	int count = 0;
	while (host_info->h_addr_list[count])
	{
		auto item = inet_ntoa(*(struct in_addr*)host_info->h_addr_list[count]);
		if (strcmp(ip, item) == 0)
		{
			found = true;
			break;
		}
		++count;
	}

#else

	//https://www.binarytides.com/get-local-ip-c-linux/

	FILE* f;
	char line[100];
	char* p = NULL;
	char* c = NULL;

	f = fopen("/proc/net/route", "r");

	while (fgets(line, 100, f))
	{
		p = strtok(line, " \t");
		c = strtok(NULL, " \t");

		if (p != NULL && c != NULL)
		{
			if (strcmp(c, "00000000") == 0)
			{
				m_net_interface = std::string(p);
				break;
			}
		}
	}

	//which family do we require , AF_INET or AF_INET6
	int fm = AF_INET; //AF_INET6
	struct ifaddrs* ifaddr, * ifa;
	int family, s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1)
	{
		return false;
	}

	//Walk through linked list, maintaining head pointer so we can free list later
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (strcmp(ifa->ifa_name, p) != 0)
			continue;

		if (family != fm)
			continue;

		auto family_size = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

		s = getnameinfo(ifa->ifa_addr, family_size, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		if (s != 0)
		{
			return false;
		}

		if (strcmp(ip, host) == 0)
		{
			found = true;
			break;
		}
	}

	freeifaddrs(ifaddr);

#endif

	if (found)
	{
		memcpy(server_info.ip_address, ip, strlen(ip) + 1);
	}

	return found;
}


static inline bool os_find_public_ip(ServerSocketInfo& server_info)
{
	char* ip = nullptr;
	bool found = false;

#if defined(_WIN32)

	char host_name[255];
	PHOSTENT host_info;

	if (gethostname(host_name, sizeof(host_name)) != 0 || (host_info = gethostbyname(host_name)) == NULL)
	{
		return false;
	}

	// TODO: gets last ip in the list?
	int count = 0;	
	while (host_info->h_addr_list[count])
	{
		ip = inet_ntoa(*(struct in_addr*)host_info->h_addr_list[count]);
		found = true;
		++count;
	}		

#else

	//https://www.binarytides.com/get-local-ip-c-linux/

	FILE* f;
	char line[100];
	char* p = NULL;
	char* c = NULL;

	f = fopen("/proc/net/route", "r");

	while (fgets(line, 100, f))
	{
		p = strtok(line, " \t");
		c = strtok(NULL, " \t");

		if (p != NULL && c != NULL)
		{
			if (strcmp(c, "00000000") == 0)
			{
				m_net_interface = std::string(p);
				break;
			}
		}
	}

	//which family do we require , AF_INET or AF_INET6
	int fm = AF_INET; //AF_INET6
	struct ifaddrs* ifaddr, * ifa;
	int family, s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1)
	{
		return false;
	}

	//Walk through linked list, maintaining head pointer so we can free list later
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (strcmp(ifa->ifa_name, p) != 0)
			continue;

		if (family != fm)
			continue;

		auto family_size = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

		s = getnameinfo(ifa->ifa_addr, family_size, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		if (s != 0)
		{
			return false;
		}

		found = true;
		ip = host;
	}

	freeifaddrs(ifaddr);

#endif

	if (found && ip)
	{
		memcpy(server_info.ip_address, ip, strlen(ip) + 1);
	}

	return found && ip;
}