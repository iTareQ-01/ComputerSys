/*
Usage: Program_Name     Port_Num
        argv[0]         argv[1]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>

void error (const char *msg)
{
    perror(msg);
    exit(2);
}

int main (int argc, char *argv[])
{
    // Print error msg to show how to use
    if(argc < 2)
    {
        fprintf(stderr, "[-]The program needs 2 parameters, the Program_Name, & the Port_Num. \nProgram terminated\n");
        exit(1);
    }

    // Declare some variables
    int SocketFD, NewSocketFD, PortNum;
    char buffer[256];

    struct sockaddr_in server_address, client_address;
    socklen_t client_len;

    // Creating the socket and return with the socket file descriptor
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(SocketFD < 0)
    {
        error("[-]Error Creating Socket.");
    } else 
    {
        printf("[+]Server Socket Created.\n");
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
        error("[-]Binding Failed.");
    } else 
    {
        printf("[+]Binding Successful.\n");
    }

    // listen to the SocketFD file with 5 clients
    if (listen(SocketFD, 5) == 0)
    {
        printf("[+]Listening...\n");
    } else
    {
        error("[-]Listening Failed.");
    }

    client_len = sizeof(client_address);

    // Accept the connection requested by client_address
    NewSocketFD = accept(SocketFD, (struct sockaddr *) &client_address, &client_len);
    if(NewSocketFD < 0)
    {
        error("[-]Error in Accepting.");
    }

    // Reading the file sent from the client to the NewSocketFD
    int n;
    int file_txt = open("recieved.txt", O_CREAT|O_WRONLY|O_TRUNC, 0642);
    while(true)
    {
        bzero(buffer, 256);
        n = read(NewSocketFD, buffer, 255);
        if (n < 0)
        {
            error("[-]Error in Reading the file");
        }
        else if (n == 0)
        {
            break;
        }
        write(file_txt, buffer, n);
    }

    // Msg to the screen if the file sent with no errors
    printf("[+]The file has been recieved successfully!\n");
    printf("[+]Saved in recieved.txt\n");

    close(NewSocketFD);
    close(SocketFD);
    return 0;

    
}