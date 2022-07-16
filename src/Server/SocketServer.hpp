#pragma once

#include <functional>
#include <string>


using cstring = const char*;


class ServerSocketInfo;

class SocketServer
{
private:
	
	using server_ptr_t = ServerSocketInfo*;

	server_ptr_t m_server = nullptr;

	static constexpr size_t BUFFER_SIZE = 50;


public:	

	SocketServer();

	~SocketServer();

	bool open(int port);

	bool start();

	bool connect();

	void disconnect();

	void close();

	bool is_open();

	bool is_running();

	bool is_connected();

	void send_text(std::string const& message);

	void receive_text();

	std::function<void(bool, cstring)> on_send;

	std::function<void(bool, cstring)> on_receive;
};