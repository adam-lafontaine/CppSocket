#pragma once

#include <string>
#include <vector>

namespace MySocketLib
{	
	struct ServerSocketInfo;

	class SocketServer
	{
	private:

		using socket_info_t = ServerSocketInfo;

		socket_info_t* m_socket_info = nullptr;

		static int constexpr MAX_CHARS = 256;
		unsigned short const DEFAULT_PORT = 27015;	

		std::string m_public_ip = "NA";
		unsigned short m_port_no = DEFAULT_PORT;	

		std::string m_status = "";

		std::vector<std::string> m_errors;

		void get_network_info();

		bool init();
		bool bind_socket();
		bool listen_socket();
		void close_socket();

		void create_socket_info();
		void destroy_socket_info();

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
			destroy_socket_info();
		}

		void set_port(unsigned port) { m_port_no = port; }

		void start();
		void stop();
		bool connect_client();
		void disconnect_client();

		std::string receive_text();
		bool send_text(std::string const& text);

		std::string status() { return m_status; }
		bool running();
		bool connected();

		bool has_error() { return !m_errors.empty(); }
		std::string latest_error();
	};

}


