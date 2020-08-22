#include "../hpp/SocketServer.hpp"

#include <iostream>


// g++ -o server server_launcher.cpp SocketServer.cpp -std=c++17
// ./server

int main(int argc, char* argv[])
{
    SocketLib::SocketServer server;

    if(argc > 1)
    {
        auto port_no = atoi(argv[1]);
        server.set_port(port_no);
    }
    
    std::cout << "server created" << '\n';

    server.start();
    if(server.has_error())
    {
        std::cout << server.latest_error() << '\n';
        return 0;
    }

    std::cout << server.status() << '\n';

    // wait for client to connect
    server.connect_client();
    if(server.has_error())
    {
        std::cout << server.latest_error() << '\n';
        return 0;
    }

    std::cout << server.status() << '\n';

    while(server.running() && server.connected())
    {
        std::string msg = server.receive_text();        
        std::cout << "Client says: " << msg << '\n';

        if(msg == "END")
        {
            server.disconnect_client();
			server.stop();
        }

    }

    std::cout << server.status() << '\n';

    
}