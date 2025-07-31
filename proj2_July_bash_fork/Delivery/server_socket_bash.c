/*
Usage: Program_Name     Port_Num
        argv[0]         argv[1]
*/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <security/pam_appl.h>
#define MAX 10000

unsigned int cli_ID = 0;
char *user_pass = NULL;
char *tofree_result = NULL; // just pointer to point to the mem need to be freed

// Signal handler to kill all the child processes
void term_handler (int signum)
{
    kill(0, SIGTERM);
    exit(0);
}

// char* popen_shell(const char* cmd);
char *exec_shell(const char* cmd);

char *authenticate(const char* username);

void error (const char *msg, int fd)
{
    close(fd);
    perror(msg);
    printf("\n");
    exit(2);
}

void exit_child_process (const char *str, int fd, char *free_cd_home, char *free_user)
{
    if ( send(fd, str, strlen(str), 0) < 0 )
    {
        if (free_cd_home != NULL) free(free_cd_home);
        if (free_user != NULL) free(free_user);
        close(fd);
        printf("[-]%s Client ID %d disconnected.\n", str, cli_ID);
        printf("\n");
        exit(4);
    }     
    close(fd);
    printf("[-]%s Client ID %d disconnected.\n", str, cli_ID);
    printf("\n");
    if (free_cd_home != NULL) free(free_cd_home);
    if (free_user != NULL) free(free_user);
    exit(4);
}

