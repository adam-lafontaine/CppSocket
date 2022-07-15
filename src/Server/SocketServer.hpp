#pragma once

class ServerSocketInfo;

class SocketServer
{
private:
	
	using server_ptr_t = ServerSocketInfo*;

	server_ptr_t m_server = nullptr;


public:

	~SocketServer();

	bool open(int port);

	bool start();	

	bool connect();

	void disconnect();

	void close();

	bool is_open();

	bool is_running();

	bool is_connected();
};