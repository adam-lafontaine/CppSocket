#include <iostream>

#include "../hpp/SocketClient.hpp"

using namespace std;


// g++ -o client client_launcher.cpp
// ./client

int main(int argc, char *argv[])
{
    if(argc < 3) {
        cout << "Usage: " << argv[0] << " " << "hostname port" << endl;
        return 0;
    }

    SocketLib::SocketClient client(argv[1], atoi(argv[2]));
    client.start();

    if(client.has_error()){
        cout << client.latest_error() << endl;
        return 0;
    }

    cout << client.status() << endl;

    // handle user input
	string text;
	while (client.running()) {
		getline(cin, text);
		client.send_text(text);

		if (text == "END") {
			client.stop();
		}
		
		
	}

    cout << client.status() << endl;

    
}