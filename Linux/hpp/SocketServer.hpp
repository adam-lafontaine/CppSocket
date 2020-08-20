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
        unsigned short const DEFAULT_PORT = 27015;
        static int constexpr MAX_CHARS = 256;
        
        bool m_running = false;
        bool m_open = false;
        bool m_connected = false;
        std::string m_status = "";

        std::string m_net_interface = "NA";
        std::string m_public_ip = "NA";

        unsigned short m_port_no;
        int m_srv_socket; // server socket file descriptor
        int m_cli_socket; // client socket file descriptor

        struct sockaddr_in m_serv_addr; // contains server address
        struct sockaddr_in m_cli_addr;  // contains client address
        socklen_t m_cli_len; // size of address on client

        void get_network_info();

        bool init();
        bool bind_socket();
        bool listen_socket();
        void close_socket();

        std::vector<std::string> m_errors;

        std::string system_error(std::string const& msg); 
        
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

    bool SocketServer::init() 
    {
        m_srv_socket = socket(AF_INET, SOCK_STREAM, 0); // create socket

        if (m_srv_socket < 0)
        {
            m_status = "ERROR opening socket";
            m_errors.push_back(system_error(m_status));
            return false;
        }

        m_open = true;

        bzero((char *)&m_serv_addr, sizeof(m_serv_addr)); // initialize serv_addr to zeros        

        m_serv_addr.sin_family = AF_INET;          // always
        m_serv_addr.sin_addr.s_addr = INADDR_ANY;  // IP address of server machine
        m_serv_addr.sin_port = htons(m_port_no);    // convert from host to network byte order

        m_status = "Server Initialized";
        return true;
    }

    bool SocketServer::bind_socket()
    {
        // bind socket to server address
        const int bind_res = bind(m_srv_socket, (struct sockaddr*) &m_serv_addr, sizeof(m_serv_addr));

        if (bind_res < 0)
        {
            m_status = "ERROR binding socket";
            m_errors.push_back(system_error(m_status));
            return false;
        }

        m_status = "Socket binded";

        return true;   
    }

    bool SocketServer::listen_socket()
    {
        const int res = listen(m_srv_socket, 5);
        const auto port = std::to_string(m_port_no);
        if(res < 0)
        {
            m_status = "ERROR listening on port " + port;
            m_errors.push_back(system_error(m_status));
            return false;
        }

        m_status =  "Listening on " + m_public_ip + " : " + port;
        return true;
    }

    void SocketServer::close_socket()
    {
        if(!m_open)
            return;

        close(m_srv_socket);
        
        m_open = false;
    }

    void SocketServer::start()
    {
        m_running = false;

		if (!init() || !bind_socket() || !listen_socket())
        {
            close_socket();
            return;
        }

		m_running = true;
    }

    void SocketServer::stop()
    {
		disconnect_client();

		m_running = false;
		close_socket();
		m_status = "Server stopped";
	}

    bool SocketServer::connect_client()
    {
        if (!m_running)
        {
			m_status = "Server cannot connect, server not running";
			return false;
		}

        m_cli_len = sizeof(m_cli_addr);

        m_status = "Server waiting for client";
		m_connected = false;

        // waits for client to connect        
        m_cli_socket = accept(m_srv_socket, (struct sockaddr *) &m_cli_addr, &m_cli_len);

        if (m_cli_socket < 0)
        {
            m_status = "ERROR on accept";
            m_errors.push_back(system_error(m_status));
            return false;
        }

        m_status = "Server connected to client";
		m_connected = true;

		return true;
    }

    void SocketServer::disconnect_client()
    {
		if (!m_connected)
            return;
        
        close(m_cli_socket);

        m_status = "Server disonnected from client";

        m_connected = false;
	}

    std::string SocketServer::receive_text()
    {
		assert(m_running);
		assert(m_connected);

		char buffer[MAX_CHARS]; // characters are read into this buffer

		bzero(buffer, MAX_CHARS); // initialize buffer to zeros

        // waits for message from client to be read to buffer
        int n_chars = read(m_cli_socket, buffer, MAX_CHARS - 1);

        if (n_chars < 0)
        {
            m_status = "ERROR reading from client socket";
            m_errors.push_back(system_error(m_status));
            return "error";
        }

        std::ostringstream oss;        
        oss << buffer;

		return oss.str();
	}

    bool SocketServer::send_text(std::string const& text)
    {
		assert(m_running);
		assert(m_connected);
        
		if (!m_running || !m_connected)
			return false;
        
        // send message to client
        int n_chars = write(m_cli_socket, text.data(), static_cast<int>(text.size()));

        if (n_chars < 0)
        {
            m_status = "ERROR writing to client socket";
            m_errors.push_back(system_error(m_status));
            return false;
        }

        return true;
	}

    std::string SocketServer::latest_error()
    {
        const auto delim = ", ";

        std::string msg = "";
        for(auto const& err : m_errors)
		{
            msg += err;
			msg += delim;
        }

		msg.pop_back();
		msg.pop_back();

        m_errors.clear();

		return msg;
    }

    std::string SocketServer::system_error(std::string const& msg)
	{
		return msg + ": " + strerror(errno);
	}

    void SocketServer::get_network_info()
    {
        //https://www.binarytides.com/get-local-ip-c-linux/

        FILE *f;
        char line[100] , *p , *c;
        
        f = fopen("/proc/net/route" , "r");
        
        while(fgets(line , 100 , f))
        {
            p = strtok(line , " \t");
            c = strtok(NULL , " \t");
            
            if(p!=NULL && c!=NULL)
            {
                if(strcmp(c , "00000000") == 0)
                {
                    m_net_interface = std::string(p);                    
                    break;
                }
            }
        }
        
        //which family do we require , AF_INET or AF_INET6
        int fm = AF_INET; //AF_INET6
        struct ifaddrs *ifaddr, *ifa;
        int family , s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1)
        {
            m_public_ip = "error";
            return;
        }

        //Walk through linked list, maintaining head pointer so we can free list later
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
                continue;

            family = ifa->ifa_addr->sa_family;
            if(strcmp( ifa->ifa_name , p) != 0)
                continue;

            if(family != fm)
                continue;

            auto family_size = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            
            s = getnameinfo( ifa->ifa_addr, family_size , host , NI_MAXHOST , NULL , 0 , NI_NUMERICHOST);
                
            if (s != 0)
            {
                m_public_ip = "error";
                return;
            }
            
            m_public_ip = std::string(host);
        }

        freeifaddrs(ifaddr);
    }
}