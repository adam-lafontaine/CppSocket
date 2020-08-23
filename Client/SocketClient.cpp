#include "SocketClient.hpp"

#if defined(_WIN32)

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#else

#include <unistd.h>
#include <arpa/inet.h>

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

    struct ClientSocketInfo
	{
		sockaddr_in srv_addr = { 0 };
		SOCKET socket = NULL;

		bool connected = false;
	};


    void SocketClient::destroy_socket_info()
	{
		if (m_socket_info == nullptr)
			return;

		if (connected())
		{
			closesocket(m_socket_info->socket);
			WSACleanup();
		}

		delete m_socket_info;
	}


    bool SocketClient::init()
	{
		// initialize WSA
		WSAData wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			m_status = "Client WSAStartup failed: " + iResult;
			m_errors.push_back(m_status);
			return false;
		}

		create_socket_info();

		m_status = "Client Initialized";
		return true;
	}


    bool SocketClient::connect_socket()
	{
		if (!running())
		{
			m_status = "Client not initialized.";
			return false;
		}

		// connect the socket
		auto socket = m_socket_info->socket;
		auto addr = (SOCKADDR*)&m_socket_info->srv_addr;
		auto size = sizeof(m_socket_info->srv_addr);

		if (connect(socket, addr, size) == SOCKET_ERROR)
		{
			m_status = "Client Connect() failed";
			m_errors.push_back(m_status);
			close_socket();
			return false;
		}

		m_socket_info->connected = true;

		return true;
	}


    bool SocketClient::send_text(std::string const& text)
	{
		assert(connected());

		if (!connected())
			return false;

		auto socket = m_socket_info->socket;
		auto data = text.data();
		auto size = static_cast<int>(text.size());

		auto n_chars = send(socket, data, size, 0);

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
		assert(connected());

		char buffer[MAX_CHARS] = "";

		bool waiting = true;
		std::ostringstream oss;

		while (waiting)
		{
			auto n_chars = recv(m_socket_info->socket, buffer, MAX_CHARS, 0);
			if (n_chars > 0) {
				waiting = false;
				oss << buffer;
			}
		}

		return oss.str();
	}


    void SocketClient::close_socket()
	{
		if (!connected())
			return;

		closesocket(m_socket_info->socket);
		WSACleanup();

		m_socket_info->connected = false;
	}

#else

    struct ClientSocketInfo
    {
        struct sockaddr_in srv_addr = { 0 };
        int socket;

        bool connected = false;     
    };


    void SocketClient::destroy_socket_info()
    {
        if (m_socket_info == nullptr)
			return;

        if (connected())
		{
			close(m_socket_info->socket);
		}

		delete m_socket_info;
    }


    bool SocketClient::init()
	{
        create_socket_info();

		if(inet_pton(AF_INET, m_srv_ip, &m_socket_info->srv_addr.sin_addr) < 0)
		{
			m_status = "Could not get server address";
			m_errors.push_back(m_status);
			return false;
		} 

		m_status = "Client Initialized";
        return true;
    }


    bool SocketClient::connect_socket()
	{		
        if (!running())
		{
			m_status = "Client not initialized.";
			return false;
		}

        // connect the socket
        auto socket = m_socket_info->socket;
        auto addr = (struct sockaddr *)&m_socket_info->srv_addr;
        auto size = sizeof(m_socket_info->srv_addr);
		
		if (connect(socket, addr, size) < 0)
		{
			m_status = "ERROR Client connect failed";
			m_errors.push_back(m_status);
			close_socket();
			return false;
		}

        m_socket_info->connected = true;

		return true;
    }


    bool SocketClient::send_text(std::string const& text)
	{
		assert(connected());

		if (!connected())
			return false;

        auto socket = m_socket_info->socket;
		auto data = text.data();
		auto size = static_cast<int>(text.size());

        auto n_chars = write(socket, data, size);

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
		assert(connected());

		char buffer[MAX_CHARS] = "";

        bool waiting = true;
		std::ostringstream oss;     

		while (waiting)
		{
			auto n_chars = read(m_socket_info->socket, buffer, MAX_CHARS - 1);
			if (n_chars > 0)
			{
				waiting = false;
				oss << buffer;
			}
		}

		return oss.str();
	}


    void SocketClient::close_socket()
	{
        if (!connected())
			return;

		close(m_socket_info->socket);

        m_socket_info->connected = false;
    }


#endif


    static void create_socket(ClientSocketInfo* info, const char* srv_ip, unsigned short srv_port)
	{
		// Create the TCP socket
		info->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        /*
        if (m_socket < 0)
		{
            m_status = "ERROR opening socket";
			m_errors.push_back(m_status);
            return false;
        }
        */
		
		info->srv_addr = { 0 };
		info->srv_addr.sin_family = AF_INET;
		info->srv_addr.sin_addr.s_addr = inet_addr(srv_ip);
		info->srv_addr.sin_port = htons(srv_port);		
	}


    void SocketClient::create_socket_info()
	{
		if (m_socket_info == nullptr)
			m_socket_info = new socket_info_t;

		create_socket(m_socket_info, m_srv_ip, m_srv_port_no);
	}


    bool SocketClient::running()
	{
		return m_socket_info != nullptr;
	}


	bool SocketClient::connected()
	{
		return m_socket_info != nullptr && m_socket_info->connected;
	}


    void SocketClient::start()
	{
		if (!init() || !connect_socket())
			return;

		assert(running());
		assert(connected());
		
		m_status = "Client started";
	}


	void SocketClient::stop()
	{
		close_socket();
		m_status = "Client stopped";
	}


    std::string SocketClient::latest_error()
	{
		const auto msg = to_csv(m_errors);

		m_errors.clear();

		return msg;
	}

}