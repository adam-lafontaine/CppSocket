#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<arpa/inet.h>

#include<ifaddrs.h>

#include<vector>
#include<sstream>
#include<cassert>

namespace SocketLib 
{ 
    class SocketServer 
    {
    private:        
        static int constexpr MAX_CHARS = 256;
        unsigned short const DEFAULT_PORT = 27015;

        std::string m_net_interface = "NA";

        std::string m_public_ip = "NA";
        unsigned short m_port_no = DEFAULT_PORT;

        int m_srv_socket; // server socket file descriptor
        int m_cli_socket; // client socket file descriptor

        struct sockaddr_in m_srv_addr = { 0 }; // contains server address
        struct sockaddr_in m_cli_addr = { 0 };  // contains client address
        socklen_t m_cli_len; // size of address on client
        
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

        // TODO: move to helpers
		std::string system_error(std::string const& msg);
		std::string to_csv(std::vector<std::string> const& list);
        
    public:
        SocketServer()
        { 
            m_port_no = DEFAULT_PORT;
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
        bool send_text(std::string const& text);        

        std::string status() { return m_status; }
        bool running() { return m_running; }
        bool connected() { return m_connected; }

        bool has_error() { return !m_errors.empty(); }
        std::string latest_error();
    };

    //====================================================    

    
}