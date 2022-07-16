#include "SocketClient.hpp"
#include "../os_socket/os_socket_client.hpp"


SocketClient::SocketClient()
{
	on_send = [](bool, cstring) {};
	on_receive = [](bool, cstring) {};
}


SocketClient::~SocketClient()
{
	if (is_connected())
	{
		disconnect();
	}

	if (m_client != nullptr)
	{
		delete m_client;
		m_client = nullptr;
	}

	os_socket_cleanup();
}


bool SocketClient::open(cstring ip_address, int port)
{
	if (m_client == nullptr)
	{
		m_client = new ClientSocketInfo;
	}

	return os_socket_init() && os_client_open(*m_client, ip_address, port);
}


bool SocketClient::connect()
{
	if (!is_open())
	{
		return false;
	}

	return m_client != nullptr && os_client_connect(*m_client);
}


void SocketClient::disconnect()
{
	os_client_close(*m_client);
}


bool SocketClient::is_open()
{
	return m_client != nullptr && m_client->open;
}


bool SocketClient::is_connected()
{
	return m_client != nullptr && m_client->connected;
}


void SocketClient::send_text(cstring message)
{
	auto socket = m_client->client_socket;
	auto buffer = message;
	auto length = (int)BUFFER_SIZE;

	bool result = os_socket_send_buffer(socket, buffer, length);

	on_send(result, buffer);
}


void SocketClient::receive_text()
{
	auto socket = m_client->client_socket;
	char buffer[BUFFER_SIZE] = { 0 };
	auto length = (int)BUFFER_SIZE;

	bool result = os_socket_receive_buffer(socket, buffer, length);

	on_receive(result, buffer);
}