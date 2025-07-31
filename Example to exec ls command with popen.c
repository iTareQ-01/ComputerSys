#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "popen() failed!";
    
    char buffer[1024];
    char* result = malloc(1);
    *result = '\0';
    
    while (!feof(pipe)) {
        if (fgets(buffer, 1024, pipe) != NULL) {
            result = realloc(result, strlen(result) + strlen(buffer) + 1);
            strcat(result, buffer);
        }
    }
    
    pclose(pipe);
    return result;
}

int main() {
    printf("%s", exec("ls -al\n"));
    return 0;
}

