#include "basic.h"

int wjj=2313;

/**
 *  func_name: daemonize()
 *          daemoin this process
 */
void daemonize()
{
    //void that process interrupted by sig before daemonize itself
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    
    //father process exit, and child process go on
    if( 0 != fork() )
            exit(0);

    if( -1 == setsid())
    {
        printf("=====ERROR!!!= setsid fail===\n");
        exit(0);
    }

    signal(SIGHUP, SIG_IGN);
    
    if(0!=fork())
        exit(0);

    if(0!=chdir("/"))
        exit(0);
}
