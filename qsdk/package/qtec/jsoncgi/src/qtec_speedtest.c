#include "stdio.h"
#include "unistd.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "signal.h"
#include "pthread.h"

#define DEBUG_PRINTF(format,...)   printf(format, ##__VA_ARGS__); fflush(stdout);

int qos_enabled=0;

pthread_t id_1=0;
pthread_t id_2=0;
int wget_flag=0; 


#define speedtest_logfile "/tmp/.speedtest"
#define WAN_SPEED_TEST_FILE "/tmp/wan_speed.txt"

#define _PATH_SPEEDTEST_PID "/etc/speedtest.pid"
#define ProgramName "SpeedTest"

#if 0
/**
 *  func_name: daemonize()
 *          daemoin this process
 */
static void daemonize()
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
#endif 



static void init_log()
{
	FILE *f1;
	if(access(speedtest_logfile,F_OK) !=0)
	{	
		return;
	}
	f1 = open(speedtest_logfile, O_RDWR | O_APPEND);

	if(f1!=NULL)
	{
		dup2(f1,1);
		dup2(f1,2);

		close(f1);
	}
	
}




int acquire_daemonlock(int closeflag) {
	static int speedtest_fd = -1;
	//char buf[3*MAX_FNAME];
	char buf[1024]={0};
	const char *pidfile;
	char *ep;
	long otherpid;
	//ssize_t num;
    int num;
    
	if (closeflag) {
		/* close stashed fd for child so we don't leak it. */
		if (speedtest_fd != -1) {
			close(speedtest_fd);
			speedtest_fd = -1;
		}
		return 0;
	}

	if (speedtest_fd == -1) {
		pidfile = _PATH_SPEEDTEST_PID;
		/* Initial mode is 0600 to prevent flock() race/DoS. */
		if ((speedtest_fd = open(pidfile, O_RDWR|O_CREAT, 0600)) == -1) {
			sprintf(buf, "can't open or create %s: %s",
				pidfile, strerror(errno));
			DEBUG_PRINTF("[%s]%s: %s\n",__func__,ProgramName, buf);

			return -1;
		}

		while (flock(speedtest_fd, LOCK_EX|LOCK_NB) < 0) {
			int save_errno = errno;

			bzero(buf, sizeof(buf));
			if ((num = read(speedtest_fd, buf, sizeof(buf) - 1)) > 0 &&
			    (otherpid = strtol(buf, &ep, 10)) > 0 &&
			    ep != buf && *ep == '\n') {
				sprintf(buf,
				    "can't lock %s, otherpid may be %ld: %s",
				    pidfile, otherpid, strerror(save_errno));
			} else {
				sprintf(buf,
				    "can't lock %s, otherpid unknown: %s",
				    pidfile, strerror(save_errno));
			}
		
			//fprintf(stderr, "%s: %s\n", ProgramName, buf);
			//log_it("CRON", getpid(), "DEATH", buf);
            DEBUG_PRINTF("[%s],%s : %s\n",__func__,ProgramName,buf);
            sleep(1);
            
		}
		(void) fchmod(speedtest_fd, 0644);
		(void) fcntl(speedtest_fd, F_SETFD, 1);
	}

	sprintf(buf, "%ld\n", (long)getpid());
    DEBUG_PRINTF("[%s]===write %s to %s====\n",__func__,buf,_PATH_SPEEDTEST_PID);
    
	(void) lseek(speedtest_fd, (off_t)0, SEEK_SET);
	num = write(speedtest_fd, buf, strlen(buf));
	(void) ftruncate(speedtest_fd, num);

    return 0;
	/* abandon fd even though the file is open. we need to keep
	 * it open and locked, but we don't need the handles elsewhere.
	 */
}

