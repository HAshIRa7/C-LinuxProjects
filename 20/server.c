#include <sys/mman.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h> 
#include <errno.h>

#define BUF_SIZE 512

struct shmbuf {
	sem_t  sem1;            /* POSIX unnamed semaphore */ 
	size_t cnt;             /* Number of bytes used in 'buf' */
	char   buf[BUF_SIZE];   /* Data being transferred */
};
int done = 0;
void term(int signum) {
	done = 1;
}
int main (int argc, char* argv[]){
	if(argc > 1) {
		printf("Too many arguments\n");
		printf("Usage: %s\n", argv[0]);
		return 1;
	}
	const char *shm_name = "/clock2";
	int fd;
	/* Open an Shared Memory Object for Read-/Write-Access */
	if((fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0644)) < 0) {
		perror("\nshm_open() in Caretaker failed");
		return 2;
	}
	/* Truncate Shared Memory Object to specific size */
	if((ftruncate(fd, sysconf(_SC_PAGE_SIZE)) < 0)) {
		perror("\nftruncate() in Caretaker failed");
		return 3;
	}

	struct shmbuf *shmp = mmap(
			NULL,
			sizeof(*shmp),
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			0
			);
	/* Create standart shmbuf*/
	if (shmp == MAP_FAILED) {
		perror("mmap");
		return 4;
	}
	time_t timer;
	struct tm* tm_info;
	/*check sem is initialized*/
	if (sem_init(&shmp->sem1, 1, 1) == -1){
		perror("sem init");
		return 5;
	}
	struct sigaction action = {};
	action.sa_flags = 0;
	action.sa_handler = term;
	if(sigaction(SIGINT, &action, NULL) < 0) {
		perror("sigaction");
		psignal(SIGINT, NULL);
	}
	if(sigaction(SIGTERM, &action, NULL) < 0) {
		perror("sigaction");
		psignal(SIGTERM, NULL);
	}
	while(!done){
		sleep(1);
		if (sem_wait(&shmp->sem1) == -1){
			perror("sem init");
			sem_destroy(&shmp->sem1);
			return 6;
		}
        if(errno == EINTR) {
            break;
        }
		timer = time(NULL);
		tm_info = localtime(&timer);
		strftime(shmp->buf, sizeof(shmp->buf), "%Y-%m-%d %H:%M:%S", tm_info); /*set format time*/
		printf("Now is %s\n", shmp->buf);
		if (sem_post(&shmp->sem1) == -1){
			perror("sem post");
			sem_destroy(&shmp->sem1);
			return 7;
		}
	} 
    if(errno == EINTR) {
        if (sem_post(&shmp->sem1) == -1){
		    perror("sem post");
		    sem_destroy(&shmp->sem1);
		    return 7;
	    }
    }
    if(shm_unlink(shm_name) < 0) {
        perror("shm_unlink"); 
        return 8;
    }/*remove shared memory*/
	if(sem_destroy(&shmp->sem1) < 0) {
        perror("sem_destroy"); 
        return 9; 
    }
	return 0;
}