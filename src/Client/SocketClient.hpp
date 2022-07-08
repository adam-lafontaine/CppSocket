#if defined(_WIN32)
#pragma once
#endif

#include <string>
#include <vector>

namespace SocketLib
{
    struct ClientSocketInfo;

    class SocketClient
	{
    private:
		using socket_info_t = ClientSocketInfo;

		socket_info_t* m_socket_info = nullptr;

        static int constexpr MAX_CHARS = 256;

		unsigned short const DEFAULT_PORT = 27015;        
		const char* DEFAULT_IP_ADDRESS = "127.0.0.1";

		const char* m_srv_ip = DEFAULT_IP_ADDRESS;
        unsigned short m_srv_port_no = DEFAULT_PORT;

		std::string m_status = "";

		std::vector<std::string> m_errors;

        bool init();
		bool connect_socket();		
		void close_socket();

		void create_socket_info();
		void destroy_socket_info();

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

		~SocketClient()
		{ 
			close_socket();
			destroy_socket_info();
		}

		void start();
		void stop();
		bool send_text(std::string const& text);
		std::string receive_text();
		std::string status() { return m_status; }
		
		
		bool running();
		bool connected();

		bool has_error() { return !m_errors.empty(); }
        std::string latest_error();
    };
}