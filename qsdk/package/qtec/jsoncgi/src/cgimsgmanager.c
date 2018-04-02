#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fwk.h>
#include <signal.h>
#include "basic.h"
#include "keyapi.h"
#include "encryption.h"
#include <sqlite3.h>
#include <sec_api.h>
#include "network_set.h"

#define CGIMSGMANGENT_LOGFILE "/tmp/.cgimsgmanager.txt"
#define STDIN		0	
#define STDOUT		1	
#define STDERR		2	
#include "sys/resource.h"

#define _PATH_DEVNULL "/dev/null"

int wget_flag=0; 

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};

void *g_msgHandle;

void vpn_remove_edit_flag()
{
    system("rm -f /tmp/vpn_edit_flag");
}


static void ProcWifiSet()
{
	printf("enter wifi up message.\n");
	system("wifi up");
    system("ifup wan");
}

static void ProcFirstbootSet()
{
    sleep(2);
	QtRestore();
}

static void ProcUpdate(int isKeepConfig)
{
    FILE *fp = NULL;
    char buffer[256] = {0};
    char downloadUrl[256] = {0};
    char cmd[512] = {0};

    UTIL_DO_SYSTEM_ACTION("rm /tmp/downloadrate");
    UTIL_DO_SYSTEM_ACTION("rm %s", QT_SW_UPGRADE_RES_FILE);
    UTIL_DO_SYSTEM_ACTION("rm %s", QT_IMG_CHK_RES_FILE);
    
    fp=fopen("/tmp/downloadUrl","r");
	if(fp)
	{
		while(NULL != fgets(buffer, 256, fp))
		{
			if(0 == strncmp(buffer, "downloadUrl", strlen("downloadUrl")))
			{
				sscanf(buffer, "downloadUrl:%s", downloadUrl);
				break;
			}			
		}
		fclose(fp);
		fp = NULL;
	}
    
    sprintf(cmd, "wget -O /tmp/firmware.img %s 2>/tmp/downloadrate", downloadUrl);
	//printf("cmd:%s.\n", cmd);    
    system(cmd);
	fp=fopen("/tmp/downloadrate","r");
    if(fp)
    {
        while(NULL != fgets(buffer, 256, fp))
    	{
            printf("buffer data:%s.\n", buffer);
            memset(buffer, 0, 256);
    	}
    	fclose(fp);
    }
	if (VOS_RET_SUCCESS == QtUpgradeSoftware(isKeepConfig))
    {   
    #if 0
        VosMsgHeader stMsg = {0};
        stMsg.dataLength = 0;
        stMsg.dst = EID_IMG_UPGRADE;
        stMsg.src = EID_CGIMSGPROC;
        stMsg.type = VOS_MSG_UPGRADE_IMG;
    	stMsg.flags_request = 1;
        stMsg.wordData = isKeepConfig;

    	vosMsg_send(g_msgHandle, &stMsg);
        #endif
    }
}

static void ProcRawKeyAdd()
{
    int usedcount = 0;
    int unusedcount = 0;
	char userid[32] = {0};
	char deviceid[32] = {0};
    struct tagCQtQkMangent *pstcqtqkmangent;

    pstcqtqkmangent = GetCQtQkMangent();
	//if rawkeynumber <50, addkey
	C_GetCount(pstcqtqkmangent, 1, &usedcount, &unusedcount, userid, deviceid);
	printf("unusedcount is %d.\n", unusedcount);
	if(unusedcount <= MIXRAWKEYNUM)
	{
		AddRawKey();
	}
	ReleaseCQtQkMangent(&pstcqtqkmangent);
}

#define WAN_SPEED_TEST_FILE "/tmp/wan_speed.txt"
static void thread_ifstat()
{
    DEBUG_PRINTF("[%s]======\n",__func__);
    char wan_ifname[64]={0};
    rtcfgUciGet("network.wan.ifname", wan_ifname);
    char cmd[256]={0};
    snprintf(cmd,256,"ifstat -i %s -n > "WAN_SPEED_TEST_FILE,wan_ifname);
    system(cmd);

    DEBUG_PRINTF("thread_ifstat exit===\n");
    pthread_detach(pthread_self());
}

