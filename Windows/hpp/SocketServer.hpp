/*
*    Authors: Adam Lafontaine, Yougui Chen
*     Course: INFO 5104
* Assignment: Project 3, Socket Library
*       Date: January 9, 2018
*
*       File: SocketServer.hpp
*/

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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

		char* _ip_address = "127.0.0.1";

		SOCKET _hSocket;
		SOCKET _hAccepted;
		sockaddr_in _serverAddress;

		bool _running = false;
		bool _open = false;
		bool _connected = false;
		string _status = "";

		bool init();
		bool bind_socket();
		bool listen_socket();
		void close();

	public:
		SocketServer() {}
		SocketServer(char* ip) : _ip_address(ip) {}

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

		string status() { return _status; }
		bool running() { return _running; }
		bool connected() { return _connected; }
	};

}


