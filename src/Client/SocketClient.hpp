#pragma once

#include <functional>
#include <string>


using cstring = const char*;


class ClientSocketInfo;


class SocketClient
{
private:
	using client_ptr_t = ClientSocketInfo*;

	client_ptr_t m_client = nullptr;

	static constexpr size_t BUFFER_SIZE = 50;


public:
	SocketClient();

	~SocketClient();

	bool open(cstring ip_address, int port);

	bool connect();

	void disconnect();

	bool is_open();

	bool is_connected();

	void send_text(std::string const& message);

	void receive_text();

	std::function<void(bool, cstring)> on_send;

	std::function<void(bool, cstring)> on_receive;
};