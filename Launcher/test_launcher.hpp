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
	print_line(client.status());

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

using arg_t = std::string;
using arg_list_t = std::vector<arg_t>;

std::string blank(arg_list_t const& args)
{
	std::string str = "NA";
	return str;
}

std::string echo(arg_list_t const& args)
{
	std::string str = "";

	for (auto const& arg : args)
	{
		str += arg;
		str += ' ';
	}

	str.pop_back();

	return str;
}

std::unordered_map <std::string, std::function<std::string(arg_list_t const&)>> callbacks
{
	{ "blank", blank },
	{ "echo", echo },
	{ "end", [](arg_list_t const&) { return "goodbye"; } },
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
	const auto process_msg = [](std::string const& msg)
	{
		const auto args = to_args(msg);
		const auto command = args[0];

		auto it = callbacks.find(command);

		return it == callbacks.end() ? "not found" : (it->second)(args);
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

		if (end_session_msg(response))
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


std::string is_expected(std::string const& a, std::string const& b)
{
	return a == b ? "OK" : "Fail";
}

void callback_map_client()
{
	while (!server_started) { /* wait for server to start */ }

	SocketLib::SocketClient client;

	client.start();
	print_line(client.status());

	client_started = true;

	if (!client.running())
		return;
	
	client.send_text("blank message from client");
	auto response = client.receive_text();
	std::string expected = "NA";
	print_line("Server responded with: " + response + ": " + is_expected(expected, response));

	client.send_text("echo message from client");
	response = client.receive_text();
	expected = "echo message from client";
	print_line("Server responded with: " + response + ": " + is_expected(expected, response));

	client.send_text("do something else");
	response = client.receive_text();
	expected = "not found";
	print_line("Server responded with: " + response + ": " + is_expected(expected, response));

	client.send_text("end");
	response = client.receive_text();
	expected = "goodbye";
	print_line("Server responded with: " + response + ": " + is_expected(expected, response));
	client.stop();
}


void test_callback_map()
{
	server_started = false;
	client_started = false;

	print_line("test_callback_map()");
	std::thread ts(callback_map_server);
	std::thread tc(callback_map_client);

	while (!server_started || !client_started) { /* wait for both processes to start */ }

	ts.join();
	tc.join();

	print_line();
}


//===========================================


void launch_test()
{
	test_send_messages();
	test_callback_map();
}