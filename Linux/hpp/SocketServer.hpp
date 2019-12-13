#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<arpa/inet.h>

#include<ifaddrs.h>

#include<vector>
#include<sstream>
#include<cassert>

using namespace std;

namespace SocketLib {    

    class SocketServer {
    private:
        unsigned short const DEFAULT_PORT = 27015;
        static int constexpr MAX_CHARS = 256;
        
        bool _running = false;
        bool _open = false;
        bool _connected = false;
        std::string _status = "";

        string _net_interface = "NA";
        string _public_ip = "NA";

        unsigned short _port_no;
        int _srv_socket; // server socket file descriptor
        int _cli_socket; // client socket file descriptor

        struct sockaddr_in _serv_addr; // contains server address
        struct sockaddr_in _cli_addr;  // contains client address
        socklen_t _cli_len; // size of address on client

        void get_network_info();

        bool init();
        bool bind_socket();
        bool listen_socket();
        void close_socket();

        std::vector<std::string> _errors;

        string system_error(string const& msg); 
        
    public:
        SocketServer() { 
            _port_no = DEFAULT_PORT;
            get_network_info();
        }

        SocketServer(unsigned short port){ 
            _port_no = port;
            get_network_info();
        }

        ~SocketServer() {
            disconnect_client();
            close_socket();
        }

        void set_port(unsigned port){ _port_no = port; }

        void start();
        void stop();
        bool connect_client();
        void disconnect_client();

        std::string receive_text();
        bool send_text(std::string const& text);        

        std::string status() { return _status; }
        bool running() { return _running; }
        bool connected() { return _connected; }

        bool has_error() { return !_errors.empty(); }
        std::string latest_error();
    };

    //====================================================

    bool SocketServer::init() {

        _srv_socket = socket(AF_INET, SOCK_STREAM, 0); // create socket

        if (_srv_socket < 0) {
            _status = "ERROR opening socket";
            _errors.push_back(system_error(_status));
            return false;
        }

        _open = true;

        bzero((char *)&_serv_addr, sizeof(_serv_addr)); // initialize serv_addr to zeros        

        _serv_addr.sin_family = AF_INET;          // always
        _serv_addr.sin_addr.s_addr = INADDR_ANY;  // IP address of server machine
        _serv_addr.sin_port = htons(_port_no);    // convert from host to network byte order

        _status = "Server Initialized";
        return true;
    }

    bool SocketServer::bind_socket() {

        // bind socket to server address
        int bind_res = bind(_srv_socket, (struct sockaddr*) &_serv_addr, sizeof(_serv_addr));

        if (bind_res < 0) {
            _status = "ERROR binding socket";
            _errors.push_back(system_error(_status));
            return false;
        }

        _status = "Socket binded";

        return true;   
    }

    bool SocketServer::listen_socket() {

        int res = listen(_srv_socket, 5);
        if(res < 0) {
            _status = "ERROR listening on port " + to_string(_port_no);
            _errors.push_back(system_error(_status));
            return false;
        }

        _status =  "Listening on " + _public_ip + " : " + to_string(_port_no);
        return true;
    }

    void SocketServer::close_socket() {
        if(!_open)
            return;

        close(_srv_socket);
        
        _open = false;
    }

    void SocketServer::start() {
        _running = false;

		if (!init()) {
            close_socket();
            return;
        }			

		if (!bind_socket()) {
            close_socket();
            return;
        }

		if (!listen_socket()) {
            close_socket();
            return;
        }

		_running = true;
    }

    void SocketServer::stop() {
		disconnect_client();

		_running = false;
		close_socket();
		_status = "Server stopped";
	}

    bool SocketServer::connect_client() {
        if (!_running) {
			_status = "Server cannot connect, server not running";
			return false;
		}

        _cli_len = sizeof(_cli_addr);

        _status = "Server waiting for client";
		_connected = false;

        // waits for client to connect        
        _cli_socket = accept(_srv_socket, (struct sockaddr *) &_cli_addr, &_cli_len);

        if (_cli_socket < 0) {
            _status = "ERROR on accept";
            _errors.push_back(system_error(_status));
            return false;
        }

        _status = "Server connected to client";
		_connected = true;

		return true;
    }

    void SocketServer::disconnect_client() {
		if (!_connected)
            return;
        
        close(_cli_socket);

        _status = "Server disonnected from client";

        _connected = false;
	}

    std::string SocketServer::receive_text() {
		assert(_running);
		assert(_connected);

		char buffer[MAX_CHARS]; // characters are read into this buffer
        int n_chars; // number of characters read or written

		bool waiting = true;
		ostringstream oss;

		bzero(buffer, MAX_CHARS); // initialize buffer to zeros

        // waits for message from client to be read to buffer
        //printf("Waiting for message from client...\n");
        n_chars = read(_cli_socket, buffer, MAX_CHARS - 1);

        if (n_chars < 0) {
            _status = "ERROR reading from client socket";
            _errors.push_back(system_error(_status));
            return "error";
        }
        
        oss << buffer;

		return oss.str();
	}

    bool SocketServer::send_text(std::string const& text) {
		assert(_running);
		assert(_connected);
		if (!_running || !_connected)
			return false;

		vector<char> message(text.begin(), text.end());

        // send message to client
        int n_chars = write(_cli_socket, message.data(), static_cast<int>(message.size()));

        if (n_chars < 0) {
            _status = "ERROR writing to client socket";
            _errors.push_back(system_error(_status));
            return false;
        }

        return true;
	}

    std::string SocketServer::latest_error() {

        std::string delim = ", ";

        ostringstream oss;
        for(const std::string& err : _errors) {
            oss << err << delim;
        }

		std::string msg = oss.str();
		msg.pop_back();
		msg.pop_back();

        _errors.clear();

		return msg;
    }

    string SocketServer::system_error(string const& msg) {

		ostringstream oss;
		oss << msg << ": " << strerror(errno);

		return oss.str();
	}

    void SocketServer::get_network_info() {
        //https://www.binarytides.com/get-local-ip-c-linux/

        FILE *f;
        char line[100] , *p , *c;
        
        f = fopen("/proc/net/route" , "r");
        
        while(fgets(line , 100 , f)) {

            p = strtok(line , " \t");
            c = strtok(NULL , " \t");
            
            if(p!=NULL && c!=NULL) {
                if(strcmp(c , "00000000") == 0) {
                    _net_interface = string(p);                    
                    break;
                }
            }
        }
        
        //which family do we require , AF_INET or AF_INET6
        int fm = AF_INET; //AF_INET6
        struct ifaddrs *ifaddr, *ifa;
        int family , s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
            _public_ip = "error";
            return;
        }

        //Walk through linked list, maintaining head pointer so we can free list later
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL)
                continue;

            family = ifa->ifa_addr->sa_family;
            if(strcmp( ifa->ifa_name , p) != 0)
                continue;

            if(family != fm)
                continue;

            auto family_size = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            
            s = getnameinfo( ifa->ifa_addr, family_size , host , NI_MAXHOST , NULL , 0 , NI_NUMERICHOST);
                
            if (s != 0) {
                _public_ip = "error";
                return;
            }
            
            _public_ip = string(host);
        }

        freeifaddrs(ifaddr);
    }
}