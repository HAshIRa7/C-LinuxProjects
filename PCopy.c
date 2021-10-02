#include <stdio.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h>   
#include <string.h>  
#include <stdlib.h>
 
#define _XOPEN_SOURCE 500 
#define N 1000 * 1000 * 1000


int CopyFile(int fd_1, int fd_2) { 
    off_t offset_1 = 0;  
    off_t offset_2 = 0;
    char* buffer = malloc(N * sizeof(char));  
    //char buffer[16];  
    ssize_t bytes_readden = 0; 
    ssize_t read_bytes = 0;  

    ssize_t bytes_written = 0;
    
    while(1) {
        ssize_t read_bytes = pread(fd_1, buffer + bytes_readden, 16, offset_1);    

        if(read_bytes < 0) { 
            return 3; 
        } 

        if(read_bytes == 0) {
            break;
        }

        bytes_readden += read_bytes; 
        offset_1 += read_bytes;  

        printf("bytes read = %ld\n", read_bytes); 

    }   

    printf("bytes readden  = %ld\n", bytes_readden);

    while(bytes_written < bytes_readden) {
        ssize_t write_bytes = pwrite(fd_2, buffer + bytes_written, bytes_readden, offset_2); 

        if(write_bytes < 0) { 
            return 4; 
        } 

        bytes_written += write_bytes; 
        offset_2 += write_bytes;       
    } 
     
    printf("bytes written = %ld\n", bytes_written); 

    return 0;// OK 
} 


int main(int argc, char* argv[]) { 

    if(argc != 3) { 
        perror("Something worong") ;
        return 1; 
    } 

    if(strcmp(argv[1], argv[2]) == 0) {  
        perror("It's the same file"); 
        return 2;
    }  

    int fd_1 = open(argv[1], O_RDONLY, 0644); 
    int fd_2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);     



    int res = CopyFile(fd_1, fd_2); 

    switch(res) { 
        case 3 : perror("Can't read all bytes"); break;
        case 4 : perror("Can't write all bytes"); break;
    } 


    if(close(fd_1) < 0) {
        perror("Can't close file for reading"); 
        return 5;
    }

    if(close(fd_2) < 0) {
        perror("Can't close file for writing");
        return 6; 
    }

    return 0;
}