void handler()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    pthread_exit(0);
}

static void thread_wget()
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    signal(SIGKILL,handler);
    while(wget_flag)
    {
        //产生数据包
        //system("rm -rf test.img");
        system("wget http://qtec-route-headimg.oss-cn-shanghai.aliyuncs.com/test/test.img -O /dev/null");
    }

    DEBUG_PRINTF("[%s]===thread_wget exit=====\n",__func__);
     pthread_detach(pthread_self());
}
static void ProcSpeedTest()
{
    DEBUG_PRINTF("====[%s]======\n",__func__);
    rtcfgUciSet("system.@system[0].speedtest=0");  //0:means testing, 1 means tested
    
    //先确认qos， 若qos开启，则先关闭qos
    int qos_enabled=0;
    char tmp_qos_enabled[6]={0};
    rtcfgUciGet("qos.wan.enabled", tmp_qos_enabled);
    qos_enabled=atoi(tmp_qos_enabled);

    if(qos_enabled == 1)
    {
        rtcfgUciSet("qos.wan.enabled=0");
        system("/etc/init.d/qos restart");
    }

    pthread_t id_1;
    pthread_t id_2;
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
    
    //创建子进程来实时记录网速，

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
out:
    wget_flag=0;

    system("killall wget");
    system("killall ifstat");
    
    //分析数据
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
    
    //如果原先qos开启，则重新打开qos
    if(qos_enabled == 1)
    {
        rtcfgUciSet("qos.wan.enabled=1");
        system("/etc/init.d/qos restart");
    }

    rtcfgUciSet("system.@system[0].speedtest=1");
    memset(cmd,0,256);
    snprintf(cmd,256,"system.@system[0].output_upload=%f",output_upload);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"system.@system[0].output_download=%f",output_download);
    rtcfgUciSet(cmd);
    return;
    
    
}

void restartGuestWifi()
{
    system("/etc/init.d/network reload");
    system("/etc/init.d/dnsmasq restart");
    system("/etc/init.d/firewall reload");
    system("/etc/init.d/qtec_ebtables restart");
    sleep(10);
    system("ifup wan");
    pthread_detach(pthread_self());
}

void ProcGuestWifiSet()
{
    printf("enter guest wifi set");
    pthread_t id_1;
    int ret;

    ret=pthread_create(&id_1,NULL,(void  *) restartGuestWifi,NULL); 
    if(ret!=0)  
    {  
        printf("Create pthread error!\n");    
    } 
    return;
}

void check_wisp_status()
{
    int i = 0;
    FILE *fp;
    QT_WDS_WIFI_NET_INFO wdsInfo;
    char lanIp[32] = {0};
    char lanNetmask[32] = {0};
    char suggestLanIp[32] = {0};
    unsigned long ip, netmask, wdsIp, wdsNetmask, lanSugIp, lanSugNetmask;
    struct lanConfig result={0};
    QT_GUEST_WIFI_CFG guestWifiCfg = {0};
    int ret = 0;
    
    sleep(20);
    while (i < 70)
    {
        char status[16] = {0};
        
        system("ubus call network.interface.wwan status | grep '\"up\"' | sed -e 's/^.*: \\(.*\\),/\\1/g' > /tmp/wdsup1");
        fp = fopen("/tmp/wdsup1","r");
        if (fp != NULL)
        {
            fgets(status, sizeof(status), fp);
            if (!strncmp(status, "true", 4))
            {
                fclose(fp);
                QtWdsGetStatus(&wdsInfo);
                rtcfgUciGet("network.lan.ipaddr",lanIp);
                rtcfgUciGet("network.lan.netmask",lanNetmask);
                ip = inet_addr(lanIp);
                netmask = inet_addr(lanNetmask);
                ip &= netmask;
                wdsIp = inet_addr(wdsInfo.ipaddr);
                wdsNetmask = inet_addr(wdsInfo.netmask);
                wdsIp &= wdsNetmask;

                if (wdsIp == ip)
                {
                    sleep(3);
                    lanSugIp = inet_addr(WDS_LAN_IP);
                    lanSugNetmask = inet_addr(WDS_LAN_NETMASK);
                    lanSugIp &= lanSugNetmask;

                    if (wdsIp != lanSugIp)
                    {
                        UTIL_STRNCPY(suggestLanIp, WDS_LAN_IP, sizeof(suggestLanIp));
                    }
                    else
                    {
                        UTIL_STRNCPY(suggestLanIp, WDS_LAN_IP_2, sizeof(suggestLanIp));
                    }
                    
                    ret = lanConfigGet(&result);
                    memset(result.ipaddress, 0, sizeof(result.ipaddress));
                    UTIL_STRNCPY(result.ipaddress, suggestLanIp, sizeof(result.ipaddress));
                    lanConfigSet(&result);
                    ret = QtGetGuestWifi(&guestWifiCfg);
                    ret = QtSetGuestWifi(&guestWifiCfg);
                    system("/etc/init.d/network reload");
                    system("/etc/init.d/dnsmasq restart");
                    system("/etc/init.d/firewall restart");
                }
                return;
            }

            fclose(fp);
        }
        i++;
        sleep(1);
    }
    system("uci set wireless.wds.mode=tmp");
    system("uci del wireless.wds.device");
    system("uci commit wireless");
    system("uci set network.guest.ifname=ath01");
    system("uci set network.guest._orig_ifname=ath01");
    system("uci set network.wan=interface");
    system("uci commit network");
    system("/etc/init.d/network reload");
    sleep(10);
    system("ifup wwan");
    pthread_detach(pthread_self());
}

