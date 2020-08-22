#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

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

		const char* DEFAULT_IP_ADDRESS = "127.0.0.1";

		const char* m_srv_ip = DEFAULT_IP_ADDRESS;
        unsigned short m_srv_port_no = DEFAULT_PORT;
		
        int m_socket; // socket file descriptor

        struct sockaddr_in m_srv_addr = { 0 }; // contains server address
        struct hostent* m_srv_ptr;   // pointer to server info

		bool m_running = false;
		bool m_open = false;
		std::string m_status = "";

		std::vector<std::string> m_errors;

        bool init();
		bool connect_socket();		
		void close_socket();

		// TODO: move to helpers
		std::string system_error(std::string const& msg);
		std::string to_csv(std::vector<std::string> const& list);

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

    //========================================

	// TODO: move to helpers
	std::string SocketClient::system_error(std::string const& msg)
	{
		return msg + ": " + strerror(errno);
	}

	// TODO: move to helpers
	std::string SocketClient::to_csv(std::vector<std::string> const& list)
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

    bool SocketClient::init()
	{
        m_socket = socket(AF_INET, SOCK_STREAM, 0);  // create socket

        if (m_socket < 0)
		{
            m_status = "ERROR opening socket";
			m_errors.push_back(m_status);
            return false;
        }

		// populate serv_addr
		m_srv_addr.sin_family = AF_INET;
		m_srv_addr.sin_port = htons(m_srv_port_no);

		if(inet_pton(AF_INET, m_srv_ip, &m_srv_addr.sin_addr) < 0)
		{
			m_status = "Could not get server address";
			m_errors.push_back(m_status);
			return false;
		}    

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
        int res = connect(m_socket,(struct sockaddr *) &m_srv_addr,sizeof(m_srv_addr));
		
		if (res < 0)
		{
			m_status = "ERROR Client connect failed";
			m_errors.push_back(m_status);
			close_socket();
			return false;
		}

		return true;
    }

    void SocketClient::close_socket()
	{
        if (!m_open)
			return;

		close(m_socket);
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

        auto n_chars = write(m_socket, text.data(), static_cast<int>(text.size()));

        if (n_chars < 0)
		{
            m_status = "ERROR writing to socket";
			m_errors.push_back(m_status);
            return false;
        }

        return true;
    }

    std::string SocketClient::receive_text()
	{
		assert(m_running);

		char buffer[MAX_CHARS]; // characters are read into this buffer

        bool waiting = true;
		std::ostringstream oss;

        bzero(buffer, MAX_CHARS);        

		while (waiting)
		{
			auto n_chars = read(m_socket, buffer, MAX_CHARS - 1);
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
		const auto msg = to_csv(m_errors);

		m_errors.clear();

		return msg;
	}

	
}

/*
http://www.linuxhowtos.org/C_C++/socket.htm

http://www.linuxhowtos.org/data/6/connect.txt




*/