int main (int argc, char *argv[])
{
    // Print error msg to show how to use
    if(argc < 2)
    {
        fprintf(stderr, "[-]The program needs 2 parameters, the Program_Name, & the Port_Num. \n[-]Program terminated!\n");
        exit(1);
    }

    // Declare some variables
    int server_socketFD, connect_client_SocketFD;
    unsigned int PortNum;
    char buffer[1024];

    struct sockaddr_in server_address, connect_client_addr;
    socklen_t client_len = 0;

    const time_t TIMEOUT_val = 20;

    // Create the socket
    server_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socketFD < 0)
    {
        error("[-]Error Creating Socket.", server_socketFD);
    } 
    else
    {
       printf("[+]Server Socket created with ID = %d\n", server_socketFD); 
    }

    // Change the option of the socket to accept multiple bindings to the same port 
    int optval = 1;
    if ( setsockopt(server_socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) error("[-]setsockopt Failure.", server_socketFD);

    // Bind the socket to a port num.
    
        // Ensure that all the bytes of server_address is 0
    bzero((char *)&server_address, sizeof(server_address));

        // Define the server_address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    PortNum = atoi(argv[1]);
    if (PortNum < 0 || PortNum >= 65535) error("[-]Please enter a valid port number!\n", server_socketFD);
    server_address.sin_port = htons(PortNum); // Host to Network Short

    if( bind(server_socketFD, (struct sockaddr *) &server_address, sizeof(server_address) ) == 0 )
    {
        printf("[+]Binding Successful.\n");
    } 
    else 
    {
        error("[-]Binding Failed.", server_socketFD);
    }

    // Listen to the bounden port for client connection requests

    if ( listen(server_socketFD, MAX) == 0 )
    {
        printf("[+]Listening ....\n");
    }
    else
    {
        error("[-]Listening Failed.", server_socketFD);
    }

    pid_t pID3 = fork();
    if (pID3 < 0)
    {
        error("[-]Error with forking pID3", server_socketFD);
    }
        
    if (pID3 == 0)
    {
        // while loop to ignore any rubbish input to stdin and exit only if type quit
        while (true)
        {
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                // Making sure the newline is replaced by NULL - from man fgets, newline is being stored
                if (strlen(buffer) > 0)
                {
                    int len = strcspn(buffer, "\n");
                    buffer[len] = '\0';

                    for (int i = 0; i < 4; i++)
                    {
                        buffer[i] = tolower(buffer[i]);
                    }
                    if ( strcmp(buffer, "quit") == 0)
                    {
                        close(server_socketFD);
                        kill (getppid(), SIGTERM);
                        _exit(0);
                    }
                }
            } else {
                error("[-]Error fgets at pID3.", server_socketFD);
            }
            // printf("TEST.\n");
        }
    }
    else
    {
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

        cli_ID += 1;
        // If the parent recieved SIGTERM, kill all the children
        signal(SIGTERM, term_handler);

        // forking multiple processes
        pid_t pID = fork();
        if (pID < 0)
        {
            close(connect_client_SocketFD);
            continue;
        }
        
        if (pID == 0)
        {    
            // close the server socket FD which in passive mode as we don't need it anymore in the child process
            close(server_socketFD);

            // Receiving the Username from the clients when connected
            char *user;
            bool auth = false;
            char *result;
            char *cd_home = (char *)calloc(7, sizeof(char)); //To manage change directory
            if (cd_home == NULL) exit_child_process ("Error with cd_home calloc,", connect_client_SocketFD, cd_home, user);
            strncat(cd_home, "/home/", 7);
            // printf("cd_home: %s", cd_home); 
            char *cd_buffer[2]; //To manage change directory
            ssize_t rec = -1;
            bzero(buffer, sizeof(buffer));
            while (true)
            {
                rec = recv(connect_client_SocketFD, buffer, sizeof(buffer), 0);
                if (rec < 0) exit_child_process ("Receiving Username Failed,", connect_client_SocketFD, cd_home, user);
                else {
                    for (int i = 0; buffer[i] != '\0'; i++)
                    {
                        buffer[i] = tolower(buffer[i]);
                    }
                    user = strndup(buffer, strlen(buffer)); // user needs to be freed as strdup use malloc
                    if (strcmp(user, "root") == 0) exit_child_process ("Root user is not allowed,", connect_client_SocketFD, cd_home, user);
                    break;
                }
            }
            
            // Sending welcome message to the clients
            if ( send(connect_client_SocketFD, "[+]Welcome to Shell Program, Please enter your user password:\n", 92, 0) < 0 ) {
                exit_child_process ("Sending welcome msg failed,", connect_client_SocketFD, cd_home, user);
            }
            
            // Intializing the session time & timeout 
            // We can make the timout function as the client with SIGALRM, but just to use all our syllabus
            fd_set master_readfd;
            FD_ZERO(&master_readfd);
            FD_SET(connect_client_SocketFD, &master_readfd);
            struct timeval TIMEOUT, client_st_time, client_end_time;
            int select_re;

            if ( gettimeofday(&client_st_time, NULL) < 0 ) {
                exit_child_process ("Falied with gettimeofday,", connect_client_SocketFD, cd_home, user);
            }

            // recv fun. is a blocking fun. by default .. as the socket created from ACCEPT is blocking by default
            // I think with the use of select, we don't need this anymore
            int flags = fcntl(connect_client_SocketFD, F_GETFL);
            if (flags < 0) {
                exit_child_process ("Error Getting flags of the connect_client_SocketFD,", connect_client_SocketFD, cd_home, user);
            }
            if ( fcntl(connect_client_SocketFD, F_SETFL, flags | O_NONBLOCK) < 0 ) {
                exit_child_process ("Error Nonblocking the connect_client_SocketFD,", connect_client_SocketFD, cd_home, user);
            }

            // Interact with the connected clients with send or recv - echo server
            unsigned int len = 0;
            while (true)
            {
                bzero(buffer, sizeof(buffer));
                rec = -1;
                select_re = -1;
                len = 0;
                TIMEOUT.tv_sec = TIMEOUT_val;
                TIMEOUT.tv_usec = 0;    

                select_re = select( (connect_client_SocketFD+1), &master_readfd, NULL, NULL, &TIMEOUT);
                if (select_re == -1) {
                    exit_child_process("Failure with select Fun.,", connect_client_SocketFD, cd_home, user);
                } 
                else if (select_re == 0) {
                    exit_child_process ("TIMEOUT Recieving From Client,", connect_client_SocketFD, cd_home, user);
                }
                else
                {
                    rec = recv(connect_client_SocketFD, buffer, sizeof(buffer), 0);
                    if (rec < 0 && errno == EAGAIN) continue;
                    else if (rec < 0 && errno == EWOULDBLOCK) continue;
                    else if (rec < 0) exit_child_process ("Receiving Failed,", connect_client_SocketFD, cd_home, user);
                    else
                    {
                        // Making sure the newline is replaced by NULL - from man fgets, newline is being stored
                        if (strlen(buffer) > 0)
                        {
                            len = strcspn(buffer, "\n");
                            buffer[len] = '\0';
                        }

                        // Check empty messages not to be received
                        if (len == 0) {
                            exit_child_process ("Server received empty Msg,", connect_client_SocketFD, cd_home, user);
                        }

                        // Check q to close the connection
                        if (len == 1)
                        {
                            if (buffer[0] == 'q' || buffer[0] == 'Q')
                            {
                                // close(server_socketFD);
                                close(connect_client_SocketFD);
                                printf("[+]Client requested disconnection.\n");
                                printf("[+]Client ID: %d disconnected.\n", cli_ID);

                                if ( gettimeofday(&client_end_time, NULL) < 0 ) {
                                    exit_child_process ("Falied with gettimeofday,", connect_client_SocketFD, cd_home, user);
                                }
                                printf("Session Time: %ld s\n", (client_end_time.tv_sec - client_st_time.tv_sec));
                                if (cd_home != NULL) free(cd_home);
                                if (user != NULL) free(user);
                                exit(3);
                            }
                        }

                        // AUTHENTICATE, If the first cycle in while loop, auth is false
                        if (auth == false) 
                        {
                            user_pass = buffer;
                            // printf("User: %s\n", user);
                            result = authenticate(user);
                            // when returned from authenticate
                            user_pass = NULL;
                            if (result == "Approved")
                            {
                                auth = true;
                                result = "Authentication done. Type your commands here:\n";
                                cd_home = (char *)realloc( cd_home, ( (6 + strlen(user))*sizeof(char) + 1 ) );
                                if (cd_home == NULL) 
                                    exit_child_process ("Error with cd_home realloc,", connect_client_SocketFD, cd_home, user); 
                                strncat(cd_home, user, strlen(user) + 1);

                                if (chdir(cd_home) != 0)
                                    exit_child_process ("Error with chdir(cd_home),", connect_client_SocketFD, cd_home, user);
                            }
                            free(user);
                            user = NULL;
                        }
                        else
                        {
                            // Sending the command to the exec_shell function
                            printf("[+]Command received from client: %s\n", buffer);
                            // Manage if the buffer has cd command
                            cd_buffer[0] = strtok(buffer, " ");
                            cd_buffer[1] = strtok(NULL, " ");

                            if(strcmp(cd_buffer[0], "cd") == 0)
                            {
                                if (cd_buffer[1] == NULL)
                                {
                                    if (chdir(cd_home) == 0) result = "Directory changed.\n";
                                    else result = "Error changing directory.\n";
                                }
                                else {
                                    if (chdir(cd_buffer[1]) == 0) result = "Directory changed.\n";
                                    else result = "Error changing directory.\n";
                                }
                            }
                            else
                            {
                                // Re connect buffer after strtok, convert '/0' to ' '
                                int temp = strlen(buffer);
                                buffer[temp] = ' ';
                                temp = strlen(buffer);
                                buffer[temp] = ' ';
                                result = exec_shell(buffer);
                            }
                            // printf("%s\n", result);  
                        }

                        len = strlen(result);

                        if ( send(connect_client_SocketFD, result, len, 0) < 0 ) {
                            if (tofree_result != NULL) free(tofree_result);
                            exit_child_process ("Sending Failed,", connect_client_SocketFD, cd_home, user);
                        }
                        else 
                        {
                            // printf("tofree_result = %s\n", tofree_result);
                            if (tofree_result != NULL) {
                                free(tofree_result);
                                tofree_result = NULL;
                            }
                            // To solve if the msg was larger than the len, I want the remain to not be sent
                            fflush(fdopen(connect_client_SocketFD, "w"));
                        }

                        if (auth == false)
                            exit_child_process("", connect_client_SocketFD, cd_home, user);
                    }
                }  
            }
            close(connect_client_SocketFD);
            if (cd_home != NULL) free(cd_home);
            if (user != NULL) free(user);
            exit(3);
        }
        else
        {
            // close the created connection FD in the parent process so, I can accept on it new connection from the while loop
            close(connect_client_SocketFD);
        }
    }
    } //Closing else forking pID3
    
    // Close the socket if there's any escape from the while loop
    close(server_socketFD);
    // printf("[+]1 socket closed.\n"); ---> No need
    exit(0);
}

