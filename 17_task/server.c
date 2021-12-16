#include <stdio.h>  
#include <fcntl.h> 
#include <sys/stat.h>
#include <mqueue.h> 
#include <signal.h>  
#include <errno.h>  
#include <stdlib.h> 
#include <string.h>

void handler(int signum) {}

int main(int argc, char** argv) {     
    if(argc != 2) {
        printf("Usage: %s open server", argv[0]);
    }

    struct sigaction sa; 
    sigemptyset(&sa.sa_mask); 
    sa.sa_handler = handler;
    sa.sa_flags = 0; 
    if(sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction"); 
        return 2; 
    }

    const char* queue_name = argv[1];
    mqd_t queue_fd = mq_open(queue_name, O_CREAT | O_EXCL | O_RDONLY, 0622, NULL); // писать могут все а читать только владелец
    if(queue_fd == (mqd_t)-1) {
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
            break;
        }  
        buffer[read_bytes] = '\0';
        printf("%s\n", buffer);
    }  
    if(errno != EINTR && errno != EAGAIN) {
        perror("mq_receive"); 
    }
    free(buffer); 
    if(mq_close(queue_fd) < 0) {
        perror("mq_close");
        return 6;
    } 
    if(mq_unlink(queue_name) < 0) {
        perror("mq_inlink");
        return 7;
    }
    return 0; 
}