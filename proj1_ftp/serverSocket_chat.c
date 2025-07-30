#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error (const char *msg)
{
    perror(msg);
    exit(1);
}

int main (int argc, char *argv[])
{
    // Print error msg to show how to use
    if(argc < 2)
    {
        fprintf(stderr, "The program needs 2 parameters, the File name & the Port. Program terminated\n");
        exit(1);
    }

    // Declare some variables
    int SocketFD, NewSocketFD, PortNum, n;
    char buffer[256];

    struct sockaddr_in server_address, client_address;
    socklen_t client_len;

    // Creating the socket and return with the socket file descriptor
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(SocketFD < 0)
    {
        error("Error Creating Socket.");
    }

    // Ensure that all the bytes of server_address is 0
    bzero((char *) &server_address, sizeof(server_address));

    // Define the server_address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    PortNum = atoi(argv[1]);
    server_address.sin_port = htons(PortNum); // Host to Network Short

    // Binding the socketFD with the IP address & PortNum
    if( bind(SocketFD, (struct sockaddr *) &server_address, sizeof(server_address) ) < 0 )
    {
        error("Binding Failed.");
    }

    // listen to the SocketFD file with 5 clients
    listen(SocketFD, 5);

    client_len = sizeof(client_address);

    // Accept the connection requested by client_address
    NewSocketFD = accept(SocketFD, (struct sockaddr *) &client_address, &client_len);
    if(NewSocketFD < 0)
    {
        error("Error in Accepting.");
    }

    // Reading the file sent from the client to the NewSocketFD
    while(true)
    {
        bzero(buffer, 256);
        n = read(NewSocketFD, buffer, 256);
        if(n < 0) error("Error in Reading.");
        printf("Client: %s", buffer);

        bzero(buffer, 256);
        fgets(buffer, 256, stdin);
        n = write(NewSocketFD, buffer, strlen(buffer));
        if(n < 0) error("Error  in Writing.");

        // char *p = buffer;
        // for( ; *p; p++) *p = tolower(*p);
        if(strncmp("bye", buffer, 3) == 0) break;
    }

    close(NewSocketFD);
    close(SocketFD);
    return 0;

    
}