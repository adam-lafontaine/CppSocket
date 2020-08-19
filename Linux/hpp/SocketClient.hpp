#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include<vector>
#include<sstream>
#include<cassert>

namespace SocketLib
{
    class SocketClient
	{
    private:
		unsigned short const DEFAULT_PORT = 27015;
        static int constexpr MAX_CHARS = 256;
        const char* DEFAULT_HOST = "localhost";
		
        const char* _srv_hostname;

        unsigned short _srv_port_no;
        int _socket; // socket file descriptor

        struct sockaddr_in _srv_addr; // contains server address
        struct hostent* _srv_ptr;   // pointer to server info

		bool _running = false;
		bool _open = false;
		std::string _status = "";

        bool init();
		bool connect_socket();		
		void close_socket();

		std::string system_error(std::string const& msg);

		std::vector<std::string> _errors;
		

	public:
		SocketClient()
		{ 
			_srv_hostname = DEFAULT_HOST;
			_srv_port_no = DEFAULT_PORT;
		 }

		SocketClient(std::string const& srv_hostname, unsigned short srv_port)
		{
			_srv_hostname = srv_hostname.c_str();
			_srv_port_no = srv_port;
		}

		SocketClient(const char* srv_hostname, unsigned short srv_port)
		{
			_srv_hostname = srv_hostname;
			_srv_port_no = srv_port;
		}

		~SocketClient() { close_socket(); }

		void start();
		void stop();
		bool send_text(std::string const& text);
		std::string receive_text();
		std::string status() { return _status; }
		bool running() { return _running; }

		bool has_error() { return !_errors.empty(); }
        std::string latest_error();

    };

    //========================================

    bool SocketClient::init()
	{
        _socket = socket(AF_INET, SOCK_STREAM, 0);  // create socket

        if (_socket < 0)
		{
            _status = "ERROR opening socket";
			_errors.push_back(system_error(_status));
            return false;
        }		

        _srv_ptr = gethostbyname(_srv_hostname);

        if(_srv_ptr == NULL)
		{
            _status = "ERROR host not found";
			_errors.push_back(system_error(_status));
            return false;
        }

        // populate serv_addr
        bzero((char *) &_srv_addr, sizeof(_srv_addr)); // initialize to zeros

        _srv_addr.sin_family = AF_INET;    

        // copy server address bytes to serv_addr
        bcopy((char *)_srv_ptr->h_addr_list[0], (char *)&_srv_addr.sin_addr.s_addr, _srv_ptr->h_length);
        // #define h_addr  h_addr_list[0]  /* address, for backward compatiblity */		

        _srv_addr.sin_port = htons(_srv_port_no);            

        _open = true;

		_status = "Client Initialized";

        return true;
    }

    bool SocketClient::connect_socket()
	{
        if (!_open)
		{
			_status = "Client not initialized.";
			return false;
		}

        // connect the socket
        int res = connect(_socket,(struct sockaddr *) &_srv_addr,sizeof(_srv_addr));		
		
		if (res < 0)
		{
			_status = "ERROR Client connect failed";
			_errors.push_back(system_error(_status));
			close_socket();
			return false;
		}

		return true;
    }

    void SocketClient::close_socket()
	{
        if (!_open)
			return;

		close(_socket);
		_open = false;
    }

    void SocketClient::start()
	{
        if (!init() || !connect_socket())
			return;

		_running = true;
		_status = "Client started";
    }

    void SocketClient::stop()
	{
		_running = false;
		close_socket();
		_status = "Client stopped";
	}

    bool SocketClient::send_text(std::string const& text)
	{
		assert(_running);
		if (!_running)
			return false;

		std::vector<char> message(text.begin(), text.end());       

        int n_chars = write(_socket, message.data(), static_cast<int>(message.size()));

        if (n_chars < 0)
		{
            _status = "ERROR writing to socket";
			_errors.push_back(system_error(_status));
            return false;
        }

        return true;
    }

    std::string SocketClient::receive_text()
	{
		assert(_running);

		char buffer[MAX_CHARS]; // characters are read into this buffer
        int n_chars; // number of characters read or written

        bool waiting = true;
		std::ostringstream oss;

        bzero(buffer, MAX_CHARS);        

		while (waiting)
		{
			n_chars = read(_socket, buffer, MAX_CHARS - 1);
			if (n_chars > 0)
			{
				waiting = false;
				oss << buffer;
			}
		}

		return oss.str();
	}

	std::string SocketClient::latest_error()
	{
        std::string delim = ", ";

        std::ostringstream oss;
        for(const std::string& err : _errors)
		{
            oss << err << delim;
        }

		std::string msg = oss.str();
		msg.pop_back();
		msg.pop_back();

		_errors.clear();

		return msg;
    }

	std::string SocketClient::system_error(std::string const& msg)
	{
		std::ostringstream oss;
		oss << msg << ": " << strerror(errno);

		return oss.str();
	}
}

/*
http://www.linuxhowtos.org/C_C++/socket.htm

http://www.linuxhowtos.org/data/6/connect.txt




*/