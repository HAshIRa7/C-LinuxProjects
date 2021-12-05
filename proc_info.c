#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pwd.h>
#include <stdlib.h>
#include <errno.h>
#include <grp.h>  
#include <limits.h> 

mode_t read_umask(void) {
    mode_t mask = umask(0);
    printf("Umask = [%04x]\n", mask);
    umask(mask);
    return mask;
}

int main()
{
    pid_t th_pid = getpid();
    pid_t th_ppid = getppid();
    gid_t th_gid = getgid();
    gid_t th_egid = getegid();
    pid_t th_sid = getsid(th_pid);
    printf(
        "PID = [%d], Parent PID = [%d], GID = [%d], EGID = [%d], SID = [%d]\n",
        th_pid, th_ppid, th_gid, th_egid, th_sid
    );

    read_umask();

    char* workdir = get_current_dir_name();
    if (workdir == NULL) {
        puts("CWD: ???");
    } else {
        printf("CWD: '%s'\n", workdir);
        free(workdir);
    }

    errno = 0;
    int priority = getpriority(PRIO_PROCESS, th_pid);
    if ((priority == -1) && (errno != 0)) {
        perror("getpriority");
        return 4;
    }
    printf("Scheduling priority: [%d]\n", priority); 

    gid_t groups_list[NGROUPS_MAX + 1];
    int groups_list_size = NGROUPS_MAX; 
    int numGroups = getgroups(groups_list_size, groups_list);
    if (numGroups < 0) {
        perror("getgroups");
        return 2;
    }
    struct group* gr_info_p;
    printf("Supplementary groups:");
    for (unsigned i = 0; i < (unsigned)numGroups; i++) {
        printf(" [%d]", groups_list[i]);
        gr_info_p = getgrgid(groups_list[i]);
        if (gr_info_p == NULL)
            printf("-(?)");
        else
            printf("-(%s)", gr_info_p->gr_name);
    }
    putchar('\n');
    struct passwd *pass = getpwuid(getuid());
    printf(
        "User: '%s'\nPassword: '%s'\nUID: [%i]\nGID: [%i]\n", 
        pass->pw_name, pass->pw_passwd, pass->pw_uid, pass->pw_gid
    );
    return 0;
}