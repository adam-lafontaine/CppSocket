#include "../Server/SocketServer.hpp"
#include "../Client/SocketClient.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <cassert>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <sstream>
#include <iterator>

bool server_started = false;
bool client_started = false;


std::mutex console_mtx;
void print_line(std::string const& msg = "")
{
	std::lock_guard<std::mutex> lk(console_mtx);

	std::cout << msg << '\n';
}


//======= SEND MESSAGES ====================


bool end_session_msg(std::string const& msg)
{
	auto copy = msg;
	std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);

	return copy == "goodbye";
}


void send_messages_server()
{
	const auto process_msg = [](std::string const& msg)
	{
		auto rev = msg;
		std::reverse(rev.begin(), rev.end());

		std::string response = "The message backwards is: '" + rev + "'";

		return response;
	};

    SocketLib::SocketServer server;

    server.start();
	print_line(server.status());

    server_started = true;

	server.connect_client();
	print_line(server.status());

    // handle input from client
	while (server.running() && server.connected())
	{
		const auto msg = server.receive_text();
		
		const auto response = process_msg(msg);
		server.send_text(response);

		if (end_session_msg(msg))
		{
			print_line("Ending server session");
			server.disconnect_client();
			server.stop();
		}		
	}

    assert(!server.running());
	assert(!server.connected());

	print_line(server.status());
}


void send_messages_client()
{
    while(!server_started) { /* wait for server to start */ }

    SocketLib::SocketClient client;

    client.start();
    std::cout << client.status() << '\n';

    client_started = true;  

	if (!client.running())
		return;

	client.send_text("first message from client");
    auto response = client.receive_text();
	print_line("Server responded with: " + response);

    client.send_text("second message from client");
    response = client.receive_text();
	print_line("Server responded with: " + response);

    client.send_text("goodbye");
    client.stop();
}


void test_send_messages()
{
	server_started = false;
	client_started = false;

	print_line("test_send_messages()");
	std::thread ts(send_messages_server);
	std::thread tc(send_messages_client);

	while (!server_started || !client_started) { /* wait for both processes to start */ }

	ts.join();
	tc.join();

	print_line();
}


//======= CALLBACK MAP ===================

std::unordered_map <std::string, std::function<void(std::string const&)>> callbacks
{
	{ "print", [](std::string const& str) { print_line(str); } }
};


std::vector<std::string> to_args(std::string const& line)
{
	std::istringstream iss(line);
	std::istream_iterator<std::string> beg(iss), end;

	std::vector<std::string> args(beg, end);

	return args;
}


void callback_map_server()
{

}


void callback_map_client()
{

}


void test_callback_map()
{
	server_started = false;
	client_started = false;
}


//===========================================


void launch_test()
{
	test_send_messages();
}