#include "../Client/SocketClient.hpp"

#include <iostream>

void launch_client(const char* srv_ip, unsigned short srv_port)
{
    SocketLib::SocketClient client(srv_ip, srv_port);
    client.start();

    if(client.has_error())
    {
        std::cout << client.latest_error() << '\n';
        return;
    }

    std::cout << client.status() << '\n';

    // handle user input
	std::string text;
	while (client.connected())
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