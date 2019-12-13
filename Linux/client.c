#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// http://www.linuxhowtos.org/C_C++/socket.htm
// gcc -o my_client client.c
// ./my_client

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd; // file descriptor stores return of socket()
    int port_no; // port number

    int n_chars; // number of characters read or written

    struct sockaddr_in serv_addr; // contains server address
    struct hostent* server_ptr;   // pointer to server info

    char buffer[256];

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    port_no = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // create socket

    if (sockfd < 0) 
        error("ERROR opening socket");

    server_ptr = gethostbyname(argv[1]);  

    if (server_ptr == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // populate serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr)); // initialize to zeros

    serv_addr.sin_family = AF_INET;    

    // copy server address bytes to serv_addr
    //bcopy((char *)server_ptr->h_addr, (char *)&serv_addr.sin_addr.s_addr, server_ptr->h_length);
    bcopy((char *)server_ptr->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server_ptr->h_length);
    // #define h_addr  h_addr_list[0]  /* address, for backward compatiblity */

    serv_addr.sin_port = htons(port_no);

    // connect to server
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    printf("Please enter the message: ");

    bzero(buffer,256);
    fgets(buffer,255,stdin);

    n_chars = write(sockfd,buffer,strlen(buffer));

    if (n_chars < 0) 
         error("ERROR writing to socket");

    bzero(buffer,256);

    n_chars = read(sockfd,buffer,255);

    if (n_chars < 0) 
         error("ERROR reading from socket");

    printf("%s\n",buffer);
    close(sockfd);

    return 0;
}