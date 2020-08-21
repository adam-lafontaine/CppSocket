#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#include <string>
#include <vector>

namespace MySocketLib
{
	// Class encapsulating WinSock server functionality
	class SocketServer {
	private:
		static int constexpr MAX_CHARS = 256;
		unsigned short const DEFAULT_PORT = 27015;		
		const char* DEFAULT_IP_ADDRESS = "10.0.0.37";

		const char* m_ip_address = DEFAULT_IP_ADDRESS;
		unsigned short m_port_no = DEFAULT_PORT;

		std::string m_public_ip = "NA";

		SOCKET m_srv_socket = NULL;
		SOCKET m_cli_socket = NULL;
		sockaddr_in m_serverAddress = { 0 };

		bool m_running = false;
		bool m_open = false;
		bool m_connected = false;
		std::string m_status = "";

		std::vector<std::string> m_errors;

		void get_network_info();

		bool init();
		bool bind_socket();
		bool listen_socket();
		void close_socket();

	public:
		SocketServer() 
		{
			get_network_info();
		}
		SocketServer(unsigned short port)
		{
			m_port_no = port;
			get_network_info();
		}

		~SocketServer()
		{
			disconnect_client();
			close_socket();
		}

		void set_port(unsigned port) { m_port_no = port; }

		void start();
		void stop();
		bool connect_client();
		void disconnect_client();

		std::string receive_text();
		void send_text(std::string const& text);

		std::string status() { return m_status; }
		bool running() { return m_running; }
		bool connected() { return m_connected; }

		bool has_error() { return !m_errors.empty(); }
		std::string latest_error();
	};

}


