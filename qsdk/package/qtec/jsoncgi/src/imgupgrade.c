#include <fwk.h>
#include <librtcfg.h>
#include "sys/resource.h"
#include <sys/wait.h>
#include <fcntl.h>     /* for open */
#include <errno.h>

void* g_mcHandle;

int main()
{
    int ret = 0;
    int i;
    int commFd = -1;
	int maxFd = -1;
	int fd, rv, n;
	fd_set readFdsMaster,rfds;
	VosMsgHeader *msg = NULL;
    struct timeval timenow;
    struct timeval tm;
    int sleepMs = 200;
    int isKeepConfig;
    #if 0
    struct rlimit    rl1;

    if(getrlimit(RLIMIT_NOFILE, &rl1) < 0)
    {
        perror("getrlimit(RLIMIT_NOFILE, &rl)");
        return -1;
    }
    if(rl1.rlim_max == RLIM_INFINITY)
    {
        rl1.rlim_max = 1024;
    }
    for(i = 0; i < rl1.rlim_max; i++)
    {
        close(i);
    }
    #endif
    if (access("/tmp/1.img", F_OK) == 0)
    {
    #if 0
        UTIL_DO_SYSTEM_ACTION("killall smd lighttpd QtecDeviceManager lanHostManager cgimsgmanager myclient scrctrl miniupnpd watchquagga dnsmasq qtec_capture lldpd ");
        struct rlimit    rl;
        int i;
        if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
        {
            perror("getrlimit(RLIMIT_NOFILE, &rl)");
            return -1;
        }
        if(rl.rlim_max == RLIM_INFINITY)
        {
            rl.rlim_max = 1024;
        }
        for(i = 0; i < rl.rlim_max; i++)
        {
            close(i);
        }
        #endif
        if (fork() == 0)
        {
            int devNullFd;
            printf("I am child\n");
            #if 1
            devNullFd = open("/dev/null", O_RDWR);
            close(0);
            dup2(devNullFd, 0);
            close(1);
            dup2(devNullFd, 1);
            close(2);
            dup2(devNullFd, 2);
            if (devNullFd != -1)
            {
               close(devNullFd);
            }
            struct rlimit    rl;
            int i;
            if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
            {
                perror("getrlimit(RLIMIT_NOFILE, &rl)");
                return -1;
            }
            if(rl.rlim_max == RLIM_INFINITY)
            {
                rl.rlim_max = 1024;
            }
            for(i = 0; i < rl.rlim_max; i++)
            {
                close(i);
            }
            #endif
            signal(SIGHUP, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGILL, SIG_DFL);
            signal(SIGTRAP, SIG_DFL);
            signal(SIGABRT, SIG_DFL); 
            signal(SIGFPE, SIG_DFL);
            signal(SIGBUS, SIG_DFL);
            signal(SIGSEGV, SIG_DFL);
            signal(SIGSYS, SIG_DFL);
            signal(SIGPIPE, SIG_DFL);
            signal(SIGALRM, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            signal(SIGUSR1, SIG_DFL);
            signal(SIGUSR2, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);  /* same as SIGCLD */
            signal(SIGPWR, SIG_DFL);
            signal(SIGWINCH, SIG_DFL);
            signal(SIGURG, SIG_DFL);
            signal(SIGIO, SIG_DFL);    /* same as SIGPOLL */
            signal(SIGSTOP, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGCONT, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGVTALRM, SIG_DFL);
            signal(SIGPROF, SIG_DFL);
            signal(SIGXCPU, SIG_DFL);
            signal(SIGXFSZ, SIG_DFL);

            char *newargv[]= {"/bin/sh","-c","cd /tmp;sleep 1; killall smd dropbear uhttpd lighttpd; sleep 1; sysupgrade 1.img", 0};
            char *newenviron[]={NULL};
            char * myargv[] = {"ls", "-al", "/etc/passwd", 0};
            execvp("/bin/sh", newargv);
            printf("I am child , I'm done, errno = %d\n", errno);
        }
        waitpid(-1, NULL, WNOHANG);
        printf("I am father , I'm done\n");
        return 0;
    }

    while (1)
    {
        if (access("/tmp/img_download_ok", F_OK) == 0)
        {
            chdir("/tmp");
            if (fork() == 0)
            {
                int devNullFd;
                printf("I am child\n");
                #if 0
                devNullFd = open("/dev/null", O_RDWR);
                close(0);
                dup2(devNullFd, 0);
                close(1);
                dup2(devNullFd, 1);
                close(2);
                dup2(devNullFd, 2);
                if (devNullFd != -1)
                {
                   close(devNullFd);
                }
                struct rlimit    rl;
                int i;
                if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
                {
                    perror("getrlimit(RLIMIT_NOFILE, &rl)");
                    return -1;
                }
                if(rl.rlim_max == RLIM_INFINITY)
                {
                    rl.rlim_max = 1024;
                }
                for(i = 0; i < rl.rlim_max; i++)
                {
                    close(i);
                }
                #endif
                signal(SIGHUP, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGILL, SIG_DFL);
                signal(SIGTRAP, SIG_DFL);
                signal(SIGABRT, SIG_DFL); 
                signal(SIGFPE, SIG_DFL);
                signal(SIGBUS, SIG_DFL);
                signal(SIGSEGV, SIG_DFL);
                signal(SIGSYS, SIG_DFL);
                signal(SIGPIPE, SIG_DFL);
                signal(SIGALRM, SIG_DFL);
                signal(SIGTERM, SIG_DFL);
                signal(SIGUSR1, SIG_DFL);
                signal(SIGUSR2, SIG_DFL);
                signal(SIGCHLD, SIG_DFL);  /* same as SIGCLD */
                signal(SIGPWR, SIG_DFL);
                signal(SIGWINCH, SIG_DFL);
                signal(SIGURG, SIG_DFL);
                signal(SIGIO, SIG_DFL);    /* same as SIGPOLL */
                signal(SIGSTOP, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCONT, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
                signal(SIGVTALRM, SIG_DFL);
                signal(SIGPROF, SIG_DFL);
                signal(SIGXCPU, SIG_DFL);
                signal(SIGXFSZ, SIG_DFL);

                char *newargv[]= {"/bin/sh","-c","cd /tmp;sleep 1; killall smd dropbear uhttpd lighttpd; sleep 1; sysupgrade firmware.img", 0};
                execvp("/bin/sh", newargv);
                printf("I am child , I'm done, errno = %d\n", errno);
            }
            waitpid(-1, NULL, WNOHANG);
            printf("I am father , I'm done\n");
            return 0;
        }
        sleep(1);
    }
    #if 0
    vosLog_init(EID_IMG_UPGRADE);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
	vosMsg_init(EID_IMG_UPGRADE, &g_mcHandle);

    vosMsg_getEventHandle(g_mcHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;

    

    
    while(1)
    {
        rfds = readFdsMaster;
        tm.tv_sec = sleepMs / MSECS_IN_SEC;
        tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
        n = select(maxFd+1, &rfds, NULL, NULL, &tm);
        if (n < 0)
        {
            printf("msg queue error, exit!");
            break;
        }
        else if (n > 0)
        {
            if (FD_ISSET(commFd, &rfds))
            {
                ret = vosMsg_receive(g_mcHandle, &msg);
                if (ret != VOS_RET_SUCCESS)
                {
                    continue;
                }
                switch(msg->type)
                {
                    case VOS_MSG_UPGRADE_IMG:
                        isKeepConfig = msg->wordData;
                        #if 1
                        if (fork() == 0)
                        {
                            int devNullFd;
                            printf("I am child\n");
                            #if 1
                            devNullFd = open("/dev/null", O_RDWR);
                            close(0);
                            dup2(devNullFd, 0);
                            close(1);
                            dup2(devNullFd, 1);
                            close(2);
                            dup2(devNullFd, 2);
                            if (devNullFd != -1)
                            {
                               close(devNullFd);
                            }
                            struct rlimit    rl;
                            int i;
                            if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
                            {
                                perror("getrlimit(RLIMIT_NOFILE, &rl)");
                                return -1;
                            }
                            if(rl.rlim_max == RLIM_INFINITY)
                            {
                                rl.rlim_max = 1024;
                            }
                            for(i = 0; i < rl.rlim_max; i++)
                            {
                                close(i);
                            }
                            #endif
                            signal(SIGHUP, SIG_DFL);
                            signal(SIGQUIT, SIG_DFL);
                            signal(SIGILL, SIG_DFL);
                            signal(SIGTRAP, SIG_DFL);
                            signal(SIGABRT, SIG_DFL); 
                            signal(SIGFPE, SIG_DFL);
                            signal(SIGBUS, SIG_DFL);
                            signal(SIGSEGV, SIG_DFL);
                            signal(SIGSYS, SIG_DFL);
                            signal(SIGPIPE, SIG_DFL);
                            signal(SIGALRM, SIG_DFL);
                            signal(SIGTERM, SIG_DFL);
                            signal(SIGUSR1, SIG_DFL);
                            signal(SIGUSR2, SIG_DFL);
                            signal(SIGCHLD, SIG_DFL);  /* same as SIGCLD */
                            signal(SIGPWR, SIG_DFL);
                            signal(SIGWINCH, SIG_DFL);
                            signal(SIGURG, SIG_DFL);
                            signal(SIGIO, SIG_DFL);    /* same as SIGPOLL */
                            signal(SIGSTOP, SIG_DFL);
                            signal(SIGTSTP, SIG_DFL);
                            signal(SIGCONT, SIG_DFL);
                            signal(SIGTTIN, SIG_DFL);
                            signal(SIGTTOU, SIG_DFL);
                            signal(SIGVTALRM, SIG_DFL);
                            signal(SIGPROF, SIG_DFL);
                            signal(SIGXCPU, SIG_DFL);
                            signal(SIGXFSZ, SIG_DFL);

                            char *newargv[]= {"/bin/sh","-c","cd /tmp;sleep 1; killall smd dropbear uhttpd lighttpd; sleep 1; sysupgrade firmware.img", 0};
                            char *newenviron[]={NULL};
                            char * myargv[] = {"ls", "-al", "/etc/passwd", 0};
                            execvp("/bin/sh", newargv);
                            printf("I am child , I'm done, errno = %d\n", errno);
                        }
                        waitpid(-1, NULL, WNOHANG);
                        printf("I am father , I'm done\n");
                        return 0;
                        #endif
                        
                        break;

                    default:
                        break;
                }
            }
        }
    }

    vosMsg_cleanup(&g_mcHandle);
    #endif
}

