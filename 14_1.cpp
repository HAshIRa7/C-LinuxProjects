#include <unistd.h> 
#include <sys/types.h> 
#include <stdio.h>  
#include <sys/wait.h> 
#include <signal.h> 
#include <stdlib.h> 
#include <string.h>

void proc_info(const char* procname) {

    printf("%s: PID %d, PPID %d, PGID %d, SID %d\n", procname, 
    getpid(), getppid(), getpgid(0), getsid(0));  

}

int main(void) {

    pid_t child_id = fork(); 

    if(child_id < 0) {
        perror("fork"); 
        return 1;
    } 
    
    //this code is executed in child process only   
    if(child_id == 0) {
        if(dup2(fileno(stderr), fileno(stdout)) < 0) {
            perror("dup2");
            return 1;
        }
        proc_info("child");
        return 0;
    } 
        proc_info("parent"); 
        wait(NULL); 

    return 0;
}