#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#include <string>

namespace MySocketLib
{
	// Class encapsulating WinSock server functionality
	class SocketServer {
	private:
		unsigned short const PORT = 27015;
		static int constexpr MAX_CHARS = 256;

		const char* m_ip_address = "127.0.0.1";

		SOCKET m_hSocket = NULL;
		SOCKET m_hAccepted = NULL;
		sockaddr_in m_serverAddress = { 0 };

		bool m_running = false;
		bool m_open = false;
		bool m_connected = false;
		std::string m_status = "";

		bool init();
		bool bind_socket();
		bool listen_socket();
		void close();

	public:
		SocketServer() {}
		SocketServer(const char* ip) : m_ip_address(ip) {}

		~SocketServer()
		{
			disconnect_client();
			close();
		}

		void start();
		void stop();
		bool connect_client();
		void disconnect_client();

		std::string receive_text();
		void send_text(std::string const& text);

		std::string status() { return m_status; }
		bool running() { return m_running; }
		bool connected() { return m_connected; }
	};

}


