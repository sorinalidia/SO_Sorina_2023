#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define REQ_PIPE_NAME "REQ_PIPE_70127"
#define RESP_PIPE_NAME "RESP_PIPE_70127"
#define BUFFER_SIZE 256

void writeStringField(int pipe, const char* str) {
    size_t length = strlen(str);
    write(pipe, str, length);
    write(pipe, "!", 1);
}

void writeNumberField(int pipe, unsigned int number) {
    write(pipe, &number, sizeof(number));
}

int main() {
    int reqPipe = -1, respPipe = -1;

    // Create the response pipe
    if (mkfifo(RESP_PIPE_NAME, 0600) != 0) {
        printf("ERROR: cannot create the response pipe\n");
        return 1;
    }

    // Open the request pipe
    reqPipe = open(REQ_PIPE_NAME, O_RDONLY);
    if (reqPipe == -1) {
        printf("ERROR: cannot open the request pipe\n");
        unlink(RESP_PIPE_NAME);
        return 1;
    }

    // Open the response pipe
    respPipe = open(RESP_PIPE_NAME, O_WRONLY);
    if (respPipe == -1) {
        printf("ERROR: cannot open the response pipe\n");
        close(reqPipe);
        unlink(RESP_PIPE_NAME);
        return 1;
    }

    // Write the connection request message
    const char* connectionRequest = "START!";
    if (write(respPipe, connectionRequest, strlen(connectionRequest)) != 6) {
        printf("ERROR: cannot write the connection request message\n");
        close(reqPipe);
        close(respPipe);
        unlink(RESP_PIPE_NAME);
        return 1;
    }
    printf("SUCCESS\n");

    char request[251];
    ssize_t bytesRead;
/*  char* pointerMemoriePartajta = NULL;
 char* pointerFisier = NULL;
 off_t sizeFile = 0; */
    while (1) {
        bytesRead = read(reqPipe, request, sizeof(request)) ;
        request[bytesRead] = '\0';
     /*    if (bytesRead<0)
        {
            close(reqPipe);
            close(respPipe);
            unlink(RESP_PIPE_NAME);

            break;
        } */

          if(strcmp(request,"EXIT!") == 0)
        {
            /* munmap(pointerMemoriePartajta,sizeof(char)*2638041);
            munmap(pointerFisier,sizeof(char)*sizeFile);
            pointerMemoriePartajta = NULL;
            shm_unlink("/tLMIZD0"); */
            close(reqPipe);
            close(respPipe);
            unlink(REQ_PIPE_NAME);
            break;
        }else if (strcmp(request, "PING!") == 0) {
            // Handle Ping Request
            const char* response1 = "PING";
            const char* response2 = "PONG";
            unsigned int number = 70127;

            int length;
            length = strlen(response1);
            write(respPipe, response1, length);
            write(respPipe, "!", 1);

            length = strlen(response2);
            write(respPipe, response2, length);
            write(respPipe, "!", 1);

            write(respPipe, &number, sizeof(number));

           // writeStringField(respPipe, response1);
            //writeStringField(respPipe, response2);
            //writeNumberField(respPipe, response3);
        }
        else{
            break;
        }
    }

    // Close the request pipe
    close(reqPipe);
  /*   close(respPipe);
    // Remove the response pipe
    if (unlink(RESP_PIPE_NAME) == -1) {
        perror("ERROR: Cannot remove the response pipe");
        return 1;
    } */

    return 0;
}
