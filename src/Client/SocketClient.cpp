#include "SocketClient.hpp"

/*

// LINUX
struct timeval tv;
tv.tv_sec = timeout_in_seconds;
tv.tv_usec = 0;
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

// WINDOWS
DWORD timeout = timeout_in_seconds * 1000;
setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

*/

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

	using socket_t = SOCKET;
	using addr_t = SOCKADDR;

    struct ClientSocketInfo
	{
		sockaddr_in srv_addr = { 0 };
		socket_t socket = NULL;

		bool connected = false;
	};


	static void os_close_socket(socket_t socket)
	{
		closesocket(socket);
	}


	static int os_socket_read(socket_t socket, char* buffer, int buffer_size)
	{
		return recv(socket, buffer, buffer_size, 0);
	}


	static int os_socket_write(socket_t socket, const char* buffer, int buffer_size)
	{
		return send(socket, buffer, buffer_size, 0);
	}


	static bool os_socket_connect(ClientSocketInfo* socket_info)
	{
		auto socket = socket_info->socket;
        auto addr = (addr_t*)&socket_info->srv_addr;
        int size = sizeof(socket_info->srv_addr);

		return connect(socket, addr, size) != SOCKET_ERROR;
	}

	static void os_socket_cleanup()
	{
		WSACleanup();
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

#else

	using socket_t = int;
	using addr_t = struct sockaddr;

    struct ClientSocketInfo
    {
        struct sockaddr_in srv_addr = { 0 };
        socket_t socket;

        bool connected = false;     
    };


	static void os_close_socket(socket_t socket)
	{
		close(socket);
	}


	static int os_socket_read(socket_t socket, char* buffer, size_t buffer_size)
	{
		return read(socket, buffer, buffer_size - 1);
	}


	static int os_socket_write(socket_t socket, const char* buffer, size_t buffer_size)
	{
		return write(socket, buffer, buffer_size);
	}


	bool os_socket_connect(ClientSocketInfo* socket_info)
	{
		auto socket = socket_info->socket;
        auto addr = (addr_t*)&socket_info->srv_addr;
        auto size = sizeof(socket_info->srv_addr);

		return connect(socket, addr, size) >= 0;
	}


	static void os_socket_cleanup()
	{
		// do nothing
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


	std::string SocketClient::receive_text()
	{
		assert(connected());

		char buffer[MAX_CHARS] = "";

		auto n_chars = os_socket_read(m_socket_info->socket, buffer, MAX_CHARS);
		if (n_chars < 0)
		{
			m_status = "ERROR reading from socket";
			m_errors.push_back(m_status);
			return "error";
		}

		std::ostringstream oss;
		oss << buffer;

		return oss.str();
	}


	bool SocketClient::send_text(std::string const& text)
	{
		assert(connected());

		if (!connected())
			return false;

        auto socket = m_socket_info->socket;
		auto data = text.data();
		auto size = static_cast<int>(text.size());

        auto n_chars = os_socket_write(socket, data, size);

        if (n_chars < 0)
		{
            m_status = "ERROR writing to socket";
			m_errors.push_back(m_status);
            return false;
        }

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
        //auto socket = m_socket_info->socket;
        //auto addr = (addr_t*)&m_socket_info->srv_addr;
        //auto size = sizeof(m_socket_info->srv_addr);
		
		if (!os_socket_connect(m_socket_info))
		{
			m_status = "ERROR Client connect failed";
			m_errors.push_back(m_status);
			close_socket();
			return false;
		}

        m_socket_info->connected = true;

		return true;
    }


	void SocketClient::destroy_socket_info()
	{
		if (m_socket_info == nullptr)
			return;

		if (connected())
		{
			os_close_socket(m_socket_info->socket);
			os_socket_cleanup();
		}

		delete m_socket_info;
	}


	void SocketClient::close_socket()
	{
		if (!connected())
			return;

		os_close_socket(m_socket_info->socket);
		os_socket_cleanup();

		m_socket_info->connected = false;
	}




}