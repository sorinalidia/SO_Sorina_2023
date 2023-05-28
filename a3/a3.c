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

    int shmFd;
    char* shmPtr=NULL;

    /* char request[251];
    ssize_t bytesRead;
    bytesRead = read(reqPipe, request, sizeof(request)) ;
    request[bytesRead] = '\0';
 */
    char request[251];
    ssize_t bytesRead;
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < sizeof(request) - 1) {
        bytesRead = read(reqPipe, request + totalBytesRead, 1);
        if (bytesRead == -1) {
            // Error occurred while reading
            break;
        } else if (bytesRead == 0) {
            // Reached end of input
            break;
        }

        totalBytesRead += bytesRead;

        if (request[totalBytesRead - 1] == '!') {
            // Found the terminator "!"
            break;
        }
    }

    request[totalBytesRead] = '\0';


  
    if(strcmp(request,"EXIT!") == 0)
    {
            munmap(shmPtr,sizeof(char)*2638041);
            //munmap(pointerFisier,sizeof(char)*sizeFile);
            shmPtr = NULL; 
            shm_unlink("/tLMIZD0");
            close(reqPipe);
            close(respPipe);
            unlink(REQ_PIPE_NAME);
            //break;
        }
        else if (strncmp(request, "PING!",6) == 0) {
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

        
        }
        else if (strncmp(request, "CREATE_SHM!",12) == 0) {
            unsigned int shmSize=0;
            read(reqPipe,&shmSize,4);
            
            // Handle Shared Memory Creation Request

            const char* shmName = "/tLMIZD0";
            //unsigned int shmSize = 2638041;
            mode_t shmPermission = 0664;
    

            shmFd = shm_open(shmName, O_CREAT | O_EXCL | O_RDWR, shmPermission);
            if (shmFd != -1) {
                if (ftruncate(shmFd, shmSize) != -1) {
                    shmPtr = (char*)mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
                    if (shmPtr != (void*)-1) {
                        // Successfully created and attached shared memory region
                        const char* successResponse = "SUCCESS";
                        const char* response="CREATE_SHM";
                        int length;
                        length = strlen(response);
                        write(respPipe, response, length);
                        write(respPipe, "!", 1);

                        length = strlen(successResponse);
                        write(respPipe, successResponse, length);
                        write(respPipe, "!", 1);
                    } else {
                        // Failed to attach shared memory region
                        const char* errorResponse = "ERROR";
                        const char* response="CREATE_SHM";
                        int length;
                        length = strlen(response);
                        write(respPipe, response, length);
                        write(respPipe, "!", 1);

                        length = strlen(errorResponse);
                        write(respPipe, errorResponse, length);
                        write(respPipe, "!", 1);
                    }
                } else {
                    // Failed to adjust shared memory size
                       const char* errorResponse = "ERROR";
                        const char* response="CREATE_SHM";
                        int length;
                        length = strlen(response);
                        write(respPipe, response, length);
                        write(respPipe, "!", 1);

                        length = strlen(errorResponse);
                        write(respPipe, errorResponse, length);
                        write(respPipe, "!", 1);
                }
                //close(shmFd);
            } else {
                // Failed to create shared memory region
                const char* errorResponse = "CREATE_SHM ERROR";
                writeStringField(respPipe, errorResponse);
            }
        }
        else if (strcmp(request, "WRITE_TO_SHM!") == 0) {
            // Handle Write to Shared Memory Request
            unsigned int offset=0, value=0;
            read(reqPipe,&offset,4);
            read(reqPipe,&value,4);
            unsigned int shmSize = 2638041;
                    if (offset >= 0 && offset<shmSize && offset + sizeof(unsigned int) <= shmSize) {
                            //unsigned int* shmData = (unsigned int*)((char*)shmPtr + offset);
                            //*shmData = value;

                            memcpy(&shmPtr[offset],(void*)&value,4);

                            // Successfully wrote to shared memory

                            const char* successResponse = "SUCCESS";
                            const char* response="WRITE_TO_SHM";
                            int length;
                            length = strlen(response);
                            write(respPipe, response, length);
                            write(respPipe, "!", 1);

                            length = strlen(successResponse);
                            write(respPipe, successResponse, length);
                            write(respPipe, "!", 1);
                        } else {
                            // Invalid offset or out of bounds
                            const char* errorResponse = "ERROR";
                            const char* response="WRITE_TO_SHM";
                            int length;
                            length = strlen(response);
                            write(respPipe, response, length);
                            write(respPipe, "!", 1);

                            length = strlen(errorResponse);
                            write(respPipe, errorResponse, length);
                            write(respPipe, "!", 1);
                       }
            }
            close(shmFd);

    // Close the request pipe
    close(reqPipe);
    close(respPipe);
    // Remove the response pipe
    if (unlink(RESP_PIPE_NAME) == -1) {
        perror("ERROR: Cannot remove the response pipe");
        return 1;
    } 

    return 0;
}
