#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define REQ_PIPE_NAME "REQ_PIPE_70127"
#define RESP_PIPE_NAME "RESP_PIPE_70127"
#define BUFFER_SIZE 256
int handleRequest(const char* request) {
    if (strcmp(request, "PING!") == 0) {
        // Handle Ping Request
        const char* response = "PING PONG 70127\n";

        // Write back the response
        int respPipe = open(RESP_PIPE_NAME, O_WRONLY);
        if (respPipe == -1) {
            printf("ERROR\ncannot open the response pipe");
           return 1;
        }

        if (write(respPipe, response, strlen(response)) == -1) {
            printf("ERROR\ncannot write the response message");
            return 1;
        }

        close(respPipe);
    } else {
        // Handle other requests
        // ...
    }
    return 1;
}

int main() {
    int reqPipe=-1,respPipe=-1;
    // Create the response pipe
    if (mkfifo(RESP_PIPE_NAME, 0600) != 0) {
        printf("ERROR\ncannot create the response pipe\n");
        return 1;
    }

    // Open the request pipe
    reqPipe = open(REQ_PIPE_NAME, O_RDONLY);
    if (reqPipe == -1) {
        printf("ERROR\ncannot open the request pipe\n");
        unlink(RESP_PIPE_NAME);
        return 1;
    }

    // Open the response pipe
    respPipe = open(RESP_PIPE_NAME, O_WRONLY);
    if (respPipe == -1) {
        printf("ERROR\ncannot open the response pipe\n");
        close(reqPipe);
        unlink(RESP_PIPE_NAME);
        return 1;
    }
    
    // Write the connection request message
    const char* connectionRequest = "START!";
    if (write(respPipe, connectionRequest, strlen(connectionRequest)) !=5) {
        printf("ERROR\n");
         close(reqPipe);
        close(respPipe);
        unlink(RESP_PIPE_NAME);
        return 1;
    }
    printf("SUCCESS\n");

    //close(respPipe);
    char *request=NULL;
    read(reqPipe,request,strlen(request));
   // request[strlen(request)]='\0';


    if (strcmp(request, "PING!") == 0) {
        respPipe = open(RESP_PIPE_NAME, O_WRONLY);
        if (respPipe == -1) {
            printf("ERROR\ncannot open the response pipe");
           return 1;
        }

        write(respPipe,"PING",4);
        write(respPipe,"PONG",4);
        unsigned int val=70127;
        write(respPipe,&val,4);
       /*  if (write(respPipe, response, strlen(response)) !=4) {
            printf("ERROR\ncannot write the response message");
            return 1;
        } */

        close(respPipe);
    }
    /* // Main loop to handle requests
    while (1) {
        // Read the request message
        char request[BUFFER_SIZE];

        ssize_t bytesRead = read(reqPipe, request, sizeof(request));
        if (bytesRead == -1) {
            perror("ERROR: Cannot read the request message");
            return 1;
        } else if (bytesRead == 0) {
            printf("Request pipe closed. Exiting...\n");
            break;
        } else {
            // Null-terminate the received data
            request[bytesRead] = '\0';

            // Handle the request
            handleRequest(request);
        }
    }  */

    // Close the request pipe
    close(reqPipe);

    // Remove the response pipe
    if (unlink(RESP_PIPE_NAME) == -1) {
        perror("ERROR: Cannot remove the response pipe");
        return 1;
    }

    return 0;
}
