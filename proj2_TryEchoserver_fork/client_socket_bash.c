/*
./client.out "Program_Name"  "ServerIP"     "PortNum"
              argv[0]          argv[1]       argv[2]   
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
#include <netdb.h>
#include <arpa/inet.h>

void error (const char *msg)
{
    perror(msg);
    printf("\n");
    exit(2);
}

int main (int argc, char *argv[])
{
    // Print error msg to show how to use
    if(argc < 3)
    {
        fprintf(stderr, "[-]The program needs 3 parameters, Program_Name, ServerIP & Port_Num to connect to.\n Program terminated!\n");
        exit(1);
    }

    // Declare some variables
    int client_socketFD, PortNum;
    char buffer[1024];

    struct sockaddr_in server_address;

    // Create the socket
    client_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socketFD < 0)
    {
        error("[-]Error Creating Socket.");
    } 
    else
    {
       printf("[+]Server Socket created with ID = %d\n", client_socketFD); 
    }

    // Connect to the server

        // make sure the server_address is ZEROs
        bzero((char *)&server_address, sizeof(server_address));

        // define the server_address structure
        server_address.sin_family = AF_INET;

            // hostent fun to store info about the host like host name and the IPv4

            // struct hostent *host_server;
            // printf("HOST Entered: %s\n", argv[1]);
            // host_server = gethostbyname(argv[1]);
            // if(host_server == NULL) error("Please, enter a valid host!");
            // printf("HOST_SERVER -> H_NAME : %s\n", host_server->h_name);
        
            // Wrong use of bcopy

            // host_server->h_length = strlen(argv[1]);
            // bcopy( host_server->h_name, (char *) &server_address.sin_addr.s_addr, host_server->h_length );
            
            // I can use inet_pton (aton is deprecated) 

            if ( inet_pton(AF_INET, argv[1], &(server_address.sin_addr.s_addr)) == 1 )
            {
                printf("[+]IP address parsed successfully!\n");
            } 
            else
            {
                error("[-]Please enter a valid IP address.\n");
            }
            // printf("HOST Recorded: %s\n", (char *)&server_address.sin_addr.s_addr);
            // printf("HOST Recorded: %d\n", server_address.sin_addr.s_addr);


        PortNum = atoi(argv[2]);
        server_address.sin_port = htons(PortNum);

    if( connect(client_socketFD, (struct sockaddr *)&server_address, sizeof(server_address)) == 0)
    {
        printf("[+]Connected to the server.\n");
    }
    else
    {
        error("[-]Connection Failed.");
    }

    // Receiving the welcome Message
    if ( recv(client_socketFD, buffer, 32, 0) < 0 )
    {
        error("[-]Receiving Failed.");
    }
    else
    {
        printf("%s\n", buffer);
    }

    // Interact with the connected clients with send or recv - echo server
    while (true)
    {
        int len = 0;
        bzero(buffer, sizeof(buffer));
        puts("Write your Msg:"); //Newline automatically added with puts
        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            // Making sure the newline is replaced by NULL - from man fgets, newline is being stored
            if (strlen(buffer) > 0)
            {
                len = strcspn(buffer, "\n");
                buffer[len] == '\0';
            }

            // Check empty messages not to be send
            if (len == 0)
            {
                printf("Please enter non empty Msg or q to exit.\n");
                continue;
            }
        }
        else
        {
            error("Error reading your Msg.");
        }
        
        if ( send(client_socketFD, buffer, strlen(buffer), 0) < 0 )
        {
            error("[-]Sending Failed.");   
        }

        // Check q to close the connection
        // We could use also strcmp
        if (len == 1)
        {
            if (buffer[0] == 'q' || buffer[0] == 'Q')
            {
                close(client_socketFD);
                printf("Client requested disconnection.\n");
                printf("1 sockets closed.\n");
                exit(3);
            }
        }

        // fgets automatically store only the "sizeof(buffer) - 1" and the last char changed to '\0'
        // So, strlen will return maximum 1024 - 1
        if ( len >= (sizeof(buffer) - 1) )
        {
            printf("Warning, you reached the maximum %ld characters for your Msg.\n", sizeof(buffer));
            // To delete the remaining data if the msg was larger than len
            fflush(stdin);
            fflush(fdopen(client_socketFD, "w"));
        }

        if ( recv(client_socketFD, buffer, len, 0) < 0 )
        {
            error("[-]Receiving Failed.");
        }
        else
        {
            printf("Msg received from server: %s\n", buffer);
        }
    }

    // Close the socket
    close(client_socketFD);
    printf("1 sockets closed.\n");
    return 0;
}