#include "SocketServer.hpp"

#if defined(_WIN32)

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#else

#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>

#endif

#include <sstream>
#include <cassert>

static std::string to_csv(std::vector<std::string> const& list)
{
    const auto delim = ", ";

    std::string msg = "";
    for (auto const& err : list)
    {
        msg += err;
        msg += delim;
    }

    msg.pop_back();
    msg.pop_back();

    return msg;
}


namespace SocketLib
{





    
#if defined(_WIN32)

    struct ServerSocketInfo
	{
		struct sockaddr_in srv_addr = { 0 };

		SOCKET srv_socket = NULL;
		SOCKET cli_socket = NULL;

		bool srv_running = false;
		bool cli_connected = false;		
	};


    void SocketServer::destroy_socket_info()
	{
		if (m_socket_info == nullptr)
			return;

		if (m_socket_info->cli_connected)
		{
			closesocket(m_socket_info->cli_socket);
			m_socket_info->cli_connected = false;
		}

		if (m_socket_info->srv_running)
		{
			closesocket(m_socket_info->srv_socket);
			WSACleanup();
			m_socket_info->srv_running = false;
		}

		delete m_socket_info;
		m_socket_info = nullptr;
	}


    bool SocketServer::init()
	{
		// initialize WSA
		WSAData wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			m_status = "Server WSAStartup failed : " + iResult;
			m_errors.push_back(m_status);
			return false;
		}

		create_socket_info();

		m_status = "Server Initialized";
		return true;
	}


    bool SocketServer::bind_socket()
	{
		auto socket = m_socket_info->srv_socket;
		auto addr = (SOCKADDR*)&m_socket_info->srv_addr;
		auto size = sizeof(m_socket_info->srv_addr);

		if (bind(socket, addr, size) == SOCKET_ERROR)
		{
			close_socket();
			m_status = "Server bind() failed.";
			m_errors.push_back(m_status);
			return false;
		}

		return true;
	}


    bool SocketServer::listen_socket()
	{
		m_socket_info->srv_running = false;
		const auto port = std::to_string(m_port_no);
		if (listen(m_socket_info->srv_socket, 1) == SOCKET_ERROR)
		{
			close_socket();
			m_status = "Server error listening on port " + port;
			m_errors.push_back(m_status);
			return false;
		}

		m_status = "Listening on " + m_public_ip + " : " + port;

		return true;
	}


    bool SocketServer::connect_client()
	{
		if (!m_socket_info->srv_running)
		{
			m_status = "Server cannot connect, server not running";
			m_errors.push_back(m_status);
			return false;
		}

		m_status = "Server waiting for client";

		m_socket_info->cli_connected = false;		

		auto srv_socket = m_socket_info->srv_socket;
		auto cli_addr = NULL; // (SOCKADDR*)&m_socket_info->cli_addr;
		auto cli_len = NULL;  // &m_socket_info->cli_len;

		m_socket_info->cli_socket = SOCKET_ERROR;

		while (m_socket_info->cli_socket == SOCKET_ERROR)
		{
			m_socket_info->cli_socket = accept(srv_socket, /*cli_addr, cli_len*/ NULL, NULL);
		}

		m_status = "Server connected to client";
		m_socket_info->cli_connected = true;

		return true;
	}


    void SocketServer::disconnect_client()
	{
		if (!connected())
			return;

		closesocket(m_socket_info->cli_socket);

		m_status = "Server disonnected from client";

		m_socket_info->cli_connected = false;
	}


    void SocketServer::close_socket()
	{
		if (!running())
			return;

		closesocket(m_socket_info->srv_socket);
		WSACleanup();
		m_socket_info->srv_running = false;
	}


    std::string SocketServer::receive_text()
	{
		assert(running());
		assert(connected());

		char buffer[MAX_CHARS] = "";

		int n_chars = recv(m_socket_info->cli_socket, buffer, MAX_CHARS, 0);
		
		if (n_chars < 0)
		{
			m_status = "ERROR reading from client socket";
			m_errors.push_back(m_status);
			return "error";
		}

		std::ostringstream oss;
		oss << buffer;

		return oss.str();
	}


    bool SocketServer::send_text(std::string const& text)
	{
		assert(running());
		assert(connected());

		if (!running() || !connected())
			return false;

		auto cli_socket = m_socket_info->cli_socket;
		auto data = text.data();
		auto size = static_cast<int>(text.size());

		int n_chars = send(cli_socket, data, size, 0);

		if (n_chars < 0)
		{
			m_status = "ERROR writing to client socket";
			m_errors.push_back(m_status);
			return false;
		}

		return true;
	}


    void SocketServer::get_network_info()
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		char Name[255];
		PHOSTENT HostInfo;
		wVersionRequested = MAKEWORD(1, 1);

		if (WSAStartup(wVersionRequested, &wsaData) == 0)
		{
			if (gethostname(Name, sizeof(Name)) == 0)
			{
				//printf("Host name: %s\n", name);
				if ((HostInfo = gethostbyname(Name)) != NULL)
				{
					int nCount = 0;
					while (HostInfo->h_addr_list[nCount])
					{
						m_public_ip = std::string(inet_ntoa(*(struct in_addr*)HostInfo->h_addr_list[nCount]));

						++nCount;
					}
				}
			}
		}
	}


