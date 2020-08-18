#include "../hpp/SocketServer.hpp"

#include <vector>
#include <sstream>
#include <cassert>

namespace MySocketLib
{
	/*
	*        Purpose: Initializes WSA, TCP socket and the server address
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Returns false if a problem occured during initialization
	*/
	bool SocketServer::init()
	{
		// initialize WSA
		WSAData wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			m_status = "Server WSAStartup failed : " + iResult;
			return false;
		}

		// Create the TCP socket
		m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		m_open = true;

		// Create the server address
		m_serverAddress = { 0 };
		m_serverAddress.sin_family = AF_INET;
		m_serverAddress.sin_port = htons(PORT);
		m_serverAddress.sin_addr.s_addr = inet_addr(m_ip_address);

		m_status = "Server Initialized";
		return true;
	}

	/*
	*        Purpose: Binds server socket the server address
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Returns false if a problem occured during binding
	*/
	bool SocketServer::bind_socket()
	{
		// bind the socket
		if (bind(m_hSocket, (SOCKADDR*)&m_serverAddress, sizeof(m_serverAddress)) == SOCKET_ERROR) {
			close();
			m_status = "Server bind() failed.";
			return false;
		}

		return true;
	}

	/*
	*        Purpose: Begins listening on socket
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Returns false and closes resources if there is a problem
	*/
	bool SocketServer::listen_socket()
	{
		m_running = false;
		if (listen(m_hSocket, 1) == SOCKET_ERROR) {
			close();
			m_status = "Server error listening on socket";
			return false;
		}

		m_status = "Server started";

		return true;
	}

	/*
	*        Purpose: Waits until connects with client
	*     Parameters: None
	*  Preconditions: Server is running
	* Postconditions: Returns true if successful
	*/
	bool SocketServer::connect_client()
	{
		if (!m_running) {
			m_status = "Server cannot connect, server not running";
			return false;
		}

		m_status = "Server waiting for client";

		m_connected = false;
		m_hAccepted = SOCKET_ERROR;
		while (m_hAccepted == SOCKET_ERROR) {
			m_hAccepted = accept(m_hSocket, NULL, NULL);
		}

		m_status = "Server connected to client";
		m_connected = true;

		return true;
	}

	/*
	*        Purpose: Attempts to initialize the server and listen on the socket
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Server is running if successful
	*/
	void SocketServer::start()
	{
		bool result = init();

		if (!result)
			return;

		result = bind_socket();

		if (!result)
			return;

		result = listen_socket();

		if (!result)
			return;

		m_running = true;
	}

	/*
	*        Purpose: Stops the server and cleans up resources
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: server is stopped and can be restarted
	*/
	void SocketServer::stop()
	{
		disconnect_client();

		m_running = false;
		close();
		m_status = "Server stopped";
	}

	/*
	*        Purpose: Closes socket and WSA
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Resources cleaned up.  Able to reconnect.
	*/
	void SocketServer::close()
	{
		if (m_open) {
			closesocket(m_hSocket);
			WSACleanup();
			m_open = false;
		}
	}

	/*
	*        Purpose: Disconnects from the client
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Client disconnected but still running.
	*/
	void SocketServer::disconnect_client()
	{
		if (m_connected) {
			closesocket(m_hAccepted);
			m_connected = false;
		}
	}

	/*
	*        Purpose: Waits for a message from the client
	*     Parameters: None
	*  Preconditions: Server is running and connected to client
	* Postconditions: Returns the message when received
	*/
	std::string SocketServer::receive_text()
	{
		assert(m_running);
		assert(m_connected);

		char recvbuf[MAX_CHARS] = "";
		bool waiting = true;
		std::ostringstream oss;

		while (waiting) {
			int bytesRecv = recv(m_hAccepted, recvbuf, MAX_CHARS, 0);
			if (bytesRecv > 0) {
				waiting = false;
				oss << recvbuf;
			}
		}

		return oss.str();
	}

	/*
	*        Purpose: Sends a string of text to the client
	*     Parameters: The text to send
	*  Preconditions: Server is running and connected to client
	* Postconditions: Message is sent
	*/
	void SocketServer::send_text(std::string const& text)
	{
		assert(m_running);
		assert(m_connected);
		if (!m_running || !m_connected)
			return;

		std::vector<char> data(text.begin(), text.end());

		int bytesSent = send(m_hAccepted, data.data(), static_cast<int>(data.size()), 0);
	}

}

