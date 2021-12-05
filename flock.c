#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h> 

extern int errno; 

int main(void) {
    char* cwd = get_current_dir_name();
    int cwd_fd = open(cwd, O_DIRECTORY);
    if (cwd_fd < 0) {
        perror("Could not open program directory");
        free(cwd);
        return 1;
    }
    free(cwd);

    int count_fd = openat(cwd_fd, "count.txt", O_CREAT | O_RDWR, 0600);
    if (lockf(count_fd, F_LOCK, 0)) {
        perror("lockf");
        close(count_fd);
        close(cwd_fd);
        return 2;
    }
    struct stat statbuf;
    if (fstatat(cwd_fd, "count.txt", &statbuf, 0) == -1) {
        perror("stat(\"count.txt\")");
        close(count_fd);
        close(cwd_fd);
        return 3;
    }
    close(cwd_fd);
    long count= 0;
    if (statbuf.st_size == 0)
        count = 1;
    else {
        char strbuf[16] = "";
        ssize_t num_read = read(count_fd, strbuf, (size_t)statbuf.st_size);
        if (num_read != statbuf.st_size) {
            perror("read");
            close(count_fd);
            return 4;
        }
        count = strtol(strbuf, NULL, 10);
        if (errno) {
            perror("strtol: conversion failed");
            close(count_fd);
            return 5;
        }
        count += 1;
    }
    FILE* fp_counter = fdopen(count_fd, "w+");
    rewind(fp_counter);
    fprintf(fp_counter, "%li", count);
    if (fflush(fp_counter)) {
        perror("Failed to write data");
        fclose(fp_counter);
        return 6;
    }
    if (lockf(count_fd, F_ULOCK, 0)) {
        perror("flock");
        fclose(fp_counter);
        return 7;
    }
    if (fclose(fp_counter)) {
        perror("fclose");
        return 8;
    }
    return 0;
}