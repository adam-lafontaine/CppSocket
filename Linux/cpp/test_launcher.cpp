#include "../hpp/SocketServer.hpp"
#include "../hpp/SocketClient.hpp"

#include <iostream>
#include <thread>
#include <mutex>


// g++ -o tester test_launcher.cpp -std=c++17
// ./tester

void run_server();
void run_client();

int main(int argc, char *argv[])
{
    /*
    SocketLib::SocketServer server;
    std::cout << "server created" << '\n';

    SocketLib::SocketClient client;
    std::cout << "client created" << '\n';

    const auto svr = [&]()
    {
        server.start();
        std::cout << server.status() << '\n';

        server.connect_client();
        std::cout << server.status() << '\n';

        server.stop();
        std::cout << server.status() << '\n';
    };

    const auto cli = [&]()
    {
        client.start();
        std::cout << client.status() << '\n';

        client.stop();
        std::cout << client.status() << '\n';
    };
    */

    std::thread ts(run_server);
    std::thread tc(run_client);
    

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


void run_server()
{
    print_line("run_server()");
}


void run_client()
{
    print_line("run_client()");
}