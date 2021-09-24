#include<stdio.h> 
#include <sys/types.h>
#include <linux/stat.h>
#include <sys/stat.h> 
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>    
#include <linux/fcntl.h>  
#include <unistd.h>   
#include <string.h>  
#include <pwd.h>  
#include <grp.h>


int statx(int dirfd, const char *pathname, int flags,
                 unsigned int mask, struct statx *statxbuf); 


const char* type(struct statx* st) { 

    switch(st->stx_mode & S_IFMT) { 
        case S_IFLNK:  return("символьная ссылка"); 
                   case S_IFREG:  return("regular file");
                   case S_IFDIR:  return("catalog"); 
                   case S_IFCHR:  return("regular device"); 
                   case S_IFIFO:  return("FIFO"); 
                   case S_IFSOCK: return("soket"); break;   

                   default: return("unknown");  
    }
}   


static void print_time(const char *field, struct statx_timestamp *ts)
{
	struct tm tm;
	time_t tim;
	char buffer[100];
	int len;

	tim = ts->tv_sec; 
    tm = *localtime(&tim); 
	len = strftime(buffer, 100, "%F %T", &tm);
	printf("%s", field);
	fwrite(buffer, 1, len, stdout);
	printf(".%09u", ts->tv_nsec);
	len = strftime(buffer, 100, "%z", &tm);
	fwrite(buffer, 1, len, stdout);
    printf("\n");
}

 char *          /* Return name corresponding to 'uid', or NULL on error */
 userNameFromId(uid_t uid)
 {
     struct passwd *pwd;
 
     pwd = getpwuid(uid);
     return (pwd == NULL) ? NULL : pwd->pw_name;
 } 

char* 
GroupNameFromId(gid_t gid) {
    struct group *grp;  

    grp = getgrgid(gid); 
    return (grp == NULL) ? NULL : grp->gr_name;  
}



int main(int argc, char* argv[]) { 

    struct statx st;     

    statx(AT_FDCWD, argv[1], AT_STATX_SYNC_AS_STAT, STATX_ALL, &st); 


    printf("File name %s\n", argv[1]);  

    printf("Size: %u\t", (uint32_t)st.stx_size); 

    printf("Blocks: %u\t", (uint32_t)st.stx_blocks); 

    printf("Block size: %u\t", (uint32_t)st.stx_blksize);  

    puts(type(&st));  
    
    printf("Device: %u %u\t", (uint32_t)st.stx_dev_major, (uint32_t)st.stx_dev_minor);  

    printf("Inode: %u\t", (uint32_t)st.stx_ino);  

    printf("Links: %u", (uint32_t)st.stx_nlink);  

    printf("\n");  
 
    printf("Access: (0%jo/",  (uintmax_t)(st.stx_mode & 0777));   

    printf(((st.stx_mode & S_IFMT) & S_IFDIR) ? "d" : "-"); 


    for(int i  = 0; i < 9; i++) {
        putchar(((st.stx_mode & 0777) & (256 >> i)) ? "rwx"[i % 3] : '-'); 
    }  

    printf(")\t");  
     
    printf("Uid (%u / %s)\t", (uint32_t)st.stx_uid, userNameFromId(st.stx_uid));    

    printf("Gid (%u / %s)\t", (uint32_t)st.stx_gid, GroupNameFromId(st.stx_gid));    

    char res[32];

    printf("\n");

   // printf("%lld.%09u\n", st.stx_btime.tv_sec, st.stx_btime.tv_nsec);  

 
    print_time("Access to file:\t", &st.stx_atime);  // stx_atime
    print_time("Modification: \t", &st.stx_mtime); // stx_mtime 
    print_time("Change: \t", &st.stx_ctime);//stx_ctime 
    print_time("Create: \t", &st.stx_btime); // stx_btime; 


   // printf("\n");    
    return 0;
} 