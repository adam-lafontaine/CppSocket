#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include<vector>
#include<sstream>
#include<cassert>

namespace SocketLib
{
    class SocketClient
	{
    private:
		unsigned short const DEFAULT_PORT = 27015;
        static int constexpr MAX_CHARS = 256;
        const char* DEFAULT_HOST = "localhost";
		
        const char* m_srv_hostname;

        unsigned short m_srv_port_no;
        int _socket; // socket file descriptor

        struct sockaddr_in m_srv_addr; // contains server address
        struct hostent* m_srv_ptr;   // pointer to server info

		bool m_running = false;
		bool m_open = false;
		std::string m_status = "";

        bool init();
		bool connect_socket();		
		void close_socket();

		std::string system_error(std::string const& msg);

		std::vector<std::string> m_errors;
		

	public:
		SocketClient()
		{ 
			m_srv_hostname = DEFAULT_HOST;
			m_srv_port_no = DEFAULT_PORT;
		 }

		SocketClient(std::string const& srv_hostname, unsigned short srv_port)
		{
			m_srv_hostname = srv_hostname.c_str();
			m_srv_port_no = srv_port;
		}

		SocketClient(const char* srv_hostname, unsigned short srv_port)
		{
			m_srv_hostname = srv_hostname;
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

    //========================================

    bool SocketClient::init()
	{
        _socket = socket(AF_INET, SOCK_STREAM, 0);  // create socket

        if (_socket < 0)
		{
            m_status = "ERROR opening socket";
			m_errors.push_back(system_error(m_status));
            return false;
        }		

        m_srv_ptr = gethostbyname(m_srv_hostname);

        if(m_srv_ptr == NULL)
		{
            m_status = "ERROR host not found";
			m_errors.push_back(system_error(m_status));
            return false;
        }

        // populate serv_addr
        bzero((char *) &m_srv_addr, sizeof(m_srv_addr)); // initialize to zeros

        m_srv_addr.sin_family = AF_INET;    

        // copy server address bytes to serv_addr
        bcopy((char *)m_srv_ptr->h_addr_list[0], (char *)&m_srv_addr.sin_addr.s_addr, m_srv_ptr->h_length);
        // #define h_addr  h_addr_list[0]  /* address, for backward compatiblity */		

        m_srv_addr.sin_port = htons(m_srv_port_no);            

        m_open = true;

		m_status = "Client Initialized";

        return true;
    }

    bool SocketClient::connect_socket()
	{
        if (!m_open)
		{
			m_status = "Client not initialized.";
			return false;
		}

        // connect the socket
        int res = connect(_socket,(struct sockaddr *) &m_srv_addr,sizeof(m_srv_addr));		
		
		if (res < 0)
		{
			m_status = "ERROR Client connect failed";
			m_errors.push_back(system_error(m_status));
			close_socket();
			return false;
		}

		return true;
    }

    void SocketClient::close_socket()
	{
        if (!m_open)
			return;

		close(_socket);
		m_open = false;
    }

    void SocketClient::start()
	{
        if (!init() || !connect_socket())
			return;

		m_running = true;
		m_status = "Client started";
    }

    void SocketClient::stop()
	{
		m_running = false;
		close_socket();
		m_status = "Client stopped";
	}

    bool SocketClient::send_text(std::string const& text)
	{
		assert(m_running);
		if (!m_running)
			return false;

        auto n_chars = write(_socket, text.data(), static_cast<int>(text.size()));

        if (n_chars < 0)
		{
            m_status = "ERROR writing to socket";
			m_errors.push_back(system_error(m_status));
            return false;
        }

        return true;
    }

    std::string SocketClient::receive_text()
	{
		assert(m_running);

		char buffer[MAX_CHARS]; // characters are read into this buffer
        ssize_t n_chars; // number of characters read or written

        bool waiting = true;
		std::ostringstream oss;

        bzero(buffer, MAX_CHARS);        

		while (waiting)
		{
			n_chars = read(_socket, buffer, MAX_CHARS - 1);
			if (n_chars > 0)
			{
				waiting = false;
				oss << buffer;
			}
		}

		return oss.str();
	}

	std::string SocketClient::latest_error()
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

	std::string SocketClient::system_error(std::string const& msg)
	{
		return msg + ": " + strerror(errno);
	}
}

/*
http://www.linuxhowtos.org/C_C++/socket.htm

http://www.linuxhowtos.org/data/6/connect.txt




*/