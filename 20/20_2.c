#include <stdio.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <sys/mman.h>  
#include <unistd.h>

int main(int argc, char** argv) {    
    if(argc != 2) {
        printf("Usage: %s [file to encrypt]", argv[0]); 
        return 1;
    } 
    int fd = open(argv[1], O_RDWR); 
    if(fd < 0) {
        perror("open"); 
        return 2;
    } 

    struct stat file_stats; 
    fstat(fd, &file_stats); 

    char *buf = mmap(NULL, file_stats.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(buf == MAP_FAILED) {
        perror("MMAP");
        close(fd); 
        return 3; 
    } 

    unsigned i; 
    for(i = 0; i < file_stats.st_size; i++) {
        buf[i] ^= 0xFF; 
    } 

    munmap(buf, file_stats.st_size);
    close(fd); 
    return 0; 
}