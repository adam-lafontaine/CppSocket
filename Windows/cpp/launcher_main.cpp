/*
*    Authors: Adam Lafontaine, Yougui Chen 
*     Course: INFO 5104
* Assignment: Project 3, Socket Library
*       Date: January 9, 2018
*
*       File: launcher_main.cpp
*/

#include <conio.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <cassert>
#include "../hpp/SocketServer.hpp"
#include "../hpp/SocketClient.hpp"

namespace msl = MySocketLib;

using namespace std;

mutex console_mtx;
bool server_running = false;
bool client_running = false;


/*
*        Purpose: Prints a line to the console in a thread-safe manner
*     Parameters: The message to print
*  Preconditions: None
* Postconditions: None
*/
void print_line(string const& msg) {
	lock_guard<mutex> lk(console_mtx);

	cout << msg << endl;
}

/*
*        Purpose: Return a string containing the passed string reversed
*                 Used to demonstrate processing a message received by a client
*     Parameters: The string to reverse
*  Preconditions: None
* Postconditions: None
*/
string process_client_message(string const& msg) {

	string rev = msg;
	reverse(rev.begin(), rev.end());

	string response = "The message backwards is: '" + rev + "'";

	return response;
}

/*
*        Purpose: Indicates whether or not to end a client-server session
*     Parameters: The message from the client
*  Preconditions: Both the client and the server are running
* Postconditions: If returns true, both the client and the server will be stopped
*/
bool end_session_msg(string const& msg) {
	string copy = msg;
	transform(copy.begin(), copy.end(), copy.begin(), ::tolower);

	return copy == "goodbye";
}

/*
*        Purpose: Creates and starts a SocketServer object and has it
*                 listen for messages from the client
*     Parameters: None
*  Preconditions: None
* Postconditions: Server is stopped when the termination message is 
*                 received from the client
*/
void start_server() {

	msl::SocketServer server("127.0.0.1");

	server.start();
	print_line(server.status());
	server.connect_client();
	print_line(server.status());

	server_running = server.running() && server.connected();

	// handle input from client
	while (server.running() && server.connected()) {
		string msg = server.receive_text();
				
		string response = process_client_message(msg);
		server.send_text(response);

		if (end_session_msg(msg)) {
			print_line("Ending server session");
			server.disconnect_client();
			server.stop();
		}		
	}
	print_line(server.status());
	assert(!server.running());
	assert(!server.connected());
	server_running = server.running() && server.connected();
}

/*
*        Purpose: Creates and starts a SocketClient object and reads input
*                 from the console and sends it to server then displays the
*                 server's response
*     Parameters: None
*  Preconditions: None
* Postconditions: Client is stopped when the user enters the end message
*/
void start_client() {
	msl::SocketClient client("127.0.0.1");
	client.start();
	print_line(client.status());

	if (!client.running())
		return;

	client_running = client.running();
	
	// handle user input
	string text;
	while (client.running()) {
		getline(cin, text);
		client.send_text(text);

		if (end_session_msg(text)) {
			client.stop();
		}
		else {
			string response = client.receive_text();
			print_line("Server responded with: " + response);
		}
	}

	print_line(client.status());

	assert(!client.running());

	client_running = client.running();	
}

/*
*        Purpose: Starts a server and client in different threads and waits
*                 waits until they are completed
*     Parameters: None
*  Preconditions: None
* Postconditions: None
*/
int main(int argc, char *argv[]) {	

	print_line("Launcher...\nHit any key to start server and client");
	_getch();
	thread ts(start_server); 
	thread tc(start_client);

	while(!server_running || !client_running){ /* wait for both processes to start */}
	print_line("\nEnter text to send to server");

	tc.join();
	ts.join();	

	_getch();
}