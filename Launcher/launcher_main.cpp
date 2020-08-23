//#define DO_SERVER
#define DO_CLIENT

#ifdef DO_SERVER
#include "server_launcher.hpp"
#endif

#ifdef DO_CLIENT
#include "client_launcher.hpp"
#endif

// g++ -o launcher launcher_main.cpp ../Server/SocketServer.cpp ../Client/SocketClient.cpp -std=c++17

int main(int argc, char* argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;   // check block integrity
dbgFlags |= _CRTDBG_DELAY_FREE_MEM_DF; // don't recycle memory
dbgFlags |= _CRTDBG_LEAK_CHECK_DF;     // leak report on exit
_CrtSetDbgFlag(dbgFlags);
#endif


#ifdef DO_SERVER

    launch_server();

#endif

#ifdef DO_CLIENT

    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " " << "IP port" << '\n';
        return 0;
    }

    launch_client(argv[1], atoi(argv[2]));

#endif



}