#include <stdio.h>  
#include <fcntl.h> 
#include <sys/stat.h>
#include <mqueue.h> 
#include <signal.h>  
#include <errno.h>  
#include <stdlib.h> 
#include <string.h>

mqd_t queue_fd;  
char* queue_name;

void handler(int signum) {
    mq_close(queue_fd); 
    mq_unlink(queue_name);  
    if(errno == 0) {
         exit(0);
    } else {
        exit(1); 
    }
}

int main(int argc, char** argv) {    
    struct sigaction sa; 
    sigemptyset(&sa.sa_mask); 
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART; 
    if(sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction"); 
        return 2; 
    }
    
    char str[256]; 
    snprintf(str, sizeof(argv[1]), "/%s", argv[1]); 
    //printf("%s\n", str); 
    queue_name = str;
    queue_fd = mq_open(str, O_CREAT | O_EXCL | O_RDONLY, 0666, NULL); 
    if(queue_fd == (mqd_t) - 1) {
        perror("mq_open");
        return 3;  
    }   

    struct mq_attr attr; 
    if(mq_getattr(queue_fd, &attr) < 0) {
        perror("mq_getattr"); 
        return 4; 
    } 
    long msg_size = attr.mq_msgsize; 
    char* buffer = malloc(msg_size + 1);  
    if(buffer == NULL) {
        perror("malloc"); 
        return 5;
    }
    ssize_t read_bytes = 0; 
    unsigned int prio = 0;  
    while(1) { 
        read_bytes = mq_receive(queue_fd, buffer, msg_size, &prio);   
        if(read_bytes < 0) {
            if(errno == EINVAL) {
                free(buffer); 
                if(mq_close(queue_fd) < 0) {
                    perror("mq_close");
                    return 6;
                } 
                if(mq_unlink(queue_name) < 0) {
                    perror("mq_inlink");
                    return 7;
                }
                break;
            } else {
                perror("mq_receive");
                return 8; 
            }
        } 
        buffer[read_bytes] = '\0';
        printf("%s\n", buffer);
    }
    return 0; 
}