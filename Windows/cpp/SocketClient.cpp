#include "../hpp/SocketClient.hpp"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#pragma comment (lib,"ws2_32.lib")

#include <sstream>
#include <cassert>


namespace MySocketLib
{	
	struct ClientSocketInfo
	{
		SOCKET socket = NULL;
		sockaddr_in srv_addr = { 0 };
	};


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

		m_socket_info = new client_socket_info_t;

		// Create the TCP socket
		m_socket_info->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		// Create the server address
		m_socket_info->srv_addr = { 0 };
		m_socket_info->srv_addr.sin_family = AF_INET;
		m_socket_info->srv_addr.sin_port = htons(m_srv_port_no);
		m_socket_info->srv_addr.sin_addr.s_addr = inet_addr(m_srv_ip);

		m_open = true;

		m_status = "Client Initialized";
		return true;
	}


	void SocketClient::close_socket()
	{
		if (!m_open)
			return;

		closesocket(m_socket_info->socket);
		WSACleanup();
		m_open = false;

		delete m_socket_info;
	}


	bool SocketClient::connect_socket()
	{
		if (!m_open)
		{
			m_status = "Client not initialized.";
			return false;
		}

		// connect the socket
		if (connect(m_socket_info->socket, (SOCKADDR*)&m_socket_info->srv_addr, sizeof(m_socket_info->srv_addr)) == SOCKET_ERROR) {
			m_status = "Client Connect() failed";
			m_errors.push_back(m_status);
			close_socket();
			return false;
		}

		return true;
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

		auto n_chars = send(m_socket_info->socket, text.data(), static_cast<int>(text.size()), 0);

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

		char recvbuf[MAX_CHARS] = "";

		bool waiting = true;
		std::ostringstream oss;

		while (waiting)
		{
			auto n_chars = recv(m_socket_info->socket, recvbuf, MAX_CHARS, 0);
			if (n_chars > 0) {
				waiting = false;
				oss << recvbuf;
			}
		}

		return oss.str();
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


	std::string SocketClient::latest_error()
	{
		const auto msg = to_csv(m_errors);

		m_errors.clear();

		return msg;
	}

}

