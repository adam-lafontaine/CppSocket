#include "../hpp/SocketServer.hpp"
#include "../hpp/SocketClient.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <cassert>
#include <algorithm>
#include <string>


// g++ -o tester test_launcher.cpp SocketServer.cpp -std=c++17 -lpthread
// ./tester

void run_server();
void run_client();

bool server_started = false;
bool client_started = false;

int main(int argc, char* argv[])
{
    std::thread ts(run_server);
    std::thread tc(run_client);

    while(!server_started || !client_started) { /* wait for both processes to start */ }    

    ts.join();
    tc.join();
}

//==============================

std::mutex console_mtx;
void print_line(std::string const& msg)
{
	std::lock_guard<std::mutex> lk(console_mtx);

	std::cout << msg << '\n';
}


bool end_session_msg(std::string const& msg)
{
	auto copy = msg;
	std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);

	return copy == "goodbye";
}


std::string process_client_message(std::string const& msg)
{
	auto rev = msg;
	std::reverse(rev.begin(), rev.end());

	std::string response = "The message backwards is: '" + rev + "'";

	return response;
}


void run_server()
{
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
		
		const auto response = process_client_message(msg);
		server.send_text(response);

		if (end_session_msg(msg)) {
			print_line("Ending server session");
			server.disconnect_client();
			server.stop();
		}		
	}

    assert(!server.running());
	assert(!server.connected());

	print_line(server.status());
}


void run_client()
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