int cancel_speedtest(int value)
{
    int ret=0;
    char cmd[256]={0};
    float max_upload=0;
    float max_download=0;
    float upload;
    float download;
    FILE *fp;
    char tmpline[256]={0};
    float output_upload;
    float output_download;
    
    DEBUG_PRINTF("===[%s]======\n",__func__);
#if 0
    if(id_1 != 0)
    {
        pthread_cancel(id_1);
    }
#endif

    system("killall ifstat");

    if(id_2 != 0)
    {
        pthread_cancel(id_2);
    }
    system("killall wget");
    //Èç¹ûÔ­ÏÈqos¿ªÆô£¬ÔòÖØÐÂ´ò¿ªqos
    if(qos_enabled == 1)
    {
        rtcfgUciSet("qos.wan.enabled=1");
        system("/etc/init.d/qos restart");
    }
    
    if(value == 1)
    {
        fp=fopen(WAN_SPEED_TEST_FILE,"r");
        if(!fp)
        {
            DEBUG_PRINTF("[%s]===wan_speed_test_file cannot find===\n",__func__);
        
        }
        else
        {
            fgets(tmpline,256,fp);
            fgets(tmpline,256,fp);

            memset(tmpline,0,256);
            while( (fgets(tmpline,256,fp)) !=NULL )
            {
                sscanf(tmpline,"%f %f",&download,&upload);
                if(download > max_download)
                {
                    max_download = download;
                }

                if(upload > max_upload)
                {
                    max_upload=upload;
                }
            }
       
        }
        DEBUG_PRINTF("===[%s]=== max_upload:%f max_download:%f ===\n",__func__,max_upload,max_download);
        output_upload = (max_upload *8)/1024 ;
        output_download = (max_download *8)/1024;
        DEBUG_PRINTF("==[%s]===out: output_upload:%f   output_download:%f ====\n", __func__,output_upload, output_download);
    
        memset(cmd,0,256);
        snprintf(cmd,256,"system.@system[0].output_upload=%f",output_upload);
        rtcfgUciSet(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"system.@system[0].output_download=%f",output_download);
        rtcfgUciSet(cmd);
    
        rtcfgUciSet("system.@system[0].speedtest=1"); //speedtest end...
    }
    
    
    DEBUG_PRINTF("[%s]===speedtest %ld cancel===\n",__func__,(long)getpid());
    exit(0);
}

static void thread_ifstat()
{
    DEBUG_PRINTF("[%s]======\n",__func__);
    char wan_ifname[64]={0};
    char tmp[64]={0};
    char wwan_ifname[64]={0};
    char cmd[256]={0};
    int sta_enabled=0;
    FILE *sta_fp;

    //åˆ¤æ–­æ— çº¿ä¸­ç»§æ˜¯å¦å¼€å¯
    memset(cmd,0,256);
    snprintf(cmd,256,"ubus call network.interface.wwan status | grep '\"up\"' | sed -e 's/^.*: \\(.*\\),/\\1/g'");
    sta_fp=popen(cmd,"r");
    if(sta_fp)
    {
        memset(tmp,0,256);
        fgets(tmp,sizeof(tmp),sta_fp);
        DEBUG_PRINTF("[%s]===tmpstr:%s===\n",__func__,tmp);
        if(strstr(tmp,"true")!=NULL)
        {
            sta_enabled=1;
        }
        pclose(sta_fp);
    }
    DEBUG_PRINTF("[%s]==sta_enabled:%d===\n",__func__,sta_enabled);
    
    if(0==sta_enabled)
    {
        //ä¿®æ”¹é€»è¾‘ä¸ºé€‰æ‹©é»˜è®¤è·¯ç”±çš„å‡ºå£ç‚¹,è‹¥æ²¡æ‰¾åˆ°ï¼Œåˆ™é€‰æ‹©wan.ifname
        FILE *pp=popen("route -n | grep ^0.0.0.0 | awk '{print $8}'","r");
        if(!pp)
        {
            rtcfgUciGet("network.wan.ifname", wan_ifname);
        }
        if(pp)
        {
            fgets(tmp,sizeof(tmp),pp);
            strcpy(wan_ifname,tmp);
            DEBUG_PRINTF("====[%s]====wan_ifname:%s, %d",__func__,wan_ifname, strlen(wan_ifname));
            //ç§»é™¤\n
            wan_ifname[strlen(wan_ifname)-1]='\0';
            pclose(pp);
        }
        memset(cmd,0,256);
        snprintf(cmd,256,"ifstat -i %s -n > "WAN_SPEED_TEST_FILE,wan_ifname);
        system(cmd);

        DEBUG_PRINTF("thread_ifstat exit===\n");
    }
    else
    {
        rtcfgUciGet("wireless.wds.device", wwan_ifname);
        DEBUG_PRINTF("[%s]===wwan_ifname:%s===\n",__func__,wwan_ifname);

        if((strlen(wwan_ifname))==0)
        {
            DEBUG_PRINTF("[%s]===can not get wwan ifname===\n",__func__);
            return ;
        }
        memset(cmd,0,256);
        snprintf(cmd,256,"ifstat -i %s -n >"WAN_SPEED_TEST_FILE,wwan_ifname);
        system(cmd);
        DEBUG_PRINTF("thread_ifstat exit====\n");
    }
}


static void thread_wget()
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    char speedtest_url[256]={0};
    char cmd[256]={0};
    rtcfgUciGet("system.@system[0].speedtesturl",speedtest_url);

    while(wget_flag)
    {
        //²úÉúÊý¾Ý°ü
        //system("rm -rf test.img");
        if(strlen(speedtest_url)!=0)
        {
            snprintf(cmd,256,"wget %s -O /dev/null",speedtest_url);
            system(cmd);
        }
        else
        {
            //default url
            system("wget http://qtec-route-headimg.oss-cn-shanghai.aliyuncs.com/test/test.img -O /dev/null");
        }
    }

    DEBUG_PRINTF("[%s]===thread_wget exit=====\n",__func__);
    
}


