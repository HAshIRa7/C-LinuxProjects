#define _GNU_SOURCE
#include <stdio.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   
#include <string.h>  
#include <signal.h> 
#include <errno.h>
#include <sys/inotify.h> 
#include <sys/syscall.h>
#include <fcntl.h> 
#include <limits.h>
#define LEN (sizeof(struct inotify_event) + NAME_MAX + 1)  
#define TIME "%a %Y-%m-%d %H:%M:%S"

/*struct inotify_event {
               int      wd;       // Watch descriptor 
               uint32_t mask;     // Mask describing event 
               uint32_t cookie;   // Unique cookie associating related
                                  // events (for rename(2)) 
               uint32_t len;      // Size of name field 
               char     name[];   // Optional null-terminated name 
           };
*/

extern int errno;  

char* support = NULL;

void handler(int sig) {
    free(support);
    exit(0);  
}  

int HandleEvents(int dir_fd, int inotify_fd); 
ssize_t ReadEvent(int inotify_fd, char* buf, size_t *buf_size); 
static void displayInotifyEvent(struct inotify_event *event, int dir_fd); 
static void PrintTime(struct statx_timestamp *stx_btime, char* name);

int main(int argc, char** argv) {  
    
    if(argc != 2) {
        printf("Usage: Invalid arguments for %s", argv[0]);
        return 1;
    }

    int inotify_fd, wd;

    struct sigaction sa = {}; 
    sa.sa_handler = &handler; 
    sigemptyset(&sa.sa_mask); 
    if(sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction\n"); 
        return 2;
    } 

    int dir_fd = open(argv[1], O_DIRECTORY | O_RDONLY);  
    if(dir_fd < 0) {
        perror("open\n"); 
        return 3; 
    }  

    inotify_fd = inotify_init();  
    if(inotify_fd < 0) {
        perror("inotify_init() function\n");
        return 4;
    } 

    wd = inotify_add_watch(inotify_fd, argv[1], IN_CREATE);
    if(wd < 0) {
        perror("inotify_add_watch\n"); 
        return 5; 
    } 
     
    int code = HandleEvents(dir_fd, inotify_fd); 
    return 0;
} 

int HandleEvents(int dir_fd, int inotify_fd) { 
    size_t buf_size = LEN; 
    char* buf = malloc(LEN);  
    support = buf;
    if(buf == NULL) {
        perror("malloc"); 
    }  
    struct inotify_event *event; 
    char* p; 
    ssize_t NumRead; 
    for(;;) {
        NumRead = ReadEvent(inotify_fd, buf, &buf_size);  
        if(NumRead < 0) {
            printf("ReadEvent went wrong!\n");
            return 6;
        } 
        for(p = buf; p < buf + NumRead; ) {
            event = (struct inotify_event*) p;
            displayInotifyEvent(event, dir_fd);
            p += sizeof(struct inotify_event) + event->len; 
        }
    }
} 

ssize_t ReadEvent(int inotify_fd, char* buf, size_t *buf_size) {
    ssize_t result = read(inotify_fd, buf, *buf_size); 
    if(result < 0) {
        if(errno == EINVAL) {
            *buf_size *= 2;
            buf = realloc(buf, *buf_size);  
            if(buf == NULL) {
                free(buf);
                return -1;
            }
            result = read(inotify_fd, buf, *buf_size); 
            if(result < 0) {
                perror("read\n"); 
            }
        } else {
            perror("read"); 
            free(buf);
            return -1; 
        }
    } 
    return result;
} 

static void displayInotifyEvent(struct inotify_event *event, int dir_fd) {  
    if(event->mask == IN_CREATE) {
        struct statx statbuf; 
        if(syscall(SYS_statx, dir_fd, event->name, AT_SYMLINK_NOFOLLOW, STATX_BTIME, &statbuf)); 
        PrintTime(&statbuf.stx_btime, event->name);
    }
    return;
} 

static void PrintTime(struct statx_timestamp *stx_btime, char* name) { 
    struct tm tm; 
    time_t btime; 
    char time_buf[100]; 
    
    btime = stx_btime->tv_sec; 
    tm = *localtime(&btime); 
    strftime(time_buf, sizeof(time_buf), TIME, &tm); 
    printf("[%s] File %s was created\n", time_buf, name);
    return; 
}