#include <iostream>

#include "../hpp/SocketServer.hpp"

using namespace std;


// g++ -o server server_launcher.cpp
// ./server

int main(int argc, char *argv[])
{
    SocketLib::SocketServer server;

    if(argc > 1) {
        auto port_no = atoi(argv[1]);
        server.set_port(port_no);
    }
    
    cout << "server created" << endl;

    server.start();
    if(server.has_error()) {
        cout << server.latest_error() << endl;
        return 0;
    }

    cout << server.status() << endl;

    // wait for client to connect
    server.connect_client();
    if(server.has_error()) {
        cout << server.latest_error() << endl;
        return 0;
    }

    cout << server.status() << endl;

    while(server.running() && server.connected()) {
        string msg = server.receive_text();        
        cout<< "Client says: " << msg << endl;

        if(msg == "END"){
            server.disconnect_client();
			server.stop();
        }

    }

    cout << server.status() << endl;

    
}