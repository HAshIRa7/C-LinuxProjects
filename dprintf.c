#include <stdio.h> 
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>



int main(int argc, char* argv[]){  

    // три аргумента 0 - испольняемый файл  
    // 1 - в какой файл записываем 
    // 2 - что записываем 

    if(argc != 3) { 
        printf("Ooops I need file and string\n"); 
        return 1;
    }  

    int flags = O_WRONLY | O_CREAT | O_TRUNC;   
     
    int fd = open(argv[1], flags, 0644); 

    if (fd < 0) { 
        perror("Failed to open file for writing\n"); 
        return 2; 
    } 

    if(dprintf(fd, "%s\n",  argv[2]) < 0){ 
        perror("Can't write in file\n"); 
        return 3;  
    } 

    if (close(fd) < 0) {
        perror("Can't close file\n"); 
        return 4;  
    }


    return 0;
}