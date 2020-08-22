//#define DO_CLIENT_SERVER
//#define DO_CLIENT
#define DO_SERVER

#ifdef DO_CLIENT_SERVER
#include "../hpp/launchers/client_server_threaded.hpp"
#endif // DO_CLIENT_SERVER

#ifdef DO_CLIENT
#include "../hpp/launchers/client_launcher.hpp"
#endif // DO_CLIENT

#ifdef DO_SERVER
#include "../hpp/launchers/server_launcher.hpp"
#endif // DO_SERVER



int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
	int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;   // check block integrity
	dbgFlags |= _CRTDBG_DELAY_FREE_MEM_DF; // don't recycle memory
	dbgFlags |= _CRTDBG_LEAK_CHECK_DF;     // leak report on exit
	_CrtSetDbgFlag(dbgFlags);
#endif


#ifdef DO_CLIENT_SERVER

	client_server_threaded();

#endif // DO_CLIENT_SERVER

#ifdef DO_CLIENT
#ifdef _DEBUG

	char app[] = "client";
	char ip[] = "10.0.0.214";
	char port[] = "27015";
	char* new_args[] = { app, ip, port };
	launch_client(3, new_args);

#else

	launch_client(argc, argv);

#endif // _DEBUG
#endif // DO_CLIENT

#ifdef DO_SERVER
#ifdef _DEBUG

	char app[] = "server";
	char port[] = "27015";
	char* new_args[] = { app, port };

	launch_server(2, new_args);

#else

	launch_server(argc, argv);

#endif // _DEBUG
#endif // DO_SERVER

}