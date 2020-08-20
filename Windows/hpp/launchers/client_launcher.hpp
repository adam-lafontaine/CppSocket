#pragma once
#include <iostream>

#include "../SocketClient.hpp"

void launch_client(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " " << "hostname port" << '\n';
        return;
    }

    MySocketLib::SocketClient client(argv[1], atoi(argv[2]));
    client.start();

    if (client.has_error())
    {
        std::cout << client.latest_error() << '\n';
        return;
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