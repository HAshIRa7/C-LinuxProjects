#include <stdio.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   
#include <string.h>  
#include <signal.h> 
#include <errno.h>
#include <sys/inotify.h> 
#include <fcntl.h> 
#include <limits.h> 
#include <set> 

int dir_fd, inotify_fd;
std::set<uint32_t> cookies;

void handler(int signo , siginfo_t *info , void *context) { 
    close(inotify_fd);
    close(dir_fd); 
    exit(0);
} 
int HandleEvents(int dir_fd, int inotify_fd); 
static void displayInotifyEvent(struct inotify_event *event, int dir_fd); 

int main(int argc, char** argv) {  
    
    if(argc != 2) {
        printf("Usage: Invalid arguments for %s", argv[0]);
        return 1;
    }

    int wd;

    struct sigaction sa; 
    sigemptyset(&sa.sa_mask); 
    sa.sa_sigaction = handler; 
    sa.sa_flags = SA_RESTART | SA_SIGINFO; 
    if(sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction\n"); 
        return 2;
    } 

    dir_fd = open(argv[1], O_DIRECTORY | O_RDONLY);  
    if(dir_fd < 0) {
        perror("open\n"); 
        return 3; 
    }  

    inotify_fd = inotify_init();  
    if(inotify_fd < 0) {
        perror("inotify_init() function\n");
        return 4;
    } 
    wd = inotify_add_watch(inotify_fd, argv[1], IN_CREATE | IN_MOVE);
    if(wd < 0) {
        perror("inotify_add_watch\n"); 
        return 5; 
    }

    int code = HandleEvents(dir_fd, inotify_fd); 
    return 0;
} 

int HandleEvents(int dir_fd, int inotify_fd) { 
    struct inotify_event *event; 
    char* p; 
    ssize_t NumRead;  
    size_t buf_size = sizeof(inotify_event) + NAME_MAX + 1;
    for(;;) { 
        char buf[buf_size];
        NumRead = read(inotify_fd, buf, buf_size);  
        if(NumRead < 0) {
            printf("ReadEvent went wrong!\n");
            return 6;
        } 
        for(p = buf; p < buf + NumRead; ) {
            event = (struct inotify_event*) p;
            displayInotifyEvent(event, dir_fd);
            p += sizeof(inotify_event) + event->len; 
        }
    }
}

static void displayInotifyEvent(struct inotify_event *event, int dir_fd) {  
    if(event->mask & IN_CREATE) {
        printf("%s\n", event->name);  
    } 
    if(event->cookie > 0) {
        if(event->mask & IN_MOVED_FROM) {
            cookies.insert(event->cookie);
        } 
        if(event->mask & IN_MOVED_TO) { 
            int tr = 1; // если нет такой
            if(cookies.find(event->cookie) != cookies.end()) {
                cookies.erase(cookies.find(event->cookie));
                tr = 0;
            }
            if(tr == 1) {
                printf("%s\n", event->name);
            }
        }
    }  
    return;
}