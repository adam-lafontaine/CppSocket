#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#include <string>


namespace MySocketLib
{
	// Class encapsulating WinSock client functionality
	class SocketClient 
	{
	private:
		unsigned short const PORT = 27015;
		static int constexpr MAX_CHARS = 256;

		const char* m_ip_address = "127.0.0.1";

		SOCKET m_hSocket = NULL;
		sockaddr_in m_serverAddress = { 0 };

		bool m_running = false;
		bool m_open = false;
		std::string m_status = "";

		bool connect_socket();
		bool init();
		void close();

	public:
		SocketClient() {}
		SocketClient(const char* ip): m_ip_address(ip) {}
		~SocketClient() { close(); }

		void start();
		void stop();
		void send_text(std::string const& text);
		std::string receive_text();
		std::string status() { return m_status; }
		bool running() { return m_running; }
	};

}