// function opens and writes to a file to pipe contents of an exec call to /bin/bash/
// we can redirect the result of the bash to the connect_client_socketFD, but we need to handle the commands without return val, like cd, mkdir, etc....
char *exec_shell(const char *cmd) {

    char sh_buffer[1024];
    // I chose calloc to intialize all the blocks with 0
    char *sh_result = (char *)calloc(1024, sizeof(char));
    if (sh_result == NULL)
    {
        // free(sh_result);
        sh_result = "Error with calloc. Your Command wasn't executed!\n"; // --------> add \n as the exec return \n when the command executed, so we need to bee the same for eye-relief purposes
        return sh_result;
    }

    int fd[2];
    // fd[1] write
    // fd[0] read
    if (pipe(fd) < 0)
    {
        free(sh_result);
        sh_result = "Error with Pipe. Your Command wasn't executed!\n";
        return sh_result;
    }
    // forking 2 processes; one for exec & the other to perserve the Socket
    pid_t pID2 = fork();
    if (pID2 < 0)
    {
        free(sh_result);
        sh_result = "Error with Fork. Your Command wasn't executed!\n";
        return sh_result;
    }
    
    if (pID2 == 0)
    {  
        // In the child - child process
        close(fd[0]); 
        // printf("cmd: %s\n", cmd);
        dup2(fd[1], 1); //std_output
        dup2(fd[1], 2); //std_err
        execl("/bin/bash", "bash", "-c", cmd, NULL);

        // If exec failed, then kill the child - child process
        perror("Exec Fun Failed.");
        close(fd[1]);
        exit(5);
    }  
    else
    {
        // In the child process with the connect_client_SocketFD open
        close(fd[1]);
        // We nned to chech the returned status from the child, if it's executed & ended normally or not
        int status;
        wait(&status);
        /* ----- NO need to handle status as if the exec returned error it will piped to the sh_result and printed to the client ----- */
        // printf("Child - Child process finished execution with status %d\n.", status);
        // if (status == NULL) {
        //     sh_result = "[-]Command exec was not able to be done, try another syntax:\n";
        // }

        bool first_read = true;
        // The counter to increase the size of sh_result by 2 first if the size of 1024 was not enough
        unsigned int counter = 2;
        int r_read = 0;
        while (true)
        {
            bzero(sh_buffer, sizeof(sh_buffer));
            r_read = read(fd[0], sh_buffer, 1024);
            // printf("r_read: %d\n", r_read);
            // printf("first_read = %B\n", first_read);
            if ( r_read > 0)
            {
                strcat(sh_result, sh_buffer);
                if (r_read >= 1024)
                {
                    // We made a tmp pointer to be able to free sh_result if failure happened. I think realloc do that for us, no need
                    char *tmp = (char *)realloc(sh_result, counter*1024);
                    if (tmp == NULL)
                    {
                        free(sh_result);
                        sh_result = "Error with realloc. Your Command wasn't executed!\n"; // --------> add \n as the exec return \n when the command executed, so we need to bee the same for eye-relief purposes
                        return sh_result;
                    }
                    sh_result = tmp;
                } else {
                    break;
                }
            } 
            else if ( r_read == 0 && first_read == true)
            {
                // If Error happened, it will redirected to FD as stderr is redirected to fd[1]
                // So, if the FD is empty, then the command is done correctly without any outputs. Like cd, mkdir, rm, etc...
                free(sh_result);
                sh_result = "Command executed without return!\n";
                return sh_result;
            }
            else if ( r_read == 0 && first_read == false)
            {
                break;
            }
            else if ( r_read < 0)
            {
                free(sh_result);
                sh_result = "Error with Read. Your Command wasn't executed!\n";
                return sh_result;
            }
            first_read = false;
            counter += 1;
        }
        tofree_result = sh_result;
        return sh_result;
    }
}

