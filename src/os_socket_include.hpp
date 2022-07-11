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

#endif


inline bool os_socket_init()
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

#if defined(_WIN32)

	return socket_handle != INVALID_SOCKET;

#else

	return socket_handle >= 0;

#endif
}


inline int os_socket_receive_buffer(socket_t socket, char* dst, int n_bytes)
{
#if defined(_WIN32)

	return recv(socket, dst, n_bytes, 0);

#else

	return read(socket, dst, n_bytes - 1);

#endif
}


inline int os_socket_send_buffer(socket_t socket, const char* src, int n_bytes)
{
#if defined(_WIN32)

	return send(socket, src, n_bytes, 0);

#else

	return write(socket, src, n_bytes);

#endif
}


inline void os_socket_close(socket_t socket)
{
#if defined(_WIN32)

	closesocket(socket);

#else

	close(socket);

#endif
}


inline void os_socket_cleanup()
{
#if defined(_WIN32)

	WSACleanup();

#else

	// Do nothing
	// Linux has no socket cleanup

#endif
}
