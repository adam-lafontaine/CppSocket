//#include "../hpp/launchers/client_server_threaded.hpp"
#include "../hpp/launchers/client_launcher.hpp"
//#include "../hpp/launchers/server_launcher.hpp"

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
	int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;   // check block integrity
	dbgFlags |= _CRTDBG_DELAY_FREE_MEM_DF; // don't recycle memory
	dbgFlags |= _CRTDBG_LEAK_CHECK_DF;     // leak report on exit
	_CrtSetDbgFlag(dbgFlags);
#endif


	//client_server_threaded();

	//launch_client(argc, argv);

	char app[] = "client";
	char ip[] = "10.0.0.214";
	char port[] = "27015";
	char* new_args[] = { app, ip, port };
	launch_client(3, new_args);

	//launch_server(argc, argv);
}