// /////////////// AUTHENTICATION WITH PAM /////////////// //

int conversation(int num_msg, const struct pam_message **msg,
		 struct pam_response **resp, void *appdata_ptr)
{ /* We malloc an array of num_msg responses */
	struct pam_response *array_resp = (struct pam_response *)malloc(num_msg * sizeof(struct pam_response));
	for (int i = 0; i < num_msg; i++) {
		/* resp_retcode should be set to zero */
		array_resp[i].resp_retcode = 0;

		/* The message received from the module */
		const char *msg_content = msg[i]->msg;

		/* Printing the message (e.g. "login:", "Password:") */
		// printf("%s", msg_content);

		/* Malloc-ing the resp string of the i-th response */
		array_resp[i].resp = (char *)malloc(strlen(user_pass) + 1);

		/* Writing password in the allocated string */
		strcpy(array_resp[i].resp, user_pass);
	}

	/* setting the param resp with our array of responses */
	*resp = array_resp;
	/* Here we return PAM_SUCCESS, which means that the conversation happened correctly.
	 * You should always check that, for example, the user didn't insert a NULL password etc */
	return PAM_SUCCESS;
}

/**
 * @brief Specifies the conversation function to use and the pointer to additional data
 */
static struct pam_conv conv = {
	conversation, /* Our conversation function */
	NULL /* We don't need additional data now*/
};

