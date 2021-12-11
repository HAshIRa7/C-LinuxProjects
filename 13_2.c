#include <stdio.h>   
#include <sys/syscall.h> 
#include <unistd.h> 
#include <poll.h>

int main(void) { 
    pid_t child_id = fork(); 
    if(child_id < 0) {
        perror("fork"); 
        return 1; 
    } 

    if(child_id == 0) { 
        int parent_pid = syscall(SYS_pidfd_open, getppid(), NULL);
        struct pollfd pollfd; 
        pollfd.fd = parent_pid; 
        pollfd.events = POLLIN; 
        printf("Waiting for parent procces...\n");
        int ready = poll(&pollfd, 1, -1); 
        if(ready < 0) {
            perror("poll");
            return 2;
        } 
        printf("Events (%#x): POLLIN is %sset\n", pollfd.revents,
                   (pollfd.revents & POLLIN) ? "" : "not ");
        close(parent_pid);
    } else {
        printf("This is parent proccess with PID = %d\n", getpid()); 
        sleep(10);
    }
    return 0;
}