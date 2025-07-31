/*
./client.out "Program_Name"  -R "ServerIP"    -p "PortNum"     -l "UserName"
              argv[0]          argv[1,2]       argv[3,4]         argv[5,6]
*/

#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <getopt.h>

const time_t TIMEOUT = 20;

void error (const char *msg, int fd)
{
    close(fd);
    perror(msg);
    printf("\n");
    exit(2);
}

void alarmhandler (int signum)
{
    printf("Timeout, you have to interact with the shell within %ld sec!\n", TIMEOUT);
    exit(4);
}

void custom_usage ()
{
    printf("[-]Usage:\n\
    client_socket_bash [options]\n\
    Options:\n\
    -R <IP address>     address of the server you need to connect to\n\
    -p <Port>           port number you target your connection\n\
    -l <Username>       username you would login with and shouldn't begin with '-'\n");
    return;
}

int main (int argc, char *argv[])
{
    // Declare some variables
    int client_socketFD;
    unsigned int PortNum;
    char buffer[1024];
    unsigned int len = 0;

    struct sockaddr_in server_address;
    // make sure the server_address is ZEROs
    bzero((char *)&server_address, sizeof(server_address));

    // define the server_address structure
    server_address.sin_family = AF_INET;

    // for(int i = 0; i < argc; i++)
    //     printf("argv[%d] = %s\n", i, argv[i]);

    // Get the option flags from the user
    int getopt_re;
    int pflag = 0, lflag = 0, Rflag = 0;
    char *user;
    while( (getopt_re = getopt(argc, argv, "+:R:p:l:")) != -1 )
    {
        /* Check if the option is correct but there was no argument. getopt only detects that if there's no arguments left, 
        otherwise it will get the next option as an argument */
        if(argv[optind-1][0] == '-' && getopt_re != '?')
        {
            fprintf(stderr,"[-]Option -%c requires an argument.\n",getopt_re);
            custom_usage();
            exit(1);
        }

        switch (getopt_re) {
            case 'p':
                if(pflag!=0)
                {
                    fprintf(stderr, "[-]duplicate port option!\n");
                    custom_usage();
                    exit(1); //duplicate pflag option
                }   
                pflag++;
                PortNum = atoi(optarg);
                if (PortNum < 0 || PortNum >= 65535) {
                    printf("[-]Please enter a valid port number!\n");
                    exit(1);
                }
                server_address.sin_port = htons(PortNum);
                break;

            case 'l':
                if(lflag!=0)
                {
                    fprintf(stderr, "[-]duplicate user option!\n");
                    custom_usage();
                    exit(1); //duplicate lflag option
                }   
                lflag++;
                // check the length of the user as it will be sent to server & stored in buffer[1024]
                if (strlen(optarg) >= 1024) {
                    printf("Username is too long!\n");
                    exit(1);
                }
                user = optarg;
                break;

            case 'R':
                if(Rflag!=0)
                {
                    fprintf(stderr, "[-]duplicate address option!\n");
                    custom_usage();
                    exit(1); //duplicate Rflag option
                }   
                Rflag++;
                
                // I can use inet_pton (aton is deprecated) 
                if ( inet_pton(AF_INET, optarg, &(server_address.sin_addr.s_addr)) == 1 )
                {
                    printf("[+]IP address parsed successfully!\n");
                    break;
                } 
                else
                {
                    struct addrinfo hint;
                    struct addrinfo *host_server;
                    bzero((char *)&hint, sizeof(hint));
                    hint.ai_family = AF_INET;

                    printf("optarg: %s\n", optarg);
                    // I tried using gethostbyname() as we took in class, but it's not stable and I found it's deprecated
                    int addrinfo_re = getaddrinfo(optarg, NULL, &hint, &host_server);

                    if(addrinfo_re != 0)
                    {
                        printf("[-]Please, enter a valid host.\n");
                        perror("getaddrinfo");
                        exit(1);
                    }
                    else
                    {
                        server_address.sin_addr = ((struct sockaddr_in *)host_server->ai_addr)->sin_addr;
                    }
                    // function more appropirate here than free, as it free all the linked list
                    freeaddrinfo(host_server);
                    break;
                }
            
            case '?':
                fprintf(stderr,"[-]Option -%c is not a valid option\n",optopt);
                custom_usage();
                exit(1);
                
            case ':':
                fprintf(stderr,"[-]Option -%c requires an argument.\n",optopt);
                custom_usage();
                exit(1);
                    
            default:
                fprintf(stderr,"[-]The value of getopt_return is %c,the option that caused this error is -%c\n",getopt_re,optopt);
                custom_usage();
                exit(1);
        }
    }

    /* Print error msg if the optind "index of next argv[optind] to be checked", at the end of while loop, wasn't = argc-1 
    as argc-1 should be the last parameter at argv[] and is an argument to the last option */
    if(optind != (argc) || argc < 7)
    {
        fprintf(stderr,"[-]Invalid Num of Arguments.\n");
        custom_usage();
        exit(1);
    }

    // Create the socket
    client_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socketFD < 0)
    {
        error("[-]Error Creating Socket.", client_socketFD);
    } 
    else
    {
       printf("[+]Server Socket created with ID = %d\n", client_socketFD); 
    }

    // Connect to the server
    if( connect(client_socketFD, (struct sockaddr *)&server_address, sizeof(server_address)) == 0)
    {
        printf("[+]Connected to the server.\n");
    }
    else
    {
        error("[-]Connection Failed.", client_socketFD);
    }

    // AUTHENTICATE
    // Send the username to the server
    if ( send(client_socketFD, user, strlen(user), 0) < 0 )
    {
        error("[-]Sending Username Failed.", client_socketFD);   
    }

    // Receiving the welcome Message
    if ( recv(client_socketFD, buffer, 92, 0) < 0 )
    {
        error("[-]Receiving Failed.", client_socketFD);
    }
    else
    {
        printf("%s\n", buffer);
    }

    // Interact with the connected clients with send or recv - echo server
    while (true)
    {
        len = 0;
        bzero(buffer, sizeof(buffer));

        // setting new alarm every iteration, so if the user didn't write anything, the alarmhandler will be called
        signal(SIGALRM, alarmhandler);
        alarm(TIMEOUT);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            // Making sure the newline is replaced by NULL - from man fgets, newline is being stored
            if (strlen(buffer) > 0)
            {
                len = strcspn(buffer, "\n");
                buffer[len] = '\0';
            }

            // Check empty messages not to be send
            if (len == 0)
            {
                printf("Please enter non empty Msg or q to exit.\n");
                alarm(0); //Re-setting the alarm signal
                continue;
            }
        }
        else
        {
            error("Error with fun reading your Msg.\n", client_socketFD);
        }

        // Cancelling the alarm signal when finish typing
        alarm(0);
        
        if ( send(client_socketFD, buffer, strlen(buffer), 0) < 0 )
        {
            error("[-]Sending Failed.", client_socketFD);   
        }

        // Check q to close the connection
        // We could use also strcmp
        if (len == 1)
        {
            if (buffer[0] == 'q' || buffer[0] == 'Q')
            {
                close(client_socketFD);
                printf("Client requested disconnection.\n");
                printf("Program closed Normally.\n");
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

        // Clearing the buffer before receiving
        bzero(buffer, sizeof(buffer));

        if ( recv(client_socketFD, buffer, sizeof(buffer), 0) < 0 )
        {
            error("[-]Receiving Failed.", client_socketFD);
        }
        else
        {
            printf("Msg received from server:\n %s\n", buffer);
        }
    }

    // Close the socket
    close(client_socketFD);
    return 0;
}