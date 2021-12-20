#include <stdio.h>
#include <sys/vfs.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/statvfs.h>

void print_info(struct statvfs* sb) {
    printf("Total:      %ju\n", (uintmax_t)sb->f_blocks * sb->f_bsize);
    printf("Total:      %ju\n", (uintmax_t)sb->f_blocks * sb->f_frsize);
    printf("Available:  %ju\n", (uintmax_t)sb->f_bavail * sb->f_bsize);
    printf("Available:  %ju\n", (uintmax_t)sb->f_bavail * sb->f_frsize);
    printf("Used:       %ju\n", (uintmax_t)sb->f_bsize * (sb->f_blocks  - sb->f_bavail)); // total - available
    printf("Used:       %ju\n", (uintmax_t)sb->f_frsize * (sb->f_blocks  - sb->f_bfree));
}


int main(int argc, char* argv[]) {
    if(argc < 2) { 
        printf("Usage %s [file_1] [file_2]...",  argv[0]);
        return 1;
    }
    struct statvfs sb;
    for(int i = 1; i < argc; i++) {
        if(statvfs(argv[i], &sb) == 0) {
            print_info(&sb);
        }
    }
    return 0;
}