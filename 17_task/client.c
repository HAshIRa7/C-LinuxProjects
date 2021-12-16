#include <stdio.h> 
#include <mqueue.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <errno.h> 
#include <string.h>

int main(int argc, char** argv) {  

    if(argc != 3) {
        printf ("Usage: %s /destination_name \"message\"\n", argv[0]);
        return 1;
    } 
    char str[256]; 
    snprintf(str, sizeof(argv[1]), "/%s", argv[1]); 
    //printf("%s\n", str);
    mqd_t queue_fd = mq_open(str, O_WRONLY); 
    if(queue_fd < 0) {
        perror("mq_open"); 
        return 1; 
    } 

    int code = 0;
    if(mq_send(queue_fd, argv[2], strlen(argv[2]), 2) < 0) {
        perror("mq_send");  
        code = 2;  
    } 
    if(mq_close(queue_fd) < 0) {
        perror("mq_close");  
        code = 3; 
    }
    return code;
}