#include <iostream>

#include "../hpp/SocketServer.hpp"
#include "../hpp/SocketClient.hpp"

using namespace std;


// g++ -o tester test_launcher.cpp
// ./tester

int main(int argc, char *argv[])
{
    SocketLib::SocketServer server;
    cout << "server created" << endl;
    server.start();
    cout << server.status() << endl;
    server.connect_client();
    cout << server.status() << endl;

    SocketLib::SocketClient client;
    client.start();
    cout << client.status() << endl;
    
    server.stop();
    cout << server.status() << endl;

    client.stop();
    cout << client.status() << endl;

    
}