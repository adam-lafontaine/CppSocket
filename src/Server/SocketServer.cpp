#include "SocketServer.hpp"
#include "../os_socket/os_socket_server.hpp"



SocketServer::SocketServer()
{
	on_send = [](bool, cstring) {};
	on_receive = [](bool, cstring) {};
}


SocketServer::~SocketServer()
{
	if (is_connected())
	{
		disconnect();
	}
	
	if (is_open())
	{
		close();
	}

	if (m_server != nullptr)
	{
		delete m_server;
		m_server = nullptr;
	}

	os_socket_cleanup();
}


bool SocketServer::open(int port)
{
	if (m_server == nullptr)
	{
		m_server = new ServerSocketInfo;
	}

	auto& server = *m_server;

	return os_socket_init() && os_server_open(server, port) && os_server_bind(server);
}


bool SocketServer::start()
{
	return m_server != nullptr && os_server_listen(*m_server);
}


bool SocketServer::connect()
{
	return m_server != nullptr && os_server_accept(*m_server);
}


void SocketServer::disconnect()
{
	os_server_disconnect(*m_server);
}


void SocketServer::close()
{
	os_server_close(*m_server);
}


bool SocketServer::is_open()
{
	return m_server != nullptr && m_server->open;
}


bool SocketServer::is_running()
{
	return m_server != nullptr && m_server->server_running;
}


bool SocketServer::is_connected()
{
	return m_server != nullptr && m_server->client_connected;
}


void SocketServer::send_text(cstring buffer)
{
	auto socket = m_server->client_socket;
	auto length = (int)BUFFER_SIZE;

	bool result = os_socket_send_buffer(socket, buffer, length);

	on_send(result, buffer);
}


void SocketServer::receive_text()
{
	auto socket = m_server->client_socket;
	auto buffer = m_recv_buffer;
	auto length = (int)BUFFER_SIZE;

	bool result = os_socket_receive_buffer(socket, buffer, length);

	on_receive(result, buffer);
}