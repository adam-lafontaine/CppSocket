#pragma once

#if defined(_WIN32)

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

using socket_t = SOCKET;
using addr_t = SOCKADDR;
using socklen_t = int;

#else

#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>

using socket_t = int;
using addr_t = struct sockaddr;

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif


static inline bool os_socket_init()
{	
#if defined(_WIN32)

	WSAData wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);

	return result == 0;

#else

	return true;

#endif
}


bool os_socket_open(socket_t& socket_handle)
{
	socket_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	return socket_handle != INVALID_SOCKET;
}


static inline bool os_socket_receive_buffer(socket_t socket, char* dst, int n_bytes)
{
	return recv(socket, dst, n_bytes, 0) != SOCKET_ERROR;
}


static inline bool os_socket_send_buffer(socket_t socket, const char* src, int n_bytes)
{
	return send(socket, src, n_bytes, 0) != SOCKET_ERROR;
}


static inline void os_socket_close(socket_t socket)
{
#if defined(_WIN32)

	closesocket(socket);

#else

	close(socket);

#endif
}


static inline void os_socket_cleanup()
{
#if defined(_WIN32)

	WSACleanup();

#else

	// Do nothing
	// Linux has no socket cleanup

#endif
}


static inline bool os_host_has_ip(const char* ip)
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

	return found;
}