#else

    struct ServerSocketInfo
    {
        struct sockaddr_in srv_addr = { 0 };
        struct sockaddr_in cli_addr = { 0 };

        int srv_socket;
        int cli_socket;
        
        socklen_t cli_len;

        bool srv_running = false;
		bool cli_connected = false;
    };


    void SocketServer::destroy_socket_info()
    {
        if (m_socket_info == nullptr)
			return;

        if (m_socket_info->cli_connected)
		{
			close(m_socket_info->cli_socket);
			m_socket_info->cli_connected = false;
		}

		if (m_socket_info->srv_running)
		{
			close(m_socket_info->srv_socket);
			m_socket_info->srv_running = false;
		}

		delete m_socket_info;
		m_socket_info = nullptr;
    }


    bool SocketServer::init() 
    {
        create_socket_info();

        m_status = "Server Initialized";
        return true;
    }


    bool SocketServer::bind_socket()
    {
        auto socket = m_socket_info->srv_socket;
        auto addr = (struct sockaddr*)&m_socket_info->srv_addr;
        auto size = sizeof(m_socket_info->srv_addr);

        if (bind(socket, addr, size) < 0)
        {
            m_status = "ERROR binding socket";
            m_errors.push_back(m_status);
            return false;
        }

        m_status = "Socket binded";

        return true;   
    }


    bool SocketServer::listen_socket()
    {
        m_socket_info->srv_running = false;
        const auto port = std::to_string(m_port_no);
        
        if(listen(m_socket_info->srv_socket, 5) < 0)
        {
            m_status = "ERROR listening on port " + port;
            m_errors.push_back(m_status);
            return false;
        }

        m_status =  "Listening on " + m_public_ip + " : " + port;
        return true;
    }


    bool SocketServer::connect_client()
    {
        if (!m_socket_info->srv_running)
		{
			m_status = "Server cannot connect, server not running";
			m_errors.push_back(m_status);
			return false;
		}

        m_socket_info->cli_len = sizeof(m_socket_info->cli_addr);

        m_status = "Server waiting for client";
		m_socket_info->cli_connected = false;

        auto srv_socket = m_socket_info->srv_socket;
        auto cli_addr = (struct sockaddr *)&m_socket_info->cli_addr;
        auto cli_len = &m_socket_info->cli_len;

        // waits for client to connect        
        m_socket_info->cli_socket = accept(srv_socket, cli_addr, cli_len);

        if (m_socket_info->cli_socket < 0)
        {
            m_status = "ERROR on accept";
            m_errors.push_back(m_status);
            return false;
        }

        m_status = "Server connected to client";
		m_socket_info->cli_connected = true;

		return true;
    }


    void SocketServer::disconnect_client()
    {
		if (!connected())
			return;
        
        close(m_socket_info->cli_socket);

        m_status = "Server disonnected from client";

        m_socket_info->cli_connected = false;
	}


    void SocketServer::close_socket()
    {
        if(!running())
            return;

        close(m_socket_info->srv_socket);
        m_socket_info->srv_running = false;
    }


    std::string SocketServer::receive_text()
    {
		assert(running());
		assert(connected());

		char buffer[MAX_CHARS] = "";
        
        int n_chars = read(m_socket_info->cli_socket, buffer, MAX_CHARS - 1);

        if (n_chars < 0)
        {
            m_status = "ERROR reading from client socket";
            m_errors.push_back(m_status);
            return "error";
        }

        std::ostringstream oss;        
        oss << buffer;

		return oss.str();
	}


    bool SocketServer::send_text(std::string const& text)
    {
		assert(running());
		assert(connected());
        
		if (!running() || !connected())
			return false;
        
        auto cli_socket = m_socket_info->cli_socket;
        auto data = text.data();
        auto size = static_cast<int>(text.size());

        int n_chars = write(cli_socket, data, size);

        if (n_chars < 0)
        {
            m_status = "ERROR writing to client socket";
            m_errors.push_back(m_status);
            return false;
        }

        return true;
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


#endif





    static void create_server_socket(ServerSocketInfo* info, unsigned short port)
    {
        // Create the TCP socket
        info->srv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        // Create the server address
        info->srv_addr = { 0 };
		info->srv_addr.sin_family = AF_INET;
        info->srv_addr.sin_addr.s_addr = INADDR_ANY;
        info->srv_addr.sin_port = htons(port);
    }


    void SocketServer::create_socket_info()
    {
        if(m_socket_info == nullptr)
			m_socket_info = new socket_info_t;

		create_server_socket(m_socket_info, m_port_no);
    }


    bool SocketServer::running()
	{
		return m_socket_info != nullptr && m_socket_info->srv_running;
	}


	bool SocketServer::connected()
	{
		return m_socket_info != nullptr && m_socket_info->cli_connected;
	}


    void SocketServer::start()
    {
        if(running())
			m_socket_info->srv_running = false;

		if (!init() || !bind_socket() || !listen_socket())
        {
            close_socket();
            return;
        }

		m_socket_info->srv_running = true;
    }


    void SocketServer::stop()
    {
		disconnect_client();
        
		close_socket();
		m_status = "Server stopped";
	}


	std::string SocketServer::latest_error()
	{
		const auto msg = to_csv(m_errors);

		m_errors.clear();

		return msg;
	}


    








}