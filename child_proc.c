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

extern int errno; 

mode_t read_umask(void) {
    mode_t mask = umask(0);
    printf("Umask = [%04x]\n", mask);
    umask(mask);
    return mask;
}

void proc_info(const char* proc_name) {
    pid_t th_pid = getpid();
    pid_t th_ppid = getppid();
    gid_t th_gid = getgid();
    gid_t th_egid = getegid();
    pid_t th_sid = getsid(th_pid);
    printf("%s: \n", proc_name);
    printf(
        "PID = [%d], Parent PID = [%d], GID = [%d], EGID = [%d], SID = [%d]\n",
        th_pid, th_ppid, th_gid, th_egid, th_sid
    );

    read_umask();

    char* workdir = get_current_dir_name();
    if (workdir == NULL) {
        puts("CWD: ???");
    } else {
        printf("CWD: '%s'\n", workdir);
        free(workdir);
    }
}

int main(void) { 
    pid_t child_id = fork(); 

    if(child_id < 0) {
        perror("fork");
        return 1;
    }  

    if(child_id == 0) {
        proc_info("Child process");
    } else {
        int status = 0; 
        if(waitpid(child_id, &status, WUNTRACED) < 0) {
            perror("waitpid"); 
            return 2; 
        } 
        if(WIFEXITED(status)) { 
            printf("Child process with PID = %d exited with code %d\n", child_id, WEXITSTATUS(status));
        } 
        else if(WIFSIGNALED(status)){
            int child_signal = WTERMSIG(status);
            printf("Child with PID = %d has exited with code %d (%s)\n",  child_id, WTERMSIG(status), sigdescr_np(child_signal));
        } else {
            printf("Child with PID = %d has exited with status %d\n", child_id, status);
        }
    }
    return 0;
}