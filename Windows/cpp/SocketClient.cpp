/*
*    Authors: Adam Lafontaine, Yougui Chen
*     Course: INFO 5104
* Assignment: Project 3, Socket Library
*       Date: January 9, 2018
*
*       File: SocketClient.cpp
*/

#include<SocketClient.hpp>
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
	bool SocketClient::init() {
		// initialize WSA
		WSAData wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			_status = "Client WSAStartup failed: " + iResult;
			return false;
		}

		// Create the TCP socket
		_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		

		// Create the server address
		_serverAddress = { 0 };
		_serverAddress.sin_family = AF_INET;
		_serverAddress.sin_port = htons(PORT);
		_serverAddress.sin_addr.s_addr = inet_addr(_ip_address);

		_open = true;

		_status = "Client Initialized";
		return true;
	}

	/*
	*        Purpose: Closes socket and WSA
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Resources cleaned up.  Able to reconnect.
	*/
	void SocketClient::close() {
		if (!_open)
			return;

		closesocket(_hSocket);
		WSACleanup();
		_open = false;
	}

	/*
	*        Purpose: Connects the client to the server
	*     Parameters: None
	*  Preconditions: Client has been initialized
	* Postconditions: Returns true if successful, false if not
	*/
	bool SocketClient::connect_socket() {

		if (!_open) {
			_status = "Client not initialized.";
			return false;
		}

		// connect the socket
		if (connect(_hSocket, (SOCKADDR*)&_serverAddress, sizeof(_serverAddress)) == SOCKET_ERROR) {
			_status = "Client Connect() failed";
			close();
			return false;
		}

		return true;
	}

	/*
	*        Purpose: Attempts to initialize the client and connect to the server
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Client is running if successful
	*/
	void SocketClient::start() {
		if (!init()) {			
			return;
		}

		if (!connect_socket()) {
			return;
		}

		_running = true;
		_status = "Client started";
	}

	/*
	*        Purpose: Stops the client and cleans up resources
	*     Parameters: None
	*  Preconditions: None
	* Postconditions: Client is stopped and can be restarted
	*/
	void SocketClient::stop() {
		_running = false;
		close();
		_status = "Client stopped";
	}

	/*
	*        Purpose: Sends a string of text to the server
	*     Parameters: The text to send
	*  Preconditions: Client is running and connected to server
	* Postconditions: Message is sent
	*/
	void SocketClient::send_text(string const& text) {
		assert(_running);
		if (!_running)
			return;

		vector<char> data(text.begin(), text.end());

		int bytesSent = send(_hSocket, data.data(), static_cast<int>(data.size()), 0);
	}

	/*
	*        Purpose: Waits for a message from the server
	*     Parameters: None
	*  Preconditions: Client is running and connected to server
	* Postconditions: Returns the message when received
	*/
	string SocketClient::receive_text() {
		assert(_running);

		char recvbuf[MAX_CHARS] = "";
		bool waiting = true;
		ostringstream oss;

		while (waiting) {
			int bytesRecv = recv(_hSocket, recvbuf, MAX_CHARS, 0);
			if (bytesRecv > 0) {
				waiting = false;
				oss << recvbuf;
			}
		}

		return oss.str();
	}

}

