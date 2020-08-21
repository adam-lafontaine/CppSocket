#include <iostream>

#include "../hpp/SocketClient.hpp"


// g++ -o client client_launcher.cpp -std=c++17
// ./client

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " " << "IP port" << '\n';
        return 0;
    }

    SocketLib::SocketClient client(argv[1], atoi(argv[2]));
    client.start();

    if(client.has_error())
    {
        std::cout << client.latest_error() << '\n';
        return 0;
    }

    std::cout << client.status() << '\n';

    // handle user input
	std::string text;
	while (client.running())
    {
		std::getline(std::cin, text);
		client.send_text(text);

		if (text == "END")
        {
			client.stop();
		}
	}

    std::cout << client.status() << '\n';
}