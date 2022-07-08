#include "../Server/SocketServer.hpp"

#include <iostream>

void launch_server()
{
    auto port = 22222;

    SocketLib::SocketServer server(port);

    std::cout << "server created" << '\n';

    server.start();
    if(server.has_error())
    {
        std::cout << server.latest_error() << '\n';
        return;
    }

    std::cout << server.status() << '\n';

    // wait for client to connect
    server.connect_client();
    if(server.has_error())
    {
        std::cout << server.latest_error() << '\n';
        return;
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