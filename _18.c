#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <errno.h>

enum
{
    DEC = 10,
    MODE = 0755
};

int main(int argc, char *argv[])
{   
    errno = 0; 
    double left = strtod(argv[1], NULL); 
    if(errno == ERANGE) {
        perror("Incorrect lower bound");
        return 1; 
    }
    double right = strtod(argv[2], NULL); 
    if(errno == ERANGE) {
        perror("Incorrect upper bound");
        return 1; 
    }
    long long int n = strtoll(argv[3], NULL, DEC); 
    if(errno == ERANGE) {
        perror("Incorrect number of segments"); 
        return 1; 
    }
    double dx = (right - left) / n;
    double square = 0.0;
    int fd = open("prog.c", O_WRONLY | O_CREAT | O_TRUNC, MODE);
    if (fd < 0) { 
        perror("failed open file");
        return 2;
    }
    char prog_start[] = "#include <math.h>\n\ndouble function(double x)\n{\n    return ";
    if (write(fd, prog_start, sizeof(prog_start) - 1) != sizeof(prog_start) - 1) { 
        perror("failed to write");
        return 3;
    }
    if (write(fd, argv[4], strlen(argv[4])) != strlen(argv[4])) { 
        perror("failed to write");
        return 3;
    }
    char prog_end[] = ";\n}\n";
    if (write(fd, prog_end, sizeof(prog_end) - 1) != sizeof(prog_end) - 1) { 
        perror("failed to write");
        return 3;
    }
    pid_t child_pid = fork();
    if (child_pid == 0) {
        execlp("gcc", "gcc", "-shared", "prog.c", "-o", "libmyfunc.so", "-lm", NULL);
        _exit(1);
    } else if (child_pid < 0){ 
        perror("failed to fork");
        return 4;
    } else {
        wait(NULL);
    }
    (void) dlerror(); 
    void *handle = dlopen("./libmyfunc.so", RTLD_LAZY); 
    if(dlerror() != NULL) {
        perror("dlopen"); 
        return 5;
    } 
    // *(void **) (&funcp) = dlsym(handle, symbol);
    double (*function)(double);
    *(void **) (&function) = dlsym(handle, "function");  
    if(dlerror() != NULL) {
        perror("dlsym");
    }
    for (int i = 0; i < n; ++i) {
        double x = left + i * dx;
        square += (function(x)) * dx;
    }
    printf("%.10g\n", square);
    dlclose(handle);
    remove("./prog.c");
    remove("./libmyfunc.so");
    close(fd);
    return 0;
}