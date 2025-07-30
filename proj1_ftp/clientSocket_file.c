/*
./client.out "Program_Name"     "filename"  "ServerIP"     "PortNum"
              argv[0]            argv[1]     argv[2]        argv[3]
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
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
    if(argc < 4)
    {
        fprintf(stderr, "[-]The program needs 4 parameters, Program_Name, File_Name, ServerIP & Port_Num.\n Program terminated.\n");
        exit(1);
    }

    // Declare some variables
    int SocketFD, PortNum;
    char buffer[256];
    struct sockaddr_in server_address;

    // Creating the socket and return with the socket file descriptor
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(SocketFD < 0)
    {
        error("[-]Error Creating Socket.");
    } else
    {
        printf("[+]Client-Server Socket Created.\n");
    }

    // Ensure that all the bytes of server_address is 0
    bzero((char *) &server_address, sizeof(server_address));

    // Define the server_address structure
        // hostent fun to store info about the host like host name and the IPv4 protocol
        struct hostent *host_server;
        host_server = gethostbyname(argv[2]);
        if(host_server == NULL) fprintf (stderr, "Please, enter a right host!");

    server_address.sin_family = AF_INET;
    bcopy( (char *) host_server->h_addr, (char *) &server_address.sin_addr.s_addr, host_server->h_length );
    PortNum = atoi(argv[3]);
    server_address.sin_port = htons(PortNum); // Host to Network Short

    // creating connect function
    if( connect(SocketFD, (struct sockaddr *) &server_address, sizeof(server_address)) < 0 )
    {
        error("[-]Connection Failed");
    } else
    {
        printf("[+]Connected to the Server.\n");
    }

    // Sending the file to the SocketFD 
    int n;
    int file_txt = open(argv[1], O_RDONLY);
    while(true)
    {
        bzero(buffer, 256);
        n = read(file_txt, buffer, 255);
        if (n < 0)
        {
            error("[-]Error in Reading the file");
        }
        else if (n == 0)
        {
            break;
        }
        write(SocketFD, buffer, 255);
    }

    // Msg to the screen if the file sent with no errors
    printf("[+]The file has been sent successfully!\n");
    printf("[+]Disconnecting from the Server...\n");
    close(SocketFD);
    return 0;

    
}