void handler(int sign_no)
{
    printf("====speed test === [%s]===sign_no:%d==\n",__func__,sign_no);
    if(sign_no==SIGUSR1)
    {
        DEBUG_PRINTF("[%s]===recive sigusr1 to cancel speedtest==\n",__func__);
        cancel_speedtest(1);
        exit(0);
    }
    else if(sign_no==SIGUSR2)
    {
        DEBUG_PRINTF("[%s]====recive sigusr2 to restart speedtest===\n",__func__);
        cancel_speedtest(0);
        exit(0);
    }
}




void main()
{
    //daemonize();
    init_log();
    acquire_daemonlock(0);

    //sigusr1 to cancel speedtest
    signal(SIGUSR1,handler);
    signal(SIGUSR2,handler);
    
#if 0
    while(1)
    {
        
    }
#endif 

    //ÏÈÈ·ÈÏqos£¬ Èôqos¿ªÆô£¬ÔòÏÈ¹Ø±Õqos

    char tmp_qos_enabled[6]={0};
    rtcfgUciGet("qos.wan.enabled", tmp_qos_enabled);
    qos_enabled=atoi(tmp_qos_enabled);

    if(qos_enabled == 1)
    {
        rtcfgUciSet("qos.wan.enabled=0");
        system("/etc/init.d/qos restart");
        rtcfgUciSet("qos.wan.enabled=1");
    }
 

    int ret=0;
    char cmd[256]={0};
    float max_upload=0;
    float max_download=0;
    float upload;
    float download;
    FILE *fp;
    char tmpline[256]={0};
    float output_upload;
    float output_download;


    rtcfgUciSet("system.@system[0].speedtest=0"); //begin speedtest...
    
    //´´½¨×Ó½ø³ÌÀ´ÊµÊ±¼ÇÂ¼ÍøËÙ£¬

    ret=pthread_create(&id_1,NULL,(void  *) thread_ifstat,NULL);  
    if(ret!=0)  
    {  
        printf("Create pthread error!\n");  
        return ;  
    }  
    wget_flag=1;
    ret=pthread_create(&id_2,NULL,(void  *) thread_wget,NULL);  
    if(ret!=0)  
    {  
        printf("Create pthread error!\n");  
        return ;  
    }  
    
   
    int i=0;
    while(i<30)
    {
        DEBUG_PRINTF("[%s]===speedtest %ld is running ===\n",__func__,(long)getpid());
        sleep(1);
        i++;
    }
    
    #if 0
    int count =0;
    int test_flag=0;
    char tmp_test_flag[6]={0};
    while(count<30)
    {
        sleep(1);
        rtcfgUciGet("system.@system[0].speedtest", tmp_test_flag);
        test_flag=atoi(tmp_test_flag);
        if(test_flag==2)
        {
            goto out;
        }
        count++;
    }
    #endif 
out:
    wget_flag=0;
    //pthread_cancel(id_1);
    system("killall ifstat");
    pthread_cancel(id_2);
    system("killall wget");
    //system("killall ifstat");
    
    //·ÖÎöÊý¾Ý
    fp=fopen(WAN_SPEED_TEST_FILE,"r");
    if(!fp)
    {
        DEBUG_PRINTF("[%s]===wan_speed_test_file cannot find===\n",__func__);
        return ;
    }
    else
    {
        fgets(tmpline,256,fp);
        fgets(tmpline,256,fp);

        memset(tmpline,0,256);
        while( (fgets(tmpline,256,fp)) !=NULL )
        {
            sscanf(tmpline,"%f %f",&download,&upload);
            if(download > max_download)
            {
                max_download = download;
            }

            if(upload > max_upload)
            {
                max_upload=upload;
            }
        }
       
    }
    DEBUG_PRINTF("===[%s]=== max_upload:%f max_download:%f ===\n",__func__,max_upload,max_download);
    output_upload = (max_upload *8)/1024 ;
    output_download = (max_download *8)/1024;
    DEBUG_PRINTF("==[%s]===out: output_upload:%f   output_download:%f ====\n", __func__,output_upload, output_download);
    
    memset(cmd,0,256);
    snprintf(cmd,256,"system.@system[0].output_upload=%f",output_upload);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"system.@system[0].output_download=%f",output_download);
    rtcfgUciSet(cmd);
    
    rtcfgUciSet("system.@system[0].speedtest=1"); //speedtest end...
    
    //Èç¹ûÔ­ÏÈqos¿ªÆô£¬ÔòÖØÐÂ´ò¿ªqos
    if(qos_enabled == 1)
    {
        rtcfgUciSet("qos.wan.enabled=1");
        system("/etc/init.d/qos restart");
    }

    exit(0);
    
    
}


