#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "fwk.h"
#include "librtcfg.h"




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

int main()
{
    FILE *fp;
    char aucBuf[64] = {0};
    int n=0;
    struct wanPppoeConfig pppCfg = {0};
    
    daemonize();

    remove(ONE_KEY_SWITCH_USR_PWD_FILE);

    UTIL_DO_SYSTEM_ACTION("pppoe-server -I eth0 -L 192.168.1.1 -R 192.168.1.2 -N 10");

    while(1){
        if (access(ONE_KEY_SWITCH_USR_PWD_FILE,F_OK) == 0){
            fp = fopen(ONE_KEY_SWITCH_USR_PWD_FILE, "r");
            if (fp != NULL){
                fread(aucBuf, 64, 1, fp);
                printf("-----------%s----------\n",aucBuf);
                sscanf(aucBuf, "%s %s", pppCfg.username, pppCfg.password);
                printf("Set pppoe username<%s>, password<%s>!\n", pppCfg.username, pppCfg.password);
                wanPppoeConfigSet(&pppCfg);
                UTIL_DO_SYSTEM_ACTION("killall pppoe-server");
                break;
            }

            
        }

        if (n >= 30){
            UTIL_DO_SYSTEM_ACTION("killall pppoe-server");
            break;
        }
        n++;
        sleep(1);
    }

    return 0;
}

