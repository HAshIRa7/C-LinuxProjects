#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>  
#include <unistd.h> 
#include <stdbool.h> 
#include <stdlib.h> 
#include <string.h>

#define N  1000 * 1000 * 1000 


int CopyFile(int fd_1, int fd_2) { 
    char* buffer = malloc(N  * sizeof(char)); 
     
    int bytes_read;  
    int count_read = 0;
    int bytes_written = 0; 

    while(true) {  
        char c;  
        //printf("WTF");
        bytes_read = read(fd_1, &c, 1);  
        if(bytes_read == 0) {
            buffer[count_read] = '\0';
            break; 
        }  
        if (bytes_read < 0) {
            return 1;
        }  
        buffer[count_read] = c; 
        count_read += bytes_read; 
    } 
     

    while (bytes_written < count_read) { 

        ssize_t res = write(fd_2, buffer + bytes_written, count_read - bytes_written);

        if(res < 0) {
            return res;
        }

        bytes_written += res;

    } 

    free(buffer);  
    return 0;
}


int main(int argc, char* argv[])  { 
    //1 исполняемый файл  
    //2 файл из которого читаем 
    //3 файл в который пишем
    if(argc != 3)  { 
        perror("OOps not correct operate\n");  
        return 1; 
    }   

    if(strcmp(argv[1], argv[2]) == 0) { 
        perror("You try to copy file to itself\n"); 
        return 2;
    }

    int fd_1 = open(argv[1], O_RDONLY, 0644);  
    int fd_2 = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644); 

    if(fd_1 < 0) {
        perror("I can't open a file for reading\n"); 
        return 3;
    } 

    if(fd_2 < 0) {
        perror("I can't open a file for writing\n");
        return 4; 
    }  

    switch(CopyFile(fd_1, fd_2)) { 
        case 1 : perror("Can't read"); 
            break; 
        case 2 : perror("Can't write"); 
            break; 
    }   
 
    if(close(fd_1) < 0)  {
        perror("Can't close file 1\n"); 
        return 6;
    }
     
    if(close(fd_2) < 0) {
        perror("Can't close file 2\n"); 
        return 7;
    }

    return 0;
}