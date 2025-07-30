/*
./client.out "filename" "ServerIP" "PortNum"
              argv[0]     argv[1]     argv[2]
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error (const char *msg)
{
    perror(msg);
    exit(2);
}

int main (int argc, char *argv[])
{
    // Print error msg to show how to use
    if(argc < 3)
    {
        fprintf(stderr, "The program needs 3 parameters, FileName, ServerIP & PortNum. Program terminated.\n");
        exit(2);
    }

    // Declare some variables
    int SocketFD, PortNum, n;
    char buffer[256];
    struct sockaddr_in server_address;

    // Creating the socket and return with the socket file descriptor
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(SocketFD < 0)
    {
        error("Error Creating Socket.");
    }

    // Ensure that all the bytes of server_address is 0
    bzero((char *) &server_address, sizeof(server_address));

    // Define the server_address structure
        // hostent fun to store info about the host like kost name and the IPv4 protocol
        struct hostent *host_server;
        host_server = gethostbyname(argv[1]);
        if(host_server == NULL) fprintf (stderr, "Please, enter a right host!");

    server_address.sin_family = AF_INET;
    bcopy( (char *) host_server->h_addr, (char *) &server_address.sin_addr.s_addr, host_server->h_length );
    PortNum = atoi(argv[2]);
    server_address.sin_port = htons(PortNum); // Host to Network Short

    // creating connect function
    if( connect(SocketFD, (struct sockaddr *) &server_address, sizeof(server_address)) < 0 )
    {
        error("Connection Failed");
    }

    // Writing the file to the SocketFD 
    // char *p = buffer;
    while(true)
    {
        bzero(buffer, 256);
        fgets(buffer, 256, stdin);
        n = write(SocketFD, buffer, strlen(buffer));
        if(n < 0) error("Error in Writing");

        bzero(buffer, 256);
        n = read(SocketFD, buffer, 256);
        if(n < 0) error("Error in Reading");
        printf("Server: %s", buffer);

        // for( ; *p; p++) *p = tolower(*p);
        if(strncmp("bye", buffer, 3) == 0) break;
    }

    close(SocketFD);
    return 0;

    
}