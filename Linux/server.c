/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
   perror(msg);
   exit(1);
}

int main(int argc, char *argv[])
{
   int sock_fd;     // file descriptor stores return of socket()
   int newsock_fd;  // file descriptor stores return of socket()

   int port_no;  // port number

   socklen_t client_len;  // size of address on client

   char buffer[256];  // characters are read into this buffer

   struct sockaddr_in serv_addr;  // contains server address
   struct sockaddr_in cli_addr;   // contains client address

   int n_chars;  // number of characters read or written

   // validate command line args
   if (argc < 2) {
      fprintf(stderr,"ERROR, no port provided\n");
      exit(1);
   }

   sock_fd = socket(AF_INET, SOCK_STREAM, 0);  // create socket

   if (sock_fd < 0) 
      error("ERROR opening socket");

   bzero((char *) &serv_addr, sizeof(serv_addr)); // initialize serv_addr to zeros

   port_no = atoi(argv[1]); // set port number from char[]

   serv_addr.sin_family = AF_INET;         // always
   serv_addr.sin_addr.s_addr = INADDR_ANY; // IP address of server machine
   serv_addr.sin_port = htons(port_no);    // convert from host to network byte order

   // bind socket to server address
   int bind_res = bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

   if (bind_res < 0) 
            error("ERROR on binding");

   listen(sock_fd, 5);

   client_len = sizeof(cli_addr);

   // waits for client to connect
   printf("Waiting for client to connect...\n");
   newsock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &client_len);

   if (newsock_fd < 0) 
         error("ERROR on accept");

   bzero(buffer, 256); // initialize buffer to zeros

   // waits for message from client to be read to buffer
   printf("Waiting for message from client...\n");
   n_chars = read(newsock_fd, buffer, 255);

   if (n_chars < 0) 
      error("ERROR reading from socket");

   printf("Here is the message: %s\n", buffer);

   // send message to client
   n_chars = write(newsock_fd, "I got your message", 18);

   if (n_chars < 0) 
      error("ERROR writing to socket");

   close(newsock_fd);
   close(sock_fd);
   return 0; 
}