char *authenticate(const char* username)
{
    // getpwnam by default should return the data in /etc/passwd file
    /* struct passwd {
                char   *pw_name;       username
                char   *pw_passwd;     user password
                char   *pw_dir;        home directory
    */

    // WE WILL USE PAM authentication module
    pam_handle_t *handle = NULL;
	const char *service_name = "pam_auth_local_users";
	int retval;
    char *pam_result;

	retval = pam_start(service_name, username, &conv, &handle); /* Initializing PAM */
	if (retval != PAM_SUCCESS) {
		fprintf(stderr, "[-]Failure in pam initialization: %s", pam_strerror(handle, retval));
		pam_result = "Error with Authentication intialization.\n Client disconnected.\n";
        return pam_result;
	} 
	else {
		printf("[+]PAM Intilization done.\n");
	}

	retval = pam_authenticate(
		handle,
		0); /* Do authentication (user will be asked for username and password)*/
	if (retval != PAM_SUCCESS) {
		fprintf(stderr, "[-]Failure in pam authentication: %s\n", pam_strerror(handle, retval));
        pam_result = "You're not authenticated to use this server.\n Client disconnected.\n";
        pam_end(handle, retval);
        return pam_result;
	}
	else {
		printf("[+]PAM Auth done.\n");
	}

	retval = pam_acct_mgmt(
		handle,
		0); /* Do account management (check the account can access the system) */
	if (retval != PAM_SUCCESS) {
		fprintf(stderr, "[-]Failure in pam account management: %s\n", pam_strerror(handle, retval));
        pam_result = "You're not authenticated to use this server.\n Client disconnected.\n";
        pam_end(handle, retval);
        return pam_result;
	}
	else {
		printf("[+]PAM Mgmt done.\n");
	}

	pam_end(handle, retval); /* ALWAYS terminate the pam transaction!! */
    pam_result = "Approved";
    // printf("%s", pam_result);
    return pam_result;
}