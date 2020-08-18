/*
*    Authors: Adam Lafontaine, Yougui Chen
*     Course: INFO 5104
* Assignment: Project 3, Socket Library
*       Date: January 9, 2018
*
*       File: SocketServer.hpp
*/

#pragma once
#define m_WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
using namespace std;

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

namespace MySocketLib {

	// Class encapsulating WinSock server functionality
	class SocketServer {
	private:
		unsigned short const PORT = 27015;
		static int constexpr MAX_CHARS = 256;

		const char* m_ip_address = "127.0.0.1";

		SOCKET m_hSocket;
		SOCKET m_hAccepted;
		sockaddr_in m_serverAddress;

		bool m_running = false;
		bool m_open = false;
		bool m_connected = false;
		string m_status = "";

		bool init();
		bool bind_socket();
		bool listen_socket();
		void close();

	public:
		SocketServer() {}
		SocketServer(const char* ip) : m_ip_address(ip) {}

		~SocketServer() {
			disconnect_client();
			close();
		}

		void start();
		void stop();
		bool connect_client();
		void disconnect_client();

		string receive_text();
		void send_text(string const& text);

		string status() { return m_status; }
		bool running() { return m_running; }
		bool connected() { return m_connected; }
	};

}


