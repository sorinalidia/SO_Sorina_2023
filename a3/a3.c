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

    int shmFd=-1;
    char* shmPtr=NULL;
    char* filePtr=NULL;
    off_t fileSize=0;
    int fd=-1;

    /* char request[251];
    ssize_t bytesRead;
    bytesRead = read(reqPipe, request, sizeof(request)) ;
    request[bytesRead] = '\0';
 */
    while(1){

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

    unsigned int shmSize=0;
  
    if(strncmp(request,"EXIT!",5) == 0)
    {
            munmap(shmPtr,sizeof(char)*2638041);
            munmap(filePtr,sizeof(char)*fileSize);
            shmPtr = NULL; 
            shm_unlink("/tLMIZD0");
            close(reqPipe);
            close(respPipe);
            close(shmFd);
          
        
            unlink(REQ_PIPE_NAME);
            break;
        }
        if (strncmp(request, "PING!",6) == 0) {
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
        if (strncmp(request, "CREATE_SHM!",12) == 0) {
            
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
                    
                        writeStringField(respPipe,response);
                        writeStringField(respPipe,successResponse);
                    } else {
                        // Failed to attach shared memory region
                        const char* errorResponse = "ERROR";
                        const char* response="CREATE_SHM";
                
                        writeStringField(respPipe,response);
                        writeStringField(respPipe,errorResponse);
                    }
                } else {
                    // Failed to adjust shared memory size
                        const char* errorResponse = "ERROR";
                        const char* response="CREATE_SHM";
                        writeStringField(respPipe,response);
                        writeStringField(respPipe,errorResponse);
                }
            } else {
                // Failed to create shared memory region
                const char* errorResponse = "ERROR";
                const char* response="CREATE_SHM";
                writeStringField(respPipe,response);
                writeStringField(respPipe, errorResponse);
            }
        }
        if (strncmp(request, "WRITE_TO_SHM!",14) == 0) {
            // Handle Write to Shared Memory Request
            unsigned int offset=0, value=0;
            read(reqPipe,&offset,sizeof(unsigned int));
            read(reqPipe,&value,sizeof(unsigned int));
            //unsigned int shmSize = 2638041;
                    if (offset >= 0 && offset<shmSize && offset + sizeof(unsigned int) <= shmSize) {
                            memcpy(&shmPtr[offset],(void*)&value,4);

                            // Successfully wrote to shared memory

                            const char* successResponse = "SUCCESS";
                            const char* response="WRITE_TO_SHM";
                            writeStringField(respPipe,response);
                            writeStringField(respPipe,successResponse);
                        } else {
                            // Invalid offset or out of bounds
                            const char* errorResponse = "ERROR";
                            const char* response="WRITE_TO_SHM";
                            writeStringField(respPipe,response);
                            writeStringField(respPipe,errorResponse);
                       }
            }

            if (strncmp(request, "MAP_FILE!", 9) == 0) {
        // Handle Map File Request
        char fileName[251];
        ssize_t bytesRead;
        bytesRead = read(reqPipe, fileName, sizeof(fileName)) ;
        fileName[bytesRead-1] = '\0';

        fd = open(fileName, O_RDONLY);
        if (fd != -1) {
            struct stat fileStat;
            if (fstat(fd, &fileStat) != -1) {
                fileSize = fileStat.st_size;
                filePtr=(char*)mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
                if (filePtr != (void*)-1) {
                    // Successfully mapped file in memory
                    const char* successResponse = "SUCCESS";
                    const char* response = "MAP_FILE";
                    writeStringField(respPipe,response);
                    writeStringField(respPipe,successResponse);
                } else {
                    // Failed to map file in memory
                    const char* errorResponse = "ERROR";
                    const char* response = "MAP_FILE";
                    writeStringField(respPipe,response);
                    writeStringField(respPipe,errorResponse);
                }
            } else {
                // Failed to get file information
                const char* errorResponse = "ERROR";
                const char* response = "MAP_FILE";
                writeStringField(respPipe,response);
                writeStringField(respPipe,errorResponse);
            
            }
        } else {
            // Failed to open the file
            const char* errorResponse = "ERROR";
            const char* response = "MAP_FILE";
            writeStringField(respPipe,response);
            writeStringField(respPipe,errorResponse);
        }
    }
     if (strncmp(request, "READ_FROM_FILE_OFFSET!", 22) == 0) {
        // Handle Read From File Offset Request
        unsigned int offset=0,nrBytes=0;
        read(reqPipe,&offset,4);
        read(reqPipe,&nrBytes,4);

            if (shmPtr != (void*)-1) {
                // Shared memory region exists
                struct stat fileStat;
                if (fstat(fd, &fileStat) != -1) {
                    fileSize = fileStat.st_size;
                    if (offset + nrBytes <= fileSize) {
                        // Read from memory-mapped file
                         char* reader=(char*)malloc((nrBytes+1)*sizeof(char));
                        memcpy(reader,(filePtr+offset),nrBytes);

                        memcpy(shmPtr, reader, nrBytes);

                        // Successfully read from file offset
                        const char* successResponse = "SUCCESS";
                        const char* response = "READ_FROM_FILE_OFFSET";
                        writeStringField(respPipe,response);
                        writeStringField(respPipe,successResponse);
                    } else {
                        // Invalid offset or out of bounds
                        const char* errorResponse = "ERROR";
                        const char* response = "READ_FROM_FILE_OFFSET";
                        writeStringField(respPipe,response);
                        writeStringField(respPipe,errorResponse);
                    }
                } else {
                    // Failed to get file information
                    const char* errorResponse = "ERROR";
                    const char* response = "READ_FROM_FILE_OFFSET";
                    writeStringField(respPipe,response);
                    writeStringField(respPipe,errorResponse);
                }
            } else {
                // Shared memory region does not exist
                const char* errorResponse = "ERROR";
                const char* response = "READ_FROM_FILE_OFFSET";
                writeStringField(respPipe,response);
                writeStringField(respPipe,errorResponse);
            
        }
    }
    if (strncmp(request, "READ_FROM_FILE_SECTION!", 23) == 0) {
        // Handle Read From File Section Request
        unsigned int sectionNo;
        unsigned int offset;
        unsigned int numBytes;

        // Read section number, offset, and number of bytes
        read(reqPipe, &sectionNo, sizeof(sectionNo));
        read(reqPipe, &offset, sizeof(offset));
        read(reqPipe, &numBytes, sizeof(numBytes));

        char *magic=(char*)malloc(sizeof(char)*5);
        unsigned char version;

        memcpy(magic,filePtr,2);
        magic[2]='\0';
        memcpy(&version,filePtr+4,1);
        memcpy(&sectionNo,filePtr+5,1);
        unsigned int numberOfSections = 0;


    if((strcmp(magic,"nQ") == 0) && (version>=122 && version<=143) && (numberOfSections>=6 && numberOfSections<=18) && sectionNo<=numberOfSections){
        // Check if shared memory region exists
        if (shmFd != -1) {
            // Get the file size
            struct stat st;
            if (fstat(fd, &st) == 0) {
                off_t fileSize = st.st_size;
                // Calculate section size
                off_t sectionSize = fileSize / sectionNo;
                // Calculate section offset
                off_t sectionOffset = (sectionNo - 1) * sectionSize;
                // Calculate absolute offset within section
                off_t absoluteOffset = sectionOffset + offset;
                if (absoluteOffset + numBytes <= fileSize) {
                    // Map the file into memory
                   
                    if (filePtr != (void*)-1) {
                        // Get the shared memory region pointer
                        if (shmPtr != (void*)-1) {
                            // Read from memory-mapped file section
                            memcpy(shmPtr, filePtr + absoluteOffset, numBytes);

                            // Successfully read from file section
                            const char* successResponse = "SUCCESS";
                            const char* response = "READ_FROM_FILE_SECTION";
                            
                            writeStringField(respPipe,response);
                            writeStringField(respPipe,successResponse);
                        } else {
                            // Failed to map shared memory region
                            const char* errorResponse = "ERROR";
                            const char* response = "READ_FROM_FILE_SECTION";
                            writeStringField(respPipe,response);
                            writeStringField(respPipe,errorResponse);
                        }

                        // Unmap the shared memory region
                       // munmap(shmPtr, fileSize);
                    } else {
                        // Failed to map the file into memory
                        const char* errorResponse = "ERROR";
                        const char* response = "READ_FROM_FILE_SECTION";
                        writeStringField(respPipe,response);
                        writeStringField(respPipe,errorResponse);
                    }
                } else {
                    // Invalid offset or out of bounds
                    const char* errorResponse = "ERROR";
                    const char* response = "READ_FROM_FILE_SECTION";
                    writeStringField(respPipe,response);
                    writeStringField(respPipe,errorResponse);
                }
            } else {
                // Failed to get file information
                const char* errorResponse = "ERROR";
                const char* response = "READ_FROM_FILE_SECTION";
                writeStringField(respPipe,response);
                writeStringField(respPipe,errorResponse);
            }
        } else {
            // Shared memory region does not exist
            const char* errorResponse = "ERROR";
            const char* response = "READ_FROM_FILE_SECTION";
            writeStringField(respPipe,response);
            writeStringField(respPipe,errorResponse);
        }
    }else {
       // Shared memory region does not exist
            const char* errorResponse = "ERROR";
            const char* response = "READ_FROM_FILE_SECTION";
            writeStringField(respPipe,response);
            writeStringField(respPipe,errorResponse);
        }
    }
    if (strncmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET!", 30) == 0) {
        // Handle Read From Logical Space Offset Request
        unsigned int logicalOffset;
        unsigned int numBytes;

        // Read logical offset and number of bytes
        read(reqPipe, &logicalOffset, sizeof(logicalOffset));
        read(reqPipe, &numBytes, sizeof(numBytes));

        // Calculate the corresponding offset in the mapped file
        
        unsigned int fileSize = lseek(fd, 0, SEEK_END);
        unsigned int alignment = 3072;
        unsigned int sectionSize = fileSize / alignment;
        unsigned int sectionNo = logicalOffset / alignment;
        unsigned int sectionOffset = (sectionNo - 1) * sectionSize;
        unsigned int absoluteOffset = sectionOffset + (logicalOffset % alignment);
      
        // Check if the offset and number of bytes are valid
        if (absoluteOffset + numBytes <= fileSize) {
            // Map the file into memory
            
            if (filePtr != (void*)-1) {
                // Get the shared memory region pointer
                
                if (shmPtr != (void*)-1) {
                    // Read from memory-mapped file section
                    memcpy(shmPtr, filePtr + absoluteOffset, numBytes);

                    // Successfully read from logical space offset
                    writeStringField(respPipe, "READ_FROM_LOGICAL_SPACE_OFFSET");
                    writeStringField(respPipe, "SUCCESS");
                } else {
                    // Failed to map shared memory region
                    writeStringField(respPipe, "READ_FROM_LOGICAL_SPACE_OFFSET");
                    writeStringField(respPipe, "ERROR");
                }

                // Unmap the shared memory region
                munmap(shmPtr, fileSize);
            } else {
                // Failed to map the file into memory
                writeStringField(respPipe, "READ_FROM_LOGICAL_SPACE_OFFSET");
                writeStringField(respPipe, "ERROR");
            }
        } else {
            // Invalid offset or out of bounds
            writeStringField(respPipe, "READ_FROM_LOGICAL_SPACE_OFFSET");
            writeStringField(respPipe, "ERROR");
        }
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
