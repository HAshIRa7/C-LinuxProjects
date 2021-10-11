#include <dirent.h>     /* Определяет константы DT_* */
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)
struct linux_dirent {
    long           d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    char           d_name[];
};
#define BUF_SIZE 1024 

using namespace std;

char mode_letter(unsigned mode) { 

    switch (mode & S_IFMT) {
           case S_IFBLK:  return 'b';
           case S_IFCHR:  return 'c';
           case S_IFDIR:  return 'd';
           case S_IFIFO: return 'f';
           case S_IFLNK:  return 'l';
           case S_IFREG:  return 'r';
           case S_IFSOCK: return 's';
           default:       return '?';
    } 
} 

char dtype_letter(unsigned dtype) { 

    switch(dtype) {
        case DT_BLK : return 'b'; break;    

        case DT_CHR : return 'c'; break;

        case DT_DIR : return 'd'; break;   

        case DT_FIFO : return 'f'; break; 

        case DT_LNK : return 'l'; break;

        case DT_REG : return 'r'; break;    

        case DT_SOCK : return 's'; break;    

        case DT_UNKNOWN : return '?'; break;

        default : return '-';
    }

} 

int main(int argc, char *argv[])
{
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;
    int bpos; 
    char d_type; 
    fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        handle_error("open");
    while(true) {
        nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
        if (nread == -1)
            handle_error("getdents");
        if (nread == 0)
            break;
        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent *) (buf + bpos);
            d_type = *(buf + bpos + d->d_reclen - 1); ///
            char type = dtype_letter(d_type);             
            if(type == '?') {
                struct stat sb; 
                if(lstat(d->d_name, &sb) == 0) {
                type = mode_letter(sb.st_mode);
            }
        }   
            printf("%c %s\n", type, d->d_name);
            bpos += d->d_reclen;
        }
    }
    exit(EXIT_SUCCESS);
}