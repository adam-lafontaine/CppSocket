#include "../hpp/SocketServer.hpp"

#include <sstream>
#include <cassert>

namespace MySocketLib
{
	bool SocketServer::init()
	{
		// initialize WSA
		WSAData wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			m_status = "Server WSAStartup failed : " + iResult;
			m_errors.push_back(m_status);
			return false;
		}

		// Create the TCP socket
		m_srv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		m_open = true;

		// Create the server address
		m_serverAddress = { 0 };
		m_serverAddress.sin_family = AF_INET;
		m_serverAddress.sin_port = htons(m_port_no);
		m_serverAddress.sin_addr.s_addr = inet_addr(m_ip_address);

		m_status = "Server Initialized";
		return true;
	}


	bool SocketServer::bind_socket()
	{
		// bind the socket
		if (bind(m_srv_socket, (SOCKADDR*)&m_serverAddress, sizeof(m_serverAddress)) == SOCKET_ERROR) {
			close_socket();
			m_status = "Server bind() failed.";
			m_errors.push_back(m_status);
			return false;
		}

		return true;
	}


	bool SocketServer::listen_socket()
	{
		m_running = false;
		const auto port = std::to_string(m_port_no);
		if (listen(m_srv_socket, 1) == SOCKET_ERROR) {
			close_socket();
			m_status = "Server error listening on socket";
			m_errors.push_back(m_status);
			return false;
		}

		m_status = "Listening on " + m_public_ip + " : " + port;

		return true;
	}


	bool SocketServer::connect_client()
	{
		if (!m_running) {
			m_status = "Server cannot connect, server not running";
			m_errors.push_back(m_status);
			return false;
		}

		m_status = "Server waiting for client";

		m_connected = false;
		m_cli_socket = SOCKET_ERROR;
		while (m_cli_socket == SOCKET_ERROR) {
			m_cli_socket = accept(m_srv_socket, NULL, NULL);
		}

		m_status = "Server connected to client";
		m_connected = true;

		return true;
	}


	void SocketServer::start()
	{
		bool result = init();

		if (!init() || !bind_socket() || !listen_socket())
			return;

		m_running = true;
	}


	void SocketServer::stop()
	{
		disconnect_client();

		m_running = false;
		close_socket();
		m_status = "Server stopped";
	}


	void SocketServer::close_socket()
	{
		if (m_open) {
			closesocket(m_srv_socket);
			WSACleanup();
			m_open = false;
		}
	}


	void SocketServer::disconnect_client()
	{
		if (m_connected) {
			closesocket(m_cli_socket);
			m_connected = false;
		}
	}


	std::string SocketServer::receive_text()
	{
		assert(m_running);
		assert(m_connected);

		char recvbuf[MAX_CHARS] = "";
		bool waiting = true;
		std::ostringstream oss;

		while (waiting) {
			int bytesRecv = recv(m_cli_socket, recvbuf, MAX_CHARS, 0);
			if (bytesRecv > 0) {
				waiting = false;
				oss << recvbuf;
			}
		}

		return oss.str();
	}


	void SocketServer::send_text(std::string const& text)
	{
		assert(m_running);
		assert(m_connected);
		if (!m_running || !m_connected)
			return;

		std::vector<char> data(text.begin(), text.end());

		int bytesSent = send(m_cli_socket, data.data(), static_cast<int>(data.size()), 0);
	}


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

