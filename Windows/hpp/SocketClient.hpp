/*
*    Authors: Adam Lafontaine, Yougui Chen
*     Course: INFO 5104
* Assignment: Project 3, Socket Library
*       Date: January 9, 2018
*
*       File: SocketClient.hpp
*/

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
using namespace std;

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

namespace MySocketLib {

	// Class encapsulating WinSock client functionality
	class SocketClient {
	private:
		unsigned short const PORT = 27015;
		static int constexpr MAX_CHARS = 256;

		char* _ip_address = "127.0.0.1";

		SOCKET _hSocket;
		sockaddr_in _serverAddress;

		bool _running = false;
		bool _open = false;
		string _status = "";

		bool connect_socket();
		bool init();
		void close();

	public:
		SocketClient() {}
		SocketClient(char* ip): _ip_address(ip) {}
		~SocketClient() {
			close();
		}

		void start();
		void stop();
		void send_text(string const& text);
		string receive_text();
		string status() { return _status; }
		bool running() { return _running; }
	};

}

