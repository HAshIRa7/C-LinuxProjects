#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h>  

// last | wc -l 

int main(void) {

    int pipe_fds[2];
    if (pipe(pipe_fds) < 0) {
        perror("pipe");
        return 1; 
    } 
    // сам пайп объект ядра, поэтому пайп не надо закрывать  
    // поэтому когда закрываем файлы пайпа, ядро само его грохает 
    // c именовыанным пайпом по другому 
    pid_t child_id = fork(); 
    if(child_id < 0) {
        perror("fork"); 
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return 1; 
    } 

    // this code executed in child
    if(child_id == 0) {
        //closed unused reading endpoint of the pipe
        close(pipe_fds[0]);  
        //redirect stdout to pipe
        if(dup2(pipe_fds[1], fileno(stdout)) < 0) {
            perror("dup2"); 
            close(pipe_fds[1]);
            return 1; 
        } 
        close(pipe_fds[1]); 
        // execute 'last' 
        execlp(
            //filename to execute
            "last", 
            // argv[0], argv[1] 
            "last", NULL
        ); 
        perror("execlp"); 
        return 1;
    }  
    close(pipe_fds[1]);  
    if(dup2(pipe_fds[0], fileno(stdin)) < 0) {
        perror("dup2"); 
        close(pipe_fds[0]);
        return 1; 
    }  

    close(pipe_fds[0]); 
    //execute wc -l
    execlp(
        //filename to execute 
        "wc", 
        // argv[0], argv[1], ...
        "wc", "-l", NULL
    );
    perror("failed to exec 'wc -l'"); 
    return 0;
}