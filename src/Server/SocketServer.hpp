#pragma once

class SocketServer
{
private:
	class ServerSocketInfo;
	using server_ptr_t = ServerSocketInfo*;

	server_ptr_t m_server = nullptr;


public:

	SocketServer();

	~SocketServer();
};