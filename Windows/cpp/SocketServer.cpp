#include "../hpp/SocketServer.hpp"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

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


namespace MySocketLib
{
	struct ServerSocketInfo
	{
		struct sockaddr_in srv_addr = { 0 };

		SOCKET srv_socket = NULL;
		SOCKET cli_socket = NULL;

		bool srv_running = false;
		bool cli_connected = false;		
	};


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


	bool SocketServer::running()
	{
		return m_socket_info != nullptr && m_socket_info->srv_running;
	}


	bool SocketServer::connected()
	{
		return m_socket_info != nullptr && m_socket_info->cli_connected;
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


	std::string SocketServer::latest_error()
	{
		const auto msg = to_csv(m_errors);

		m_errors.clear();

		return msg;
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

}