void ProcWdsSet()
{
    printf("enter wds set");
    pthread_t id_1;
    int ret;

    ret=pthread_create(&id_1,NULL,(void  *) check_wisp_status,NULL);  
    system("echo '--------------------\n' > /tmp/wdsdebug");
    if(ret!=0)  
    {  
        printf("Create pthread error!\n");  
        return ;  
    }  
    sleep(1);
    system("/etc/init.d/network reload");
    system("/etc/init.d/firewall reload");
}


void ProcAntiWifiInit()
{
    char enable[16]={0};
    char routerAccess[16] = {0};
    char lanDevAccess[16] = {0};
    char cmd[256] = {0};
    char cmd1[256] = {0};
    char cmd2[256] = {0};
	char cmd3[256] = {0};
    int i, ret;
    char macaddr[64] = {0};
	char lanip[64] = {0};
    char netmask[64] = {0};
	FILE *fp = NULL;

    fp = fopen("/etc/config/antiwifi", "r");
	if(NULL == fp)
	{
		fp = fopen("/etc/config/antiwifi", "a+");
		if (fp != NULL)
		{
			rtcfgUciAdd("antiwifi","systeminfo");
			rtcfgUciCommit("antiwifi");
		    fclose(fp);
		}
		else
		{
			printf("open file antiwifi error!.\n");
			return;
		}
	}
	else
	{
		fclose(fp);
	}

	system("ebtables -N devaccess");
	system("ebtables -D FORWARD -j devaccess");
	system("ebtables -A FORWARD -j devaccess");
    system("ebtables -F devaccess");
    rtcfgUciGet("antiwifi.@systeminfo[0].enable", enable);
    rtcfgUciGet("antiwifi.@systeminfo[0].router_access", routerAccess);
    rtcfgUciGet("antiwifi.@systeminfo[0].lan_dev_access", lanDevAccess);

	rtcfgUciGet("network.lan.ipaddr",lanip);
    rtcfgUciGet("network.lan.netmask",netmask);
    if(atoi(enable))
    {
        /* ebtables set */
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -t nat -D PREROUTING -i ath0 -j mark --mark-set 0x50");
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd1, 0, 256);
        snprintf(cmd1, 256, "ebtables -t nat -D PREROUTING -i ath1 -j mark --mark-set 0x50");
		printf("cmd1:%s.\n", cmd1);
		system(cmd1);
        
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -t nat -A PREROUTING -i ath0 -j mark --mark-set 0x50");
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd1, 0, 256);
        snprintf(cmd1, 256, "ebtables -t nat -A PREROUTING -i ath1 -j mark --mark-set 0x50");
		printf("cmd1:%s.\n", cmd1);
		system(cmd1);
        
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "iptables -t nat -D prerouting_lan_rule -p tcp -m multiport --dport 80,8080,443 ! -d %s -m mark --mark 0x50 -j DNAT --to %s:81", lanip, lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);
        
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "iptables -t nat -A prerouting_lan_rule -p tcp -m multiport --dport 80,8080,443 ! -d %s -m mark --mark 0x50 -j DNAT --to %s:81", lanip, lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

        if(atoi(routerAccess))
        {
            memset(cmd, 0, 256);
    		snprintf(cmd, 256, "iptables -t mangle -D PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
    		printf("cmd:%s.\n", cmd);
    		system(cmd);
            
    		memset(cmd, 0, 256);
    		snprintf(cmd, 256, "iptables -t mangle -A PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
    		printf("cmd:%s.\n", cmd);
    		system(cmd);
        }

        if(atoi(lanDevAccess))
    	{
            memset(cmd, 0, 256);
            snprintf(cmd, 256, "ebtables -D devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
            system(cmd);
            memset(cmd, 0, 256);
            snprintf(cmd, 256, "ebtables -D devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
            system(cmd);
            memset(cmd, 0, 256);
    		snprintf(cmd, 256, "ebtables -D devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
    		printf("cmd:%s.\n", cmd);
    		system(cmd);

            memset(cmd, 0, 256);
            snprintf(cmd, 256, "ebtables -A devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
            system(cmd);
            memset(cmd, 0, 256);
            snprintf(cmd, 256, "ebtables -A devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
            system(cmd);
    		memset(cmd, 0, 256);
    		snprintf(cmd, 256, "ebtables -I devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
    		printf("cmd:%s.\n", cmd);
    		system(cmd);

    	}        
    }

    /* whitelist init*/
    i = -1;
	ret = 0;
    while(ret == 0)
    {
        i++;
        memset(cmd,0,256);
        snprintf(cmd,256,"antiwifi.@whitelist[%d].macaddr",i);
        memset(macaddr,0,64);
        ret = rtcfgUciGet(cmd,macaddr);
        if(0 == ret)
        {
            snprintf(cmd, 256, "ebtables -D devaccess -s %s -j RETURN", macaddr);
            snprintf(cmd1, 256, "ebtables -D devaccess -d %s -j RETURN", macaddr);
        	snprintf(cmd2, 256, "iptables -t nat -D prerouting_lan_rule -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macaddr);
			snprintf(cmd3, 256, "iptables -t mangle -D PREROUTING -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macaddr);
            system(cmd);
            system(cmd1);
        	system(cmd2);
			system(cmd3);
            memset(cmd, 0, 256);
            memset(cmd1, 0, 256);
            memset(cmd2, 0, 256);
			memset(cmd3, 0, 256);
            snprintf(cmd, 256, "ebtables -I devaccess -s %s -j RETURN", macaddr);
            snprintf(cmd1, 256, "ebtables -I devaccess -d %s -j RETURN", macaddr);
        	snprintf(cmd2, 256, "iptables -t nat -I prerouting_lan_rule -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macaddr);
			snprintf(cmd3, 256, "iptables -t mangle -I PREROUTING -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macaddr);
            system(cmd);
            system(cmd1);
        	system(cmd2);
			system(cmd3);
        }
    }
    
}


void ProcOneKeySwitch()
{
    FILE *fp;
    char aucBuf[64] = {0};
    int n=0;
    struct wanPppoeConfig pppCfg = {0};
    

    remove(ONE_KEY_SWITCH_USR_PWD_FILE);

    UTIL_DO_SYSTEM_ACTION("pppoe-server -I eth0 -L 192.168.1.1 -R 192.168.1.2 -N 10");
    UTIL_DO_SYSTEM_ACTION("echo 1 > %s", ONE_KEY_SWITCH_RESULT_FILE);
    while(1){
        if (access(ONE_KEY_SWITCH_USR_PWD_FILE,F_OK) == 0){
            fp = fopen(ONE_KEY_SWITCH_USR_PWD_FILE, "r");
            if (fp != NULL){
                fread(aucBuf, 64, 1, fp);
                printf("-----------%s----------\n",aucBuf);
                if (sscanf(aucBuf, "%s %s", pppCfg.username, pppCfg.password) != 2)
                {
                    printf("Get username and password error!\n");
                    UTIL_DO_SYSTEM_ACTION("echo 4 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                    fclose(fp);
                    break;
                }
                printf("Set pppoe username<%s>, password<%s>!\n", pppCfg.username, pppCfg.password);
                UTIL_DO_SYSTEM_ACTION("echo 2 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                wanPppoeConfigSet(&pppCfg);
                UTIL_DO_SYSTEM_ACTION("echo 3 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                UTIL_DO_SYSTEM_ACTION("killall pppoe-server");
                UTIL_DO_SYSTEM_ACTION("echo 6 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                fclose(fp);
                break;
            }

            
        }

        if (n >= 30){
            UTIL_DO_SYSTEM_ACTION("killall pppoe-server");
            UTIL_DO_SYSTEM_ACTION("echo 5 > %s", ONE_KEY_SWITCH_RESULT_FILE);
            break;
        }
        n++;
        sleep(1);
    }
}

void ProcFirewallSet()
{
    sleep(1);
    system("/etc/init.d/dnsmasq restart");
    system("/etc/init.d/firewall restart");
}

void ProcLanCfgSet()
{
    sleep(1);
    system("ubus call network.interface.lan prepare");
    system("ifup lan");
    system("/etc/init.d/dnsmasq restart");
    system("/etc/init.d/firewall restart");
    system("wifi up");
}

void restart_vpn_network()
{
#if 0
    char cmd[128] = {0};
    
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "ubus call network.interface.%s status > /tmp/vpnstatus", vpnCfgArray[j].ifname);
    system(cmd);
    system("grep '\"up\"' /tmp/vpnstatus | sed -e 's/^.*: \\(.*\\),/\\1/g' > /tmp/vpnlinkstatus");
    fp = fopen("/tmp/vpnlinkstatus", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%s", linkstatus);
        if (!strncmp(linkstatus, "true", 4))
        {
            vpnCfgArray[j].status = 1;
        }
        else
        {
            vpnCfgArray[j].status = 0;
        }
        fclose(fp);
    }
    else
    {
        vpnCfgArray[j].status = 0;
    }
    #endif
    sleep(8);
    system("ifup -b -w");    
    sleep(2);
    vpn_remove_edit_flag();
    pthread_detach(pthread_self());
}

void ProcVpnCfgSet()
{
    char cmd[128] = {0};
    pthread_t id_1;
    int ret;

    ret=pthread_create(&id_1,NULL,(void  *) restart_vpn_network,NULL);  
    if(ret!=0)  
    {  
        printf("Create pthread error!\n");  
        return ;  
    }  
    system("/etc/init.d/firewall reload");
}
void checkWdsDown()
{
    int i = 0;
    FILE *fp;
    sleep(20);
    char mode[16] = {0};
    rtcfgUciGet("wireless.wds.mode", mode);
    if (util_strlen(mode) == 0 || util_strncmp(mode, "sta", util_strlen("sta")))
    {
        DEBUG_PRINTF("[%s][%d]:wds cfg has been tear down, no need to check\n",__func__,__LINE__);
        return;
    }
    while (i < 90)
    {
        char status[16] = {0};
        
        system("ubus call network.interface.wwan status | grep '\"up\"' | sed -e 's/^.*: \\(.*\\),/\\1/g' > /tmp/wdsup1");
        fp = fopen("/tmp/wdsup1","r");
        if (fp != NULL)
        {
            fgets(status, sizeof(status), fp);
            if (!strncmp(status, "true", 4))
            {
                fclose(fp);
                return;
            }

            fclose(fp);
        }
        i++;
        sleep(1);
    }
    system("uci set wireless.wds.mode=tmp");
    system("uci del wireless.wds.device");
    system("uci commit wireless");
    system("uci set network.guest.ifname=ath01");
    system("uci set network.guest._orig_ifname=ath01");
    system("uci set network.wan=interface");
    system("uci commit network");
    system("/etc/init.d/network reload");
    pthread_detach(pthread_self());
}
void ProcWdsDown()
{
    char cmd[128] = {0};
    pthread_t id_1;
    int ret;

    ret=pthread_create(&id_1,NULL,(void  *) checkWdsDown,NULL);  
}

/**
 *  function: main
 *           the entrance of cgi message handle
 */
void main()
{
    //step1: daemonize itself to avoid interrupted by console 
    //daemonize();
	
	int ret = 0;
	int n = -1;
	int commFd = -1;
	int maxFd = -1;
	int fd, rv;
	fd_set readFdsMaster,rfds;
	VosMsgHeader *msg = NULL;
	struct VosMsgBody *body = NULL;
    int nRet = 0;
	int i = 0, j = 0;
    int nRow = 0; 
    int nColumn = 0;
	int randomReadyFlag = 0;
    char randBuf[16] = {0};
	char mainKey[17] = {0};
    char szSql[4096] = {0};
    char** pResult;
	char* cErrMsg;
    sqlite3 *dbHangle;
	FILE *fp = 0;
    struct timeval tv1, tv2;
    char cmd[128] = {0};
	ret=vosMsg_init(EID_CGIMSGPROC, &g_msgHandle);
    gettimeofday(&tv1, NULL);
    snprintf(cmd, sizeof(cmd), "echo %us %uus > /tmp/spiint_time_start",tv1.tv_sec, tv1.tv_usec);
    system(cmd);
	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return;
	}

	if(access(CGIMSGMANGENT_LOGFILE,F_OK) ==0)
	{
		freopen(CGIMSGMANGENT_LOGFILE,"w",stdout);
        freopen(CGIMSGMANGENT_LOGFILE,"w",stderr);
	}

    /* add qtkeymanger function, check database creation and ramdom data is ready */
    nRet = sqlite3_open("/etc/testDB.db", &dbHangle);
 	if(nRet){
		printf("CQtQkMangentCommon::QkPoolOpen, sqlite3_open error\n");
 	}

	while(0 != sqlite3_get_table(dbHangle, "select * from rawkey;", &pResult, 0, 0, &cErrMsg))
	{
        memset(szSql, 0, 4096);
		sprintf(szSql,"create table rawkey(key_id text primary key,key_state int,key blob, validity_time int,create_time text,modify_time text,peeridList text);");
		nRet = sqlite3_exec(dbHangle, szSql, NULL, 0, &cErrMsg);
		if (0 != nRet){
	        printf("create table rawkey error\n");
	    }
        sqlite3_free_table(pResult);
        sleep(1);
	}
	sqlite3_free_table(pResult);
	
	while(0 != sqlite3_get_table(dbHangle, "select * from synkey;", &pResult, 0, 0, &cErrMsg))
	{
        memset(szSql, 0, 4096);
		sprintf(szSql,"create table synkey(key_id text primary key,key_state int,key blob,user_id text,device_id text,validity_time int,create_time text,modify_time text,peeridList text);");
		nRet = sqlite3_exec(dbHangle, szSql, NULL, 0, &cErrMsg);
		if (0 != nRet){
	        printf("create table synkey error\n");
	    }
        sqlite3_free_table(pResult);
        sleep(1);
	}
	sqlite3_free_table(pResult);

    while(0 != sqlite3_get_table(dbHangle, "select * from startkey;", &pResult, &nRow, &nColumn, &cErrMsg))
	{
        memset(szSql, 0, 4096);
		sprintf(szSql,"create table startkey(keyid text primary key);");
		nRet = sqlite3_exec(dbHangle, szSql, NULL, 0, &cErrMsg);
		if (0 != nRet){
	        printf("create table startkey error\n");
	    }
        sqlite3_free_table(pResult);
        sleep(1);
	}
    sqlite3_free_table(pResult);
    
    while(nRow == 0)
    {
        nRet = sqlite3_exec(dbHangle, "insert into startkey values ('10000000');", NULL, 0, &cErrMsg);
        if (0 != nRet){
	        printf("create table startkey error\n");
	    }
        nRet = sqlite3_get_table(dbHangle, "select * from startkey;", &pResult, &nRow, &nColumn, &cErrMsg);
        sqlite3_free_table(pResult);
    }
	sqlite3_close(dbHangle);
    while (j < 10)
    {
        if (QtGetSpiLock(g_msgHandle) == 0)
        {      
        	while(0 == randomReadyFlag)
        	{
        		memset(randBuf, 0, 16);
            	GetRandom(randBuf, 16);
        		for(i = 0; i < 16; i++)
        		{
        			if(randBuf[i] != 0)
        			{
        				randomReadyFlag = 1;
        				printf("random data is ok!\n");
        				break;
        			}
        		}
        		sleep(1);
        	}

        	//保密芯片主key存到本地内存
            ret = LoadData(mainKey, 16);
            if(0 == ret)
            {
            	fp = fopen("/etc/info/mainkey", "w+");	
            	if(fp)
            	{
            		fprintf(fp, "%s", mainKey);
                    fclose(fp);
                    fp = NULL;
            	}
            	else
            	{
            		printf("open file failed again.\n");
            	}
            }
            QtReleaseSpiLock(g_msgHandle);
            break;
        }
        j++;
        sleep(1);
    }
	ProcRawKeyAdd();
    gettimeofday(&tv2, NULL);
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "echo %us %uus > /tmp/spiint_time_end",tv2.tv_sec, tv2.tv_usec);
    system(cmd);
	/* antiwifi init */
    ProcAntiWifiInit();

	//receive message
	vosMsg_getEventHandle(g_msgHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;
	while(1)
	{
		rfds = readFdsMaster;
		n = select(maxFd+1, &rfds, NULL, NULL, NULL);
		if (n < 0)
		{
			continue;
		}
		
		if (FD_ISSET(commFd, &rfds))
		{
			ret = vosMsg_receive(g_msgHandle, &msg);
			if (ret != VOS_RET_SUCCESS)
			{
				continue;
			}
			switch(msg->type)
			{
				case VOS_MSG_WIFI_REQ:
				{
					body = msg;
					ProcWifiSet();
					break;
				}
                case VOS_MSG_UPDATE_REQ:
				{
					DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
					ProcUpdate(0);
					break;
				}
                case VOS_MSG_UPDATE_KEEPCFG_REQ:
				{
					DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
					ProcUpdate(1);
					break;
				}
                case VOS_MSG_SPEEDTEST_REQ:
                {
                    //ProcSpeedTest();
                    system("qtec_speedtest&");
                    break;
                }
                case VOS_MSG_FIREWALL_RESTART_REQ:
                {
                    system("/etc/init.d/firewall restart &");
                    break;
                }
                case VOS_MSG_QOS_RESTART_REQ:
                {
                    system("/etc/init.d/qos restart &");
                    break;
                }
                case VOS_MSG_SET_GUEST_WIFI_REQ:
                {
                    ProcGuestWifiSet();
                    break;
                }
                case VOS_MSG_SET_WDS_REQ:
                {
                    ProcWdsSet();
                    break;
                }
                case VOS_MSG_RAWKEY_REQ:
                {
                    ProcRawKeyAdd();
                    break;
                }
                case VOS_MSG_ONE_KEY_SWITCH:
                {
                    ProcOneKeySwitch();
                    break;
                }
                case VOS_MSG_FIREWALL_SET:
                {
                    ProcFirewallSet();
                    break;
                }
                case VOS_MSG_LAN_CFG_SET:
                {
                    ProcLanCfgSet();
                    break;
                }
				case VOS_MSG_FIRSTBOOT_REQ:
                {
                    ProcFirstbootSet();
                    break;
                }
                case VOS_MSG_VPN_SET:
                {
                    ProcVpnCfgSet();
                    break;
                }
                case VOS_MSG_DISKREFORMAT_REQ:
                {   
                    system("qtec_disk_reformat.sh >/dev/console &");
                    break;
                }
                case VOS_MSG_DISKCHECK_REQ:
                {
                    system("qtec_disk.sh >/dev/console &");
                    break;
                }
                case VOS_MSG_WDS_DOWN:
                {
                    ProcWdsDown();
                    break;
                }
                
				default:
					break;
			}
			if (NULL!= msg)
			{				
				VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);				
			}
		}
	}
	vosMsg_cleanup(&g_msgHandle);
}

