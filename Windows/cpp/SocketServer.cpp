/*
*    Authors: Adam Lafontaine, Yougui Chen
*     Course: INFO 5104
* Assignment: Project 3, Socket Library
*       Date: January 9, 2018
*
*       File: SocketServer.cpp
*/

#include<SocketServer.hpp>
#include<vector>
#include<sstream>
#include<cassert>

namespace MySocketLib {

	/*
	*        Purpose: Initializes WSA, TCP socket and the server address
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Returns false if a problem occured during initialization
	*/
	bool SocketServer::init() {
		// initialize WSA
		WSAData wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			_status = "Server WSAStartup failed : " + iResult;
			return false;
		}

		// Create the TCP socket
		_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		_open = true;

		// Create the server address
		_serverAddress = { 0 };
		_serverAddress.sin_family = AF_INET;
		_serverAddress.sin_port = htons(PORT);
		_serverAddress.sin_addr.s_addr = inet_addr(_ip_address);

		_status = "Server Initialized";
		return true;
	}

	/*
	*        Purpose: Binds server socket the server address
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Returns false if a problem occured during binding
	*/
	bool SocketServer::bind_socket() {

		// bind the socket
		if (bind(_hSocket, (SOCKADDR*)&_serverAddress, sizeof(_serverAddress)) == SOCKET_ERROR) {
			close();
			_status = "Server bind() failed.";
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
	bool SocketServer::listen_socket() {

		_running = false;
		if (listen(_hSocket, 1) == SOCKET_ERROR) {
			close();
			_status = "Server error listening on socket";
			return false;
		}

		_status = "Server started";

		return true;
	}

	/*
	*        Purpose: Waits until connects with client
	*     Parameters: None
	*  Preconditions: Server is running
	* Postconditions: Returns true if successful
	*/
	bool SocketServer::connect_client() {
		if (!_running) {
			_status = "Server cannot connect, server not running";
			return false;
		}

		_status = "Server waiting for client";

		_connected = false;
		_hAccepted = SOCKET_ERROR;
		while (_hAccepted == SOCKET_ERROR) {
			_hAccepted = accept(_hSocket, NULL, NULL);
		}

		_status = "Server connected to client";
		_connected = true;

		return true;
	}

	/*
	*        Purpose: Attempts to initialize the server and listen on the socket
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Server is running if successful
	*/
	void SocketServer::start() {

		bool result = init();

		if (!result)
			return;

		result = bind_socket();

		if (!result)
			return;

		result = listen_socket();

		if (!result)
			return;

		_running = true;
	}

	/*
	*        Purpose: Stops the server and cleans up resources
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: server is stopped and can be restarted
	*/
	void SocketServer::stop() {
		disconnect_client();

		_running = false;
		close();
		_status = "Server stopped";
	}

	/*
	*        Purpose: Closes socket and WSA
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Resources cleaned up.  Able to reconnect.
	*/
	void SocketServer::close() {
		if (_open) {
			closesocket(_hSocket);
			WSACleanup();
			_open = false;
		}
	}

	/*
	*        Purpose: Disconnects from the client
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Client disconnected but still running.
	*/
	void SocketServer::disconnect_client() {
		if (_connected) {
			closesocket(_hAccepted);
			_connected = false;
		}
	}

	/*
	*        Purpose: Waits for a message from the client
	*     Parameters: None
	*  Preconditions: Server is running and connected to client
	* Postconditions: Returns the message when received
	*/
	string SocketServer::receive_text() {
		assert(_running);
		assert(_connected);

		char recvbuf[MAX_CHARS] = "";
		bool waiting = true;
		ostringstream oss;

		while (waiting) {
			int bytesRecv = recv(_hAccepted, recvbuf, MAX_CHARS, 0);
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
	void SocketServer::send_text(string const& text) {
		assert(_running);
		assert(_connected);
		if (!_running || !_connected)
			return;

		vector<char> data(text.begin(), text.end());

		int bytesSent = send(_hAccepted, data.data(), static_cast<int>(data.size()), 0);
	}

}

