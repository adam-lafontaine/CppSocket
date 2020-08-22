#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#include <string>
#include <vector>


namespace MySocketLib
{
	typedef struct ClientSocketInfo client_socket_info_t;

	class SocketClient 
	{
	private:
		static int constexpr MAX_CHARS = 256;
		unsigned short const DEFAULT_PORT = 27015;		
		const char* DEFAULT_IP_ADDRESS = "127.0.0.1";

		const char* m_srv_ip = DEFAULT_IP_ADDRESS;
		unsigned short m_srv_port_no = DEFAULT_PORT;

		client_socket_info_t* m_socket_info;

		SOCKET m_socket = NULL;
		sockaddr_in m_srv_addr = { 0 };

		bool m_running = false;
		bool m_open = false;
		std::string m_status = "";

		std::vector<std::string> m_errors;

		bool init();
		bool connect_socket();
		void close_socket();

	public:
		SocketClient() {}

		SocketClient(const char* ip, unsigned short srv_port)
		{
			m_srv_ip = ip;
			m_srv_port_no = srv_port;
		}

		SocketClient(std::string const& ip, unsigned short srv_port)
		{
			m_srv_ip = ip.c_str();
			m_srv_port_no = srv_port;
		}

		~SocketClient() { close_socket(); }

		void start();
		void stop();
		bool send_text(std::string const& text);
		std::string receive_text();
		std::string status() { return m_status; }
		bool running() { return m_running; }

		bool has_error() { return !m_errors.empty(); }
		std::string latest_error();
	};




}

