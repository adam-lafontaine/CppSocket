#include "../hpp/SocketServer.hpp"
#include "../hpp/SocketClient.hpp"

#include <iostream>
#include <thread>


// g++ -o tester test_launcher.cpp
// ./tester

int main(int argc, char *argv[])
{
    SocketLib::SocketServer server;
    std::cout << "server created" << '\n';

    SocketLib::SocketClient client;
    std::cout << "client created" << '\n';

    server.start();
    std::cout << server.status() << '\n';

    server.connect_client();
    std::cout << server.status() << '\n';
    
    client.start();
    std::cout << client.status() << '\n';
    
    server.stop();
    std::cout << server.status() << '\n';

    client.stop();
    std::cout << client.status() << '\n';

    
}