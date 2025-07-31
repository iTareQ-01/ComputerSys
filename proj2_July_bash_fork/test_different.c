
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


int main()
{
    char buffer[] = "la -l -a";
    char *cd_buffer[2];
    
    printf("[+]Command received from client: %s\n", buffer);
    // Manage if the buffer has cd command
    cd_buffer[0] = strtok(buffer, " ");
    cd_buffer[1] = strtok(NULL, " ");
    printf("cd_buffer[0]: %s\n", cd_buffer[0]);
    printf("cd_buffer[1]: %s\n", cd_buffer[1]);
    printf("strlen cd_buffer[0]: %ld\n", strlen(cd_buffer[0]) );
    printf("strlen cd_buffer[1]: %ld\n", strlen(cd_buffer[1]) );
    // while(*cd_buffer[0])
    // printf("cd_buffer[0]: %02x\n", (unsigned int) *cd_buffer[0]++);
                                int temp = strlen(cd_buffer[0]);
                                buffer[temp] = ' ';
                                temp = strlen(cd_buffer[0]);
                                buffer[temp] = ' ';

    printf("buffer: %s\n", buffer);
    
}