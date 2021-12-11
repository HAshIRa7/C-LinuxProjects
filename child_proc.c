#define _GNU_SOURCE 
#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>  
#include <sys/wait.h>
#include <sys/stat.h> 
#include <stdlib.h> 
#include <signal.h>
#include <string.h>

int main(void) { 
    pid_t child_id = fork(); 
    if(child_id < 0) {
        perror("fork");
        return 1;
    }  

    if(child_id == 0) {
        printf("Child process "); 
        pid_t child = getpid(); 
        printf("PID = %d\n", child);
        sleep(60);
    } else {
        int status = 0; 
        if(waitpid(child_id, &status, WUNTRACED | WCONTINUED) < 0) {
            perror("waitpid"); 
            return 2; 
        }  
        while(WIFEXITED(status) == 0 && WIFSIGNALED(status) == 0) {
            if(WIFSTOPPED(status)) {
                printf("Child with PID = %d was stopped by signal %d (%s)\n", child_id, WSTOPSIG(status), sigdescr_np(WTERMSIG(status)));     
            } 
            if(WIFCONTINUED(status)) {
                printf("Child with PID = %d continue to work\n", child_id);
            } 
            if(waitpid(child_id, &status, WUNTRACED | WCONTINUED) < 0) {
                perror("waitpid"); 
                return 3;
            }
        }
        if(WIFEXITED(status)) { 
            printf("Child process with PID = %d exited with code %d\n", child_id, WEXITSTATUS(status));
        } 
        else if(WIFSIGNALED(status)) {
            int child_signal = WTERMSIG(status);
            printf("Child with PID = %d was killed by signal %d (%s)\n",  child_id, child_signal, sigdescr_np(child_signal));
            if(WCOREDUMP(status)) {
                printf("core dump\n");
            }
        }
    }
    return 0;
}