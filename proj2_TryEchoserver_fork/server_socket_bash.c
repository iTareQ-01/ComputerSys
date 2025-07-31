/*
Usage: Program_Name     Port_Num
        argv[0]         argv[1]
*/

#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#define MAX 100

void error (const char *msg)
{
    perror(msg);
    printf("\n");
    exit(2);
}

int main (int argc, char *argv[])
{
    // Print error msg to show how to use
    if(argc < 2)
    {
        fprintf(stderr, "[-]The program needs 2 parameters, the Program_Name, & the Port_Num. \nProgram terminated!\n");
        exit(1);
    }

    // Declare some variables
    int server_socketFD, connect_client_SocketFD, PortNum;
    char buffer[1024];

    struct sockaddr_in server_address, connect_client_addr;
    socklen_t client_len = 0;

    // Create the socket
    server_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socketFD < 0)
    {
        error("[-]Error Creating Socket.");
    } 
    else
    {
       printf("[+]Server Socket created with ID = %d\n", server_socketFD); 
    }

    // Change the option of the socket to accept multiple bindings to the same port 
    int optval = 1;
    setsockopt(server_socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Bind the socket to a port num.
    
        // Ensure that all the bytes of server_address is 0
    bzero((char *)&server_address, sizeof(server_address));

        // Define the server_address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    PortNum = atoi(argv[1]);
    server_address.sin_port = htons(PortNum); // Host to Network Short

    if( bind(server_socketFD, (struct sockaddr *) &server_address, sizeof(server_address) ) == 0 )
    {
        printf("[+]Binding Successful.\n");
    } 
    else 
    {
        error("[-]Binding Failed.");
    }

    // Listen to the bounden port for client connection requests

    if ( listen(server_socketFD, MAX) == 0 )
    {
        printf("[+]Listening ....\n");
    }
    else
    {
        error("[-]Listening Failed.");
    }

    // Accept multiple requested connections
    while(true)
    {   
        // make the client_address = zero
        bzero((char *)&connect_client_addr, sizeof(connect_client_addr));
        connect_client_SocketFD = accept(server_socketFD, (struct sockaddr *) &connect_client_addr, &client_len);
        if(connect_client_SocketFD < 0)
        {
            perror("[-]Error in Accepting.");
            printf("\n");
            continue;
        }
        else
        {
            printf("[+]1 New Connection Accepted!\n");
        }

        // forking multiple processes
        pid_t pID = fork();
        if (pID < 0)
        {
            close(connect_client_SocketFD);
            continue;
        }
        
        if (pID == 0)
        {    
            // close the server socket FD which in passive mode as we don't need it anymore i nthe child process
            close(server_socketFD);

            // Sending welcome message to the clients
            if ( send(connect_client_SocketFD, "[+]Welcome to The echo server:\n", 32, 0) < 0 ) {
                error ("Sending welcome msg failed");
            }
            
            // Interact with the connected clients with send or recv - echo server
            while (true)
            {
                bzero(buffer, sizeof(buffer));
                ssize_t rec = -1;
                
                // recv fun. is a blocking fun. by default .. as the socket created from ACCEPT is blocking by default
                rec = recv(connect_client_SocketFD, buffer, sizeof(buffer), 0);
                if (rec < 0 && errno == EAGAIN) continue;
                else if (rec < 0 && errno == EWOULDBLOCK) continue;
                else if (rec < 0) error("[-]Receiving Failed.");

                int len = 0;
                // Making sure the newline is replaced by NULL - from man fgets, newline is being stored
                if (strlen(buffer) > 0)
                {
                    len = strcspn(buffer, "\n");
                    buffer[len] == '\0';
                }

                // Check empty messages not to be received
                if (len == 0) 
                {
                    // close(server_socketFD);
                    close(connect_client_SocketFD);
                    printf("Server received empty Msg.\n");
                    printf("Client ID: %d disconnected.\n", connect_client_SocketFD);
                    exit(3);
                }

                // Check q to close the connection
                if (len == 1)
                {
                    if (buffer[0] == 'q' || buffer[0] == 'Q')
                    {
                        // close(server_socketFD);
                        close(connect_client_SocketFD);
                        printf("Client requested disconnection.\n");
                        printf("Client ID: %d disconnected.\n", connect_client_SocketFD);
                        exit(3);
                    }
                }

                printf("Msg received from client: %s\n", buffer);
                
                if ( send(connect_client_SocketFD, buffer, len, 0) < 0 )
                {
                    error("[-]Sending Failed.");
                }
                else 
                {
                    // To solve if the msg was larger than the len, I want the remain to not be sent
                    fflush(fdopen(connect_client_SocketFD, "w"));
                }
            }
        }
        else
        {
            // close the created connection FD in the parent process so, I can accept on it new connection from the while loop
            close(connect_client_SocketFD);
        }
    }
    
    // Close the socket if there's any escape from the while loop
    close(server_socketFD);
    close(connect_client_SocketFD);
    printf("2 sockets closed.\n");
    return 0;
}