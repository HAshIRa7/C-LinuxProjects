#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage: %s [function] [lower bound] [upper bound]\n", argv[0]);
        return 1;
    }
    errno = 0;
    double lb = strtod(argv[2], NULL);
    if (errno == ERANGE) {
        printf("Incorrect lower bound value.\n");
        return 1;
    }
    double ub = strtod(argv[3], NULL);
    if (errno == ERANGE) {
        printf("Incorrect upper bound value.\n");
        return 1;
    }

    char lib_link_name[] = "./funcXXXXXX"; // './' prefix if used by dlopen function
    char* lib_file_name = lib_link_name + 2; // '+2' since we need to skip './' prefix
    int lib_fd = mkstemp(lib_file_name);  
    if (lib_fd == -1) {
        perror("Could not create temporary file");
        return 1;
    }

    int gcc_pipe[2];
    if (pipe(gcc_pipe)) {
        perror("pipe");
        close(lib_fd);
        unlink(lib_file_name);
        return 1;
    }

    pid_t child_pid = fork();
    if (child_pid < 0) { //fork failed
        perror("fork");
        close(lib_fd);
        unlink(lib_file_name);
        return 2;
    } else if (child_pid == 0) { //child only section
        close(gcc_pipe[1]);
        int gcc_inputfd = dup2(gcc_pipe[0], STDIN_FILENO);
        if (gcc_inputfd < 0) {
            perror("dup2");
            close(gcc_pipe[0]);
            close(lib_fd);
            return 1;
        }
        int gcc_outputfd = dup2(lib_fd, STDOUT_FILENO);
        if (gcc_outputfd < 0) {
            perror("dup2");
            close(gcc_pipe[0]);
            close(lib_fd);
            return 1;
        }
        close(gcc_pipe[0]);
        execlp(
            "gcc", "gcc", "-Wall", "-Wextra", "-O3",
            "-fPIC", "-fPIE", "-shared",
            "-xc", "-", "-lm", "-o", lib_file_name,
            NULL
        );
        perror("Exec GCC");
        return 1;
    } //end of child section
    close(gcc_pipe[0]);
    close(lib_fd);
    FILE* gcc_istream = fdopen(gcc_pipe[1], "w");
    fprintf(
        gcc_istream, 
        "#include <math.h>\n"
        "#define f(x) %s\n"
        "double integr(double lb, double ub) {\n"
        "double step = 0.001;\n"
        "unsigned long iter = 0;\n"
        "double x = 0.0;\n"
        "double i = 0.0;\n"
        "while (x < ub) {\n"
        "    i += step * (f(x) + f((x + step))) / 2;\n"
        "    iter += 1;\n"
        "    x = lb + step * iter;\n"
        "}\n"
        "return i;\n"
        "}\n",
        argv[1]
    );
    if (fclose(gcc_istream)) {
        perror("fclose");
        close(gcc_pipe[1]);
        unlink(lib_file_name);
        return 1;
    }
    int status;
    waitpid(child_pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
        printf("Error, gcc exited with code %d\n", WEXITSTATUS(status));
        unlink(lib_file_name);
        return 1;
    }

    void* lib = dlopen(lib_link_name, RTLD_LAZY);
    char* dlerrstr;
    if(lib == NULL) {
        perror(dlerror());
        unlink(lib_file_name);
        return 4;
    }

    double (*fun)(double, double);
    *(void **) (&fun) = dlsym(lib, "integr");
    if ((dlerrstr = dlerror()) != NULL) {
        puts(dlerrstr);
        dlclose(lib);
        unlink(lib_file_name);
        return 1;
    }
    double res;
    if (lb < ub)
        res = (*fun)(lb, ub);
    else
        res = -(*fun)(ub, lb);
    printf("%lf\n", res);

    dlclose(lib);
    unlink(lib_file_name);
    return 0;
}