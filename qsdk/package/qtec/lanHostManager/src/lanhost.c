#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lanhost.h"
#include "signal.h"
#include "pthread.h"
#include "cJSON.h"
#include "string.h"
#include <errno.h>
#include <librtcfg.h>
#include <libubus.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/blobmsg_json.h>
#include <json-c/json.h>
#include <fwk.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <qtec_firewall_basic.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define ARP_FILE "/proc/net/arp"
#define LANHOST_MAXLINESIZE 256
#define LAN_IFNAME "br-lan"
#define DHCP_FILE "/tmp/dhcp.leases"

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};

void *g_msgHandle;

pthread_mutex_t mut;
pthread_t workthread;

//variable about ubus 
struct ubus_context *ctx;
struct blob_buf b;
static int g_htx = 0;
static int g_hrx = 0;
static int g_routertx = 0;
static int g_routerrx = 0;
static int g_arpboundflag = 1;


static int check_mac_match(char *mac1, char * mac2)
{
    
    int mac1_1=0;
    int mac1_2=0;
    int mac1_3=0;
    int mac1_4=0;
    int mac1_5=0;
    int mac1_6=0;
    int mac2_1=0;
    int mac2_2=0;
    int mac2_3=0;
    int mac2_4=0;
    int mac2_5=0;
    int mac2_6=0;
    sscanf(mac1,"%02x:%02x:%02x:%02x:%02x:%02x",&mac1_1,&mac1_2,&mac1_3,&mac1_4,&mac1_5,&mac1_6);
    sscanf(mac2,"%02x:%02x:%02x:%02x:%02x:%02x",&mac2_1,&mac2_2,&mac2_3,&mac2_4,&mac2_5,&mac2_6);

    if( (mac1_1 == mac2_1) && (mac1_2==mac2_2) && (mac1_3==mac2_3) && (mac1_4==mac2_4) && (mac1_5==mac2_5) && (mac1_6==mac2_6) )
    {
        //DEBUG_PRINTF("[%s]====mac1 %s is the same as mac2:%s ====\n",__func__,mac1,mac2);
        return 0;
    }
    else 
    {
        //DEBUG_PRINTF("[%s]===mac1 %s is not the same as mac2:%s====\n",__func__,mac1,mac2);
        return -1;
    }
    
}

/**
 * func_name: getArpEntryTableNum()
 *            get the num of arp entry table from /proc/net/arp
 */
int getArpEntryTableNum(int *output)
{
    FILE *fp=NULL;
    if( (fp=fopen(ARP_FILE,"r"))==NULL)
    {
        printf("===ERROR!!!==%s===can't open FILE: "ARP_FILE"====\n",__func__);
        return -1;
    }

    char str[LANHOST_MAXLINESIZE]={0};
    int num=0;
    
    while((fgets(str,LANHOST_MAXLINESIZE,fp)) !=NULL )
    {
        num++;
    }

    *output= num -1;
    
    fclose(fp);

    return 0;
}

#if 0
/**
 * func_name: prase the arp string
 *          for example:192.168.1.100       0x1       0x2    00:0e:c6:d2:32:74   *   br-lan
 *
 */
int praseArpString(char *inputString, char *output1, char *output2, char *output3, char *output4, char *output5)
{
    char *point=inputString;
    int i=0;
    
    while(inputString[i] !=' ')
    {
        i++;
    }
    DEBUG_PRINTF("===i: %d ===\n", i);
    inputString[i]='\0';
    strcpy(output1,point);
    DEBUG_PRINTF("===output1: %s===\n",output1); 
    i++; 
    while(' ' == inputString[i])
    {
        i++;
    }
    DEBUG_PRINTF("===i: %d====\n",i);
    point= &(inputString[i]);

    while(inputString[i] != ' ')
    {
        i++;
    }

    DEBUG_PRINTF("===i: %d====\n",i);
    inputString[i]='\0';
    strcpy(output2,point);
    DEBUG_PRINTF("===output2: %s===\n",output2);
    my_mark();
    i++;

    while(' ' == inputString[i])
    {
        i++;
    }

    DEBUG_PRINTF("===i: %d====\n",i);
    point = &(inputString[i]);

    while(inputString[i] != ' ')
    {
        i++;
    }

    DEBUG_PRINTF("===i: %d====\n",i);
    inputString[i]='\0';
    strcpy(output3,point);
    DEBUG_PRINTF("===output3: %s===\n",output3);
    i++;

    while(' ' == inputString[i])
    {
        i++;
    }
    point = &(inputString[i]);

    while(inputString[i] != ' ')
    {
        i++;
    }
    inputString[i]='\0';
    strcpy(output4,point);
    DEBUG_PRINTF("===output4: %s====\n",output4);
    i++;

    while((' ' == inputString[i]) ||('*')==inputString[i])
    {
        i++;
    }
    point=&(inputString[i]);
    strcpy(output5,point);
    DEBUG_PRINTF("===output5: %s=====\n",output5);

    return 0;
}
#endif 

/**
 * func_name: getArpEntryTable
 *              get arp entry table from /proc/net/arp
 *
 *  input:  a. outputArray  === one pointer which point to one memory has been malloced, the memory size is 
 *  (*arraynum) * (struct arpEntry)
 *          b. *arraynum  === the num of outputarray entry 
 */
int getArpEntryTable(struct arpEntry *outputArray, int *arraynum)
{
    FILE *fp=NULL;
    if( (fp=fopen(ARP_FILE,"r"))==NULL )
    {
        printf("==ERROR!!!==%s === can't open FILE: "ARP_FILE"====\n",__func__);
        return -1;
    }
    
    char str[LANHOST_MAXLINESIZE]={0};
    
    //first line: IP address    HW type     Flags       HW address      Mask        Device
    fgets(str,LANHOST_MAXLINESIZE,fp);
    
    DEBUG_PRINTF("===str: %s=====\n",str);
    
    memset(str,0,LANHOST_MAXLINESIZE);
    int index =0;
    int i=0;
    char tmpchar1[20]={0};
    char tmpchar2[20]={0};
    char tmpchar3[20]={0};
    char tmpchar4[20]={0};
    char tmpchar5[20]={0};
    DEBUG_PRINTF(" tmpchar1: %d  \n",tmpchar1);
    DEBUG_PRINTF(" tmpchar2: %d  \n",tmpchar2);
    DEBUG_PRINTF(" tmpchar3: %d  \n",tmpchar3);
    DEBUG_PRINTF(" tmpchar4: %d  \n",tmpchar4);
    DEBUG_PRINTF(" tmpchar5: %d  \n",tmpchar5);
    char *point=NULL;
    while( ((fgets(str,LANHOST_MAXLINESIZE,fp)) !=NULL ) && (i< *arraynum))
    {
        DEBUG_PRINTF("=====str: %d  %s==\n",strlen(str),str);
        
        sscanf(str,"%s %s %s %s * %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5);

        //praseArpString(str,tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5);
        //DEBUG_PRINTF("len: %d tmpchar1:%s====\n",strlen(tmpchar1),tmpchar1);
        //DEBUG_PRINTF("len: %d tmpchar2:%s====\n",strlen(tmpchar2),tmpchar2);
        //DEBUG_PRINTF("len: %d tmpchar3:%s====\n",strlen(tmpchar3),tmpchar3);
        //DEBUG_PRINTF("len: %d tmpchar4:%s====\n",strlen(tmpchar4),tmpchar4);
        //DEBUG_PRINTF("len: %d tmpchar5:%s====\n",strlen(tmpchar5),tmpchar5);
       
        strncpy(outputArray[i].ipaddr,tmpchar1, sizeof(outputArray[i].ipaddr));
        outputArray[i].HWType=atoi(&(tmpchar2[2]));
        outputArray[i].flags=atoi(&(tmpchar3[2]));
        strncpy(outputArray[i].macaddr,tmpchar4, sizeof(outputArray[i].macaddr));
        strncpy(outputArray[i].device,tmpchar5, sizeof(outputArray[i].device)); 
        i++;
    }
    
    fclose(fp);
    return 0;
}

/**
 *  funcname: check legal lan host entry
 *  	    return 0 is legal lan host 
 *          return -1 is illegal
 */
int checkLanHost(struct arpEntry *input_entry)
{
    char cmd[64]={0};
	char lanip[64]={0};
	char lannetmask[64]={0};
//	int ret;
//	struct fw3_macblock_rule *rule;
//	struct list_head *tmp=NULL;

	ulong l1,l2,l3;
	
	if(strncmp(input_entry->device,LAN_IFNAME,strlen(LAN_IFNAME)) !=0)
	{
		return -1;
	}

	if(strlen(input_entry->ipaddr) == 0 )
	{
		DEBUG_PRINTF("==[%s]====ipaddr is null ===\n",__func__);
		return -1;
	}

	if(strcmp(input_entry->macaddr,"00:00:00:00:00:00") == 0 )
	{
		DEBUG_PRINTF("===[%s]===not get arp response===\n",__func__);
		return -1;
	}
	
	memset(cmd,0,64);
    snprintf(cmd,64,"network.lan.ipaddr");
    rtcfgUciGet(cmd,lanip);

	memset(cmd,0,64);
	snprintf(cmd,64,"network.lan.netmask");
	rtcfgUciGet(cmd,lannetmask);

	DEBUG_PRINTF("===[%s]====lanip: %s=== lannetmask: %s== devip:%s == \n",__func__,lanip,lannetmask,input_entry->ipaddr);
	l1=inet_addr(lanip);
	l2=inet_addr(input_entry->ipaddr);
	l3=inet_addr(lannetmask);

	if((l1&l3) != (l2&l3) )
	{
		DEBUG_PRINTF("[%s]====not in the same subnet with lan ip====\n",__func__);
		return -1;
	}

#if 0
	//get macblock list
	ret=get_macblocktable();
	tmp=global_macblock_rules.next;

	while(tmp!=&global_macblock_rules)
    {
         
        rule = (struct fw3_macblock_rule *)tmp;
   

	    if(strcmp(rule->src_mac,input_entry->macaddr)==0)
	    {
	    	DEBUG_PRINTF("[%s]====== dev entry mac:%s in macblock list ==== \n",__func__,input_entry->macaddr);
			return -1;
	    }
  
        tmp=tmp->next;
    }

    //free_memory
	qtec_fw_free_list(&global_macblock_rules);
#endif

	return 0;
}

/**
 * funcname: getLanHostEntryTableNum
 *           get the num of LanHostEntryTable
 */
int getLanHostEntryTableNum(int *output)
{
    int i=0;
    int j=0;
    while(i<arpEntryTableNum)
    {
        //if(strncmp(arpEntryTable[i].device,LAN_IFNAME,strlen(LAN_IFNAME))==0)
        if(checkLanHost(&(arpEntryTable[i])) == 0)
        {
            i++;
            j++;   
        }
        else
        {
            i++;
        }
    }

    *output=j;
    return 0;
}

/**
 * funcname: getLanHostEntryTable(struct lanHostEntry *outputArray, int *arraynum)
 *           get lanhostarraytable from arpentrytable
 */
int getLanHostEntryTable(struct lanHostEntry *outputArray, int *arraynum)
{
    int i=0;
    int j=0;
    while( (i<arpEntryTableNum) && (j<*arraynum) )
    {
        //if(strncmp(arpEntryTable[i].device,LAN_IFNAME,strlen(LAN_IFNAME))==0)
        if(checkLanHost(&(arpEntryTable[i])) == 0 )
        {
            strcpy(outputArray[j].ipaddr,arpEntryTable[i].ipaddr);
            strcpy(outputArray[j].macaddr,arpEntryTable[i].macaddr);
            if(arpEntryTable[i].flags & 2)
            {
                outputArray[j].online=1;
            }

            DEBUG_PRINTF("=====LanHostEntry=====\n");
            DEBUG_PRINTF("===outputArray[%d].ipaddr: %s====\n",j, outputArray[j].ipaddr);
            DEBUG_PRINTF("===outputArray[%d].macaddr: %s ====\n",j,outputArray[j].macaddr);
            DEBUG_PRINTF("===outputArray[%d].online:%d=====\n",j,outputArray[j].online);
            i++;
            j++;
        }
        else
        {
            i++;
        }
    }
}

/**
 *  funcname: praseDhcpFile
 *              prase dhcp config from /tmp/dhcp.leases to get hostname, connection_type
 */
int praseDhcpFile()
{
    FILE *fp=NULL;
    if( (fp=fopen(DHCP_FILE,"r"))==NULL )
    {
        printf("====ERROR!!! == %s=== can't open FILE: "DHCP_FILE"====\n",__func__);
        return -1;
    }
    int i=0; 
    char str[LANHOST_MAXLINESIZE]={0};
    char tmpchar1[64]={0};
    char tmpchar2[64]={0};
    char tmpchar3[64]={0};
    char tmpchar4[64]={0};
    char tmpchar5[64]={0};
    while( (fgets(str,LANHOST_MAXLINESIZE,fp)) !=NULL )
    {
        DEBUG_PRINTF("===func:%s ==== str==== %s===\n",__func__,str);
        //for example:
        //1497447462 f4:31:c3:10:87:ec 192.168.1.118 iphone 01:f4:31:31:c3:10:87:ec
        sscanf(str,"%s %s %s %s %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5);
        //DEBUG_PRINTF("==tmpchar1: %s====\n",tmpchar1);
        //DEBUG_PRINTF("==tmpchar2: %s====\n",tmpchar2);
        //DEBUG_PRINTF("==tmpchar3: %s====\n",tmpchar3);
        //DEBUG_PRINTF("==tmpchar4: %s====\n",tmpchar4);
        //DEBUG_PRINTF("==tmpchar5: %s====\n",tmpchar5);
            
        for(i=0;i<lanHostEntryTableNum;i++)
        {
            if ( strncmp(lanHostEntryTable[i].macaddr,tmpchar2,strlen(tmpchar2))==0 )
            {
                memset(lanHostEntryTable[i].hostname,0,sizeof(lanHostEntryTable[i].hostname));
                strncpy(lanHostEntryTable[i].hostname,tmpchar4, sizeof(lanHostEntryTable[i].hostname));
                lanHostEntryTable[i].connection_type=1;
                break;
            }
        }
        memset(str,0,LANHOST_MAXLINESIZE);
        memset(tmpchar1,0,64);
        memset(tmpchar2,0,64);
        memset(tmpchar3,0,64);
        memset(tmpchar4,0,64);
        memset(tmpchar5,0,64);
    }

    fclose(fp);
    return 0;
}

//special care: send msg 2 myclient 
//void sc_sendmsg2myclient(char *macaddress, int changed_online)
void sc_sendmsg2myclient(struct specialcareEntry * input_entry, int changed_online)
{
    DEBUG_PRINTF("[%s]====macaddress:%s, hostname:%s  devicetype:%s ===== changed_online %d====\n",__func__,input_entry->macaddr,input_entry->hostname,input_entry->devicetype, changed_online);
    struct VosMsgBody stMsg = {0};
    cJSON *obj = NULL;
    char msg[256]={0};
	if (g_msgHandle == NULL)
	{
        DEBUG_PRINTF("[%s]====g_msgHandle is null ===\n",__func__);
		return;
	}
    if(strlen(input_entry->hostname) == 0 )
    {
        strcpy(input_entry->hostname,"unknow");
    }
    if(strlen(input_entry->devicetype)==0)
    {
        strcpy(input_entry->devicetype,"unknow");
    }
    snprintf(msg,256,"%s %s %s %d",input_entry->macaddr,input_entry->hostname,input_entry->devicetype,changed_online);
    

    DEBUG_PRINTF("[%s]=====msg is %s====\n",__func__,msg);

    memcpy(stMsg.buf, msg, strlen(msg));
    stMsg.stHead.dataLength = strlen(msg) + 1;
	stMsg.stHead.dst = EID_MYWEBSOCKET;
	stMsg.stHead.src = EID_LANHOST;
	stMsg.stHead.type = VOS_MSG_SPECIALCARE_NOTICE;
	stMsg.stHead.flags_response = 1;
	vosMsg_send(g_msgHandle, &stMsg);
    
}

//特殊关注:检测是否有上线和下线提醒发生
//返回 0 代表没有， 返回 1代表上线通知，返回2代表下线通知
int sc_detectchange(struct specialcareEntry * input_entry)
{
    DEBUG_PRINTF("[%s]=====input_entry mac:%s flag is %d===\n",__func__,input_entry->macaddr,input_entry->flag);
    int i=0;
    int j=0;
    int tmp=0;
    
    if(input_entry->flag == 0 )
    {
        return 0;
    }

    for(i=0;i<lanHostEntryTableNum;i++)
    {
        if(strcmp(lanHostEntryTable[i].macaddr, input_entry->macaddr)==0)
        {   
            strncpy(input_entry->hostname,lanHostEntryTable[i].hostname,sizeof(input_entry->hostname));
            tmp = lanHostEntryTable[i].devicetype;
            if( tmp== 0)
            {
                strncpy(input_entry->devicetype,"pc",sizeof(input_entry->devicetype));
            }
            else if(tmp == 1)
            {
                strncpy(input_entry->devicetype,"android",sizeof(input_entry->devicetype));
            }
            else if(tmp == 2)
            {
                strncpy(input_entry->devicetype,"ios",sizeof(input_entry->devicetype));
            }
            else if(tmp == 3)
            {
                strncpy(input_entry->devicetype,"unknow",sizeof(input_entry->devicetype));
            }
            
            for(j=0;j<backupLanHostEntryTableNum;j++)
            {
                if(strcmp(backupLanHostEntryTable[j].macaddr,lanHostEntryTable[i].macaddr)==0)
                {
                    if(backupLanHostEntryTable[j].online == lanHostEntryTable[i].online)
                    {
                        DEBUG_PRINTF("[%s]===macaddress:%s=== online not changed\n",__func__,input_entry->macaddr);
                        return 0;
                    }
                    else
                    {
                        if((lanHostEntryTable[i].online == 1) && (backupLanHostEntryTable[j].online == 0))
                        {
                            return 1;
                        }
                        else if((lanHostEntryTable[i].online == 0) && (backupLanHostEntryTable[j].online == 1))
                        {
                            return 2;
                        }
                    }
                }
            }
            if(j==backupLanHostEntryTableNum)
            {
                DEBUG_PRINTF("[%s]===not find match entry in backup lanhost entry===\n",__func__);
                if(lanHostEntryTable[i].online == 1)
                {
                    return 1;
                }
                else
                    return 0;
            }
            return 0;
        }
    } 

    if(i==lanHostEntryTableNum)
    {
        DEBUG_PRINTF("[%s] === not find match entry in lanhost entry table===\n",__func__);
        for(j=0;j<backupLanHostEntryTableNum;j++)
        {
            if(strcmp(backupLanHostEntryTable[j].macaddr,input_entry->macaddr)==0)
            {
                strncpy(input_entry->hostname,lanHostEntryTable[i].hostname,sizeof(input_entry->hostname));
                tmp = lanHostEntryTable[i].devicetype;
                if( tmp== 0)
                {
                    strncpy(input_entry->devicetype,"pc",sizeof(input_entry->devicetype));
                }
                else if(tmp == 1)
                {
                    strncpy(input_entry->devicetype,"android",sizeof(input_entry->devicetype));
                }
                else if(tmp == 2)
                {
                    strncpy(input_entry->devicetype,"ios",sizeof(input_entry->devicetype));
                }
                else if(tmp == 3)
                {
                    strncpy(input_entry->devicetype,"unknow",sizeof(input_entry->devicetype));
                }
                
                if(backupLanHostEntryTable[j].online==1)
                {
                    return 2;
                }
                else 
                    return 0;
            }
        }

        if(j == backupLanHostEntryTableNum)
        {
            return 0;
        }
    }
    return 0;
}
// 在这个函数处理特殊关注逻辑
// 参数格式为macstring|opt macstring2|opt2
//上线: 如果某个device online是1, 且在备份设备列表里没有它或者 有它，但它显示离线，则表明该设备上线
//下线: 如果某个device online 为0， 但在备份设备列表有它，且它状态为1，则表面该设备离线
void proc_specialcare()
{
    DEBUG_PRINTF("====[%s]=======\n",__func__);
    char specialcare[2048]={0};
    char cmd[64]={0};
    char *delim=" ";
    char *p;
    struct specialcareEntry  specialcareArray[64]={0};
    int index=0;
    int i=0;
    int j=0;
    int k=0;
    char tmpchar[128]={0};
    int ret=0;
    char hostname[64]={0};
    char devicetype[64]={0};
    
    memset(cmd,0,64);
    snprintf(cmd,64,"system.@system[0].specialcare");
    rtcfgUciGet(cmd,specialcare);
    
    if( strlen(specialcare) == 0)
    {
        DEBUG_PRINTF("[%s]====no device in specialcare====\n",__func__);
        return;
    }

    

    p=strtok(specialcare,delim);
    DEBUG_PRINTF("[%s]====p:%s=======\n",__func__,p);
    
    memset(tmpchar,0,sizeof(tmpchar));
    strncpy(tmpchar,p,sizeof(tmpchar)-1);
    specialcareArray[index].flag=atoi(&(tmpchar[strlen(tmpchar)-1]));
    tmpchar[strlen(tmpchar)-2]='\0';
    strncpy(specialcareArray[index].macaddr,tmpchar,sizeof(specialcareArray[index].macaddr)-1);
    index++;

    while((p=strtok(NULL,delim)))
    {
        memset(tmpchar,0,sizeof(tmpchar));
        strncpy(tmpchar,p,sizeof(tmpchar));
        specialcareArray[index].flag=atoi(&(tmpchar[strlen(tmpchar)-1]));
        tmpchar[strlen(tmpchar)-2]='\0';
        strncpy(specialcareArray[index].macaddr,tmpchar,sizeof(specialcareArray[index].macaddr)-1);
        index++;
    }
    //至此获取到特殊关注的设备及对应的flag
    for(i=0;i<index;i++)
    {
        DEBUG_PRINTF("[%s]  specialcareArray[%d] macaddr:%s flag:%d====\n",__func__,i,specialcareArray[i].macaddr,specialcareArray[i].flag);
        
    }

    for(i=0;i<index;i++)
    {
        ret=sc_detectchange(&(specialcareArray[i]));
        if((ret & (specialcareArray[i].flag))!=0 )
        {
            sc_sendmsg2myclient(specialcareArray[i].macaddr,ret);
        }  
    }
}


/*
 * record new online host time
 * if online state did not change, time is same as last recode.
*/
void updateInfo()
{
    int i = 0;
    int j = 0;
	FILE *fp = NULL;
	char buffer[64] = {0};
	char tmp1[64] = {0};
	char tmp2[64] = {0};
    fp = fopen("/proc/uptime", "r+");
    if (!fp)
    {
        printf("Fail to open uptime\n");
        return;
    }

    if(fgets(buffer, 64, fp) != NULL)
	{
		sscanf(buffer, "%s %s", tmp1, tmp2);
		printf("buffer:%s,tmp1:%s,tmp2:%s.\n", buffer, tmp1, tmp2);
		
	}
    for(i = 0; i < lanHostEntryTableNum; i++)
    {
       
	    lanHostEntryTable[i].time = atoi(tmp1);
		
		printf("lanHostEntryTable[%d].time is:%ld.\n", i, lanHostEntryTable[i].time);
        /* if online state did not change, time is same as last recode */
        for(j = 0; j < backupLanHostEntryTableNum; j++)
        {
            if(strcmp(lanHostEntryTable[i].ipaddr, backupLanHostEntryTable[j].ipaddr) == 0)
            {
                if(lanHostEntryTable[i].online == backupLanHostEntryTable[j].online)
                {
                    lanHostEntryTable[i].time = backupLanHostEntryTable[j].time;
					printf("update lanHostEntryTable[%d].time is:%d.\n", i, lanHostEntryTable[i].time);
                }
      			break;
            }
        }
    }

	/* 检查eth_type，如果该状态从无线状态变更了，说明该设备下线 */
    for(j = 0; j < backupLanHostEntryTableNum; j++)
    {
        if(strcmp(backupLanHostEntryTable[j].eth_type, "eth1") != 0)
        {
            for(i = 0; i < lanHostEntryTableNum; i++)
            {
                if((strcasecmp(lanHostEntryTable[i].macaddr, backupLanHostEntryTable[j].macaddr) == 0) && (strcasecmp(lanHostEntryTable[i].ipaddr, backupLanHostEntryTable[j].ipaddr) == 0))
                {
					if(strcmp(lanHostEntryTable[i].eth_type, "eth1") == 0)
					{
						strcpy(lanHostEntryTable[i].eth_type, backupLanHostEntryTable[j].eth_type);
                    	lanHostEntryTable[i].online = 0; 
					}
					break;
                }
            }
        }
    }

    fclose(fp);
    fp = NULL;
}

/*
 * funcname: lanHostMainLogic
 *      the main logic is in below:
 *      a. get arp entry table from /proc/net/arp
 *      b. depends on device name, to check lanhostEntrytable from arp entry table
 *          in this step, can check below parameter:
 *            1. ip ;
 *            2. mac;
 *            3. online;
 *      c. get hostname and connection_type after check dhcp files
 *            4.hostname
 *            5.connection_type
 */
int lanHostMainLogic()
{
    //firstly, clean the pre-malloc data
    arpEntryTableNum=0;
    #if 0
    if(arpEntryTable!=NULL)
    {
        free(arpEntryTable);
        arpEntryTable = NULL;
    }
    #endif
    lanHostEntryTableNum=0;
    #if 0
    if(lanHostEntryTable !=NULL)
    {
        free(lanHostEntryTable);
        lanHostEntryTable = NULL;
    }
    #endif
    getArpEntryTableNum(&arpEntryTableNum);
    arpEntryTableNum = MIN(arpEntryTableNum, 128);
    DEBUG_PRINTF("=====[%s] arpEntryTableNum: %d ====\n", __func__,arpEntryTableNum);

    if(arpEntryTableNum !=0 )
    {
        //arpEntryTable=malloc(arpEntryTableNum * sizeof(struct arpEntry));
        memset(arpEntryTable,0,(arpEntryTableNum * sizeof(struct arpEntry)));
    }
    else
    {
        printf("===warning===there are no arp entry===\n");
    }

    getArpEntryTable(arpEntryTable,&arpEntryTableNum);


    getLanHostEntryTableNum(&lanHostEntryTableNum);
    lanHostEntryTableNum = MIN(lanHostEntryTableNum, 128);

    DEBUG_PRINTF("====lanHostEntryTableNum: %d===\n", lanHostEntryTableNum);
    if(lanHostEntryTableNum != 0)
    {
        //lanHostEntryTable=malloc(lanHostEntryTableNum * sizeof(struct lanHostEntry));
        memset(lanHostEntryTable,0,(lanHostEntryTableNum * sizeof(struct lanHostEntry)));
    }
    else
    {
        printf("====WARNING=== no lanhostentry====\n");
    }

    getLanHostEntryTable(lanHostEntryTable,&lanHostEntryTableNum);
    
    praseDhcpFile();

    
    return 0;
}

/**
 * func_name: outputAllLanHostInfo
 *          output all lanhost info
 *
 */
void outputAllLanHostInfo(struct lanHostEntry *input, int *arraynum)
{
    struct lanHostEntry *outputArray=input;
    int num=0;
    num = lanHostEntryTableNum;
    printf("#########num:%d##########%s#%d##\n", num, __FUNCTION__, __LINE__);
    *arraynum =num;
    int i=0;
    if( num != 0)
    {
        //outputArray=malloc(num * sizeof(struct lanHostEntry));
        memset(outputArray,0,(num*sizeof(struct lanHostEntry)));
        for(i=0;i<num;i++)
        {
            outputArray[i].online=lanHostEntryTable[i].online;
            strcpy(outputArray[i].ipaddr,lanHostEntryTable[i].ipaddr);
            strcpy(outputArray[i].macaddr,lanHostEntryTable[i].macaddr);
            strcpy(outputArray[i].hostname,lanHostEntryTable[i].hostname);
			strcpy(outputArray[i].eth_type,lanHostEntryTable[i].eth_type);
			outputArray[i].devicetype=lanHostEntryTable[i].devicetype;
            outputArray[i].connection_type=lanHostEntryTable[i].connection_type;
            outputArray[i].rx=lanHostEntryTable[i].rx;
            outputArray[i].tx=lanHostEntryTable[i].tx;
			outputArray[i].time=lanHostEntryTable[i].time;
        }
    }
    else
    {
        printf("===WARNING!  there is no lan host ===\n");
    }printf("#########num:%d##########%s#%d##\n", num, __FUNCTION__, __LINE__);
    return;
}

/**
 * func_name: outputOnlineLanHostInfo
 *          output online lan host info
 *
 */
struct lanHostEntry* outputOnlineLanHostInfo(int *arraynum)
{

    struct lanHostEntry *outputArray=NULL;
    int i=0;
    int j=0;
    int num=0;
    for(i=0;i<lanHostEntryTableNum;i++)
    {
        if(lanHostEntryTable[i].online==1)
            j++;
    }
    num =j;
    *arraynum = num;
    
    if( num != 0)
    {
        outputArray =malloc( num * sizeof(struct lanHostEntry));
        memset(outputArray, 0, (num*sizeof(struct lanHostEntry)));

        i=0;
        j=0;
        while(i<lanHostEntryTableNum)
        {
            if(lanHostEntryTable[i].online==1)
            {
                outputArray[j].online=lanHostEntryTable[i].online;
                strcpy(outputArray[j].ipaddr,lanHostEntryTable[i].ipaddr);
                strcpy(outputArray[j].macaddr,lanHostEntryTable[i].macaddr);
                strcpy(outputArray[j].hostname,lanHostEntryTable[i].hostname);
				strcpy(outputArray[j].eth_type,lanHostEntryTable[i].eth_type);
				outputArray[j].devicetype=lanHostEntryTable[i].devicetype;
                outputArray[j].connection_type=lanHostEntryTable[i].connection_type;
                outputArray[j].rx=lanHostEntryTable[i].rx;
                outputArray[j].tx=lanHostEntryTable[i].tx;
				outputArray[j].time=lanHostEntryTable[i].time;
                i++;
                j++;
            }
            else
            {
                i++;
            }
        }
    }
    else
    {
        printf("===WARNING ! there is no online lan host===\n"); 
    }
    return outputArray;
}

/**
 *   func_name: showLanHostEntryTable
 */
void showLanHostEntryTable()
{
    int i=0;
    DEBUG_PRINTF("=====showLanHostEntryTable ================\n");
    for(i=0;i<lanHostEntryTableNum;i++)
    {
        DEBUG_PRINTF("======lanHostEntryTable[%d].online:       %d =====\n", i, lanHostEntryTable[i].online);
        DEBUG_PRINTF("======lanHostEntryTable[%d].ipaddr:       %s =====\n", i, lanHostEntryTable[i].ipaddr);
        DEBUG_PRINTF("======lanHostEntryTable[%d].macaddr:      %s =====\n", i, lanHostEntryTable[i].macaddr);
        DEBUG_PRINTF("======lanHostEntryTable[%d].hostname:     %s =====\n", i, lanHostEntryTable[i].hostname);
        DEBUG_PRINTF("======lanHostEntryTable[%d].connect_type: %d =====\n", i, lanHostEntryTable[i].connection_type);
        DEBUG_PRINTF("======lanHostEntryTable[%d].eth_type:     %s =====\n", i, lanHostEntryTable[i].eth_type);
        DEBUG_PRINTF("======lanHostEntryTable[%d].rx:           %d =====\n", i, lanHostEntryTable[i].rx);
        DEBUG_PRINTF("======lanHostEntryTable[%d].tx:           %d =====\n", i, lanHostEntryTable[i].tx);
    }
}

/**
 *  func_name: syncinformation 
 *            sync information from lanHostEntryTable to backupLanHostEntryTable
 */
void syncinformation()
{
    DEBUG_PRINTF("===============%s=============\n",__func__);
    backupLanHostEntryTableNum=0;
    #if 0
    if(backupLanHostEntryTable !=NULL)
    {
        free(backupLanHostEntryTable);
        backupLanHostEntryTable = NULL;
    }
    #endif
    outputAllLanHostInfo(backupLanHostEntryTable, &backupLanHostEntryTableNum);

}

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

void callWwanSpeed()
{
    char wwanIfname[16] = {0};
    char cmd[256] = {0};
    
    rtcfgUciGet("network.wwan.ifname", wwanIfname);
    
    system("iptables -N Callwanuploadspeed");
    system("iptables -N Callwandownloadspeed");
    system("iptables -I FORWARD -j Callwandownloadspeed");
    system("iptables -I FORWARD -j Callwanuploadspeed");

    memset(cmd,0,256);
    snprintf(cmd,256,"iptables -A Callwanuploadspeed -i %s",wwanIfname);
    system(cmd);
    memset(cmd,0,256);
    snprintf(cmd,256,"iptables -A Callwandownloadspeed -o %s",wwanIfname);
    system(cmd);

    sleep(1);

    system("iptables -nvL Callwanuploadspeed > /tmp/.wanuploadspeed");
    system("iptables -nvL Callwandownloadspeed > /tmp/.wandownloadspeed");

    FILE *up=NULL;
    FILE *down=NULL;

    up = fopen("/tmp/.wanuploadspeed","r");

    /*
    Chain Callanuploadspeed (4 references)
    pkts bytes target     prot opt in     out     source               destination
    0     0            all  --  *      *       192.168.1.100        0.0.0.0/0
    */
    if( up == NULL)
    {
        printf("====ERROR!!!=== %s==== can't open wanuploadspeed file ",__func__);
        return;
    }

    char str[256]={0};
    char tmpchar1[64]={0};
    char tmpchar2[64]={0};
    char tmpchar3[64]={0};
    char tmpchar4[64]={0};
    char tmpchar5[64]={0};
    char tmpchar6[64]={0};
    char tmpchar7[64]={0};
    char tmpchar8[64]={0};
    char datalen = 0;
    fgets(str,256,up);
    fgets(str,256,up);
    memset(str,0,256);
    fgets(str,256,up);
    sscanf(str,"%s %s %s %s %s %s %s %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5,tmpchar6,tmpchar7,tmpchar8);

    datalen = strlen(tmpchar2);
    if(datalen > 0)
    {
        if(tmpchar2[datalen - 1] == 'K')
        {
            g_routerrx = atoi(tmpchar2) * 1024;
        }
        else if(tmpchar2[datalen - 1] == 'M')
        {
            g_routerrx = atoi(tmpchar2) * 1024 * 1024;
        }
        else
        {
            g_routerrx = atoi(tmpchar2);
        }
    }
    
    memset(tmpchar1,0,64);
    memset(tmpchar2,0,64);
    memset(tmpchar3,0,64);
    memset(tmpchar4,0,64);
    memset(tmpchar5,0,64);
    memset(tmpchar6,0,64);
    memset(tmpchar7,0,64);
    memset(tmpchar8,0,64);

    fclose(up);

    down = fopen("/tmp/.wandownloadspeed","r");

    /*
    Chain Callanuploadspeed (4 references)
    pkts bytes target     prot opt in     out     source               destination
    0     0            all  --  *      *       192.168.1.100        0.0.0.0/0
    */
    if( down == NULL)
    {
        printf("====ERROR!!!=== %s==== can't open wandownloadspeed file ",__func__);
        return;
    }
    
    fgets(str,256,down);
    fgets(str,256,down);
    memset(str,0,256);
    fgets(str,256,down);
    sscanf(str,"%s %s %s %s %s %s %s %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5,tmpchar6,tmpchar7,tmpchar8);

    datalen = strlen(tmpchar2);
    if(datalen > 0)
    {
        if(tmpchar2[datalen - 1] == 'K')
        {
            g_routertx = atoi(tmpchar2) * 1024;
        }
        else if(tmpchar2[datalen - 1] == 'M')
        {
            g_routertx = atoi(tmpchar2) * 1024 * 1024;
        }
        else
        {
            g_routertx = atoi(tmpchar2);
        }
    }

    fclose(down);

    if(g_routertx > g_htx)
    {
        g_htx = g_routertx;
    }

    if(g_routerrx > g_hrx)
    {
        g_hrx = g_routerrx;
    }
    
    memset(cmd,0,256);
    snprintf(cmd,256,"iptables -D Callwanuploadspeed -i %s",wwanIfname);
    system(cmd);
    memset(cmd,0,256);
    snprintf(cmd,256,"iptables -D Callwandownloadspeed -o %s",wwanIfname);
    system(cmd);
    
    system("iptables -D FORWARD -j Callwanuploadspeed");
    system("iptables -D FORWARD -j Callwandownloadspeed");
    system("iptables -Z Callwanuploadspeed");
    system("iptables -Z Callwandownloadspeed");
    system("iptables -X Callwanuploadspeed");
    system("iptables -X Callwandownloadspeed");
        
}

void callRouterSpeed()
{
	DEBUG_PRINTF("=========%s===========\n",__func__);
    int starttx, startrx, endtx, endrx;
    char buffer[64] = {0};
    FILE *fptx = NULL;
    FILE *fprx = NULL;
    FILE *fp = NULL;
    int wwanflag = 0;
    char status[8] = {0};
    char wwanIfname[16] = {0}; 

    system("ubus call network.interface.wwan status | grep '\"up\"' | sed -e 's/^.*: \\(.*\\),/\\1/g' > /tmp/wdsup");
    fp = fopen("/tmp/wdsup","r");
    if (fp != NULL)
    {
        fgets(status, sizeof(status), fp);
        if (!strncmp(status, "true", 4))
        {
            wwanflag = 1;
        }
        fclose(fp);
        fp = NULL;
    }

    if(wwanflag)
    {
        callWwanSpeed();
        return;
    }
    
    system("ifconfig eth0 | grep 'RX bytes' | sed -e 's/^.*:\\([0-9]*\\).*/\\1/g' >/tmp/routertx");
    system("ifconfig eth0 | grep 'RX bytes' | sed -e 's/^.*RX\\ bytes:\\([0-9]*\\).*/\\1/g' >/tmp/routerrx");
    fptx = fopen("/tmp/routertx", "r");
    if(fptx)
    {
        memset(buffer, 0, 64);
        fgets(buffer, 64, fptx);  
        sscanf(buffer,"%d\n", &starttx);
        printf("starttx is:%d.\n", starttx);
        fclose(fptx);
        fptx = NULL;
    }
    fprx = fopen("/tmp/routerrx", "r");
    if(fprx)
    {
        memset(buffer, 0, 64);
        fgets(buffer, 64, fprx);
        sscanf(buffer,"%d\n", &startrx);
        printf("startrx is:%d.\n", startrx);
        fclose(fprx);
        fprx = NULL;
    }
    sleep(1);

    system("ifconfig eth0 | grep 'RX bytes' | sed -e 's/^.*:\\([0-9]*\\).*/\\1/g' >/tmp/routertx");
    system("ifconfig eth0 | grep 'RX bytes' | sed -e 's/^.*RX\\ bytes:\\([0-9]*\\)\.*/\\1/g' >/tmp/routerrx");
    
    fptx = fopen("/tmp/routertx", "r");
    if(fptx)
    {
        memset(buffer, 0, 64);
        fgets(buffer, 64, fptx);
        sscanf(buffer,"%d", &endtx);
		printf("endtx is:%d.\n", endtx);
        fclose(fptx);
        fptx = NULL;
    }
    fprx = fopen("/tmp/routerrx", "r");
    if(fprx)
    {
        memset(buffer, 0, 64);
        fgets(buffer, 64, fprx);
        sscanf(buffer,"%d", &endrx);
        printf("endrx is:%d.\n", endrx);
        fclose(fprx);
        fprx = NULL;
    }

    g_routertx = endtx - starttx;
    g_routerrx = endrx - startrx;
    if(g_routertx > g_htx)
    {
        g_htx = g_routertx;
    }

    if(g_routerrx > g_hrx)
    {
        g_hrx = g_routerrx;
    }
    printf("g_routertx:%d, g_routerrx:%d.", g_routertx, g_routerrx);
	
	return;
}

/**
 *  function_name: calLanSpeed
 *                calculate lan client network upload and download speed
 *                the rule will be written into FORWARD chain / filter table
 *			2017/9/28: wjj: seems iptables capture few packets 
 */
void calLanSpeed()
{
    system("iptables -N Callanuploadspeed");
    system("iptables -N Callandownloadspeed");
    system("iptables -I FORWARD -j Callanuploadspeed");
    system("iptables -I FORWARD -j Callandownloadspeed");
    //system("iptables -N DOWNLOAD");
    char cmd[256]={0};
    //for example: iptables -I Callanspeed -s 192.168.1.100
    int i=0;
    for(i=0;i<lanHostEntryTableNum;i++)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -A Callanuploadspeed -s %s",lanHostEntryTable[i].ipaddr);
        system(cmd);
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -A Callandownloadspeed -d %s",lanHostEntryTable[i].ipaddr);
        system(cmd);
    }
    sleep(1);

    system("iptables -nvL Callanuploadspeed > /tmp/.lanuploadspeed");
    system("iptables -nvL Callandownloadspeed > /tmp/.landownloadspeed");

    FILE *up=NULL;
    FILE *down=NULL;

    up = fopen("/tmp/.lanuploadspeed","r");

    /*
    Chain Callanuploadspeed (4 references)
    pkts bytes target     prot opt in     out     source               destination
    0     0            all  --  *      *       192.168.1.100        0.0.0.0/0
    */
    if( up == NULL)
    {
        printf("====ERROR!!!=== %s==== can't open lanuploadspeed file ",__func__);
        return;
    }

    char str[LANHOST_MAXLINESIZE]={0};
    char tmpchar1[64]={0};
    char tmpchar2[64]={0};
    char tmpchar3[64]={0};
    char tmpchar4[64]={0};
    char tmpchar5[64]={0};
    char tmpchar6[64]={0};
    char tmpchar7[64]={0};
    char tmpchar8[64]={0};
    fgets(str,LANHOST_MAXLINESIZE,up);
    fgets(str,LANHOST_MAXLINESIZE,up);
    memset(str,0,LANHOST_MAXLINESIZE);
    i=0;
    while ( (fgets(str,LANHOST_MAXLINESIZE,up)) !=NULL )
    {
        DEBUG_PRINTF("==func: %s=====str====%s==i:%d===\n",__func__,str, i);
    
        sscanf(str,"%s %s %s %s %s %s %s %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5,tmpchar6,tmpchar7,tmpchar8);
        //DEBUG_PRINTF("===tmpchar1: %s====\n",tmpchar1);
        //DEBUG_PRINTF("===tmpchar2: %s====\n",tmpchar2);
        //DEBUG_PRINTF("===tmpchar3: %s====\n",tmpchar3);
        //DEBUG_PRINTF("===tmpchar4: %s====\n",tmpchar4);
        //DEBUG_PRINTF("===tmpchar5: %s====\n",tmpchar5);
        //DEBUG_PRINTF("===tmpchar6: %s====\n",tmpchar6);
        //DEBUG_PRINTF("===tmpchar7: %s====\n",tmpchar7);
        //DEBUG_PRINTF("===tmpchar8: %s====\n",tmpchar8);

        lanHostEntryTable[i].tx=atoi(tmpchar2);
        memset(tmpchar1,0,64);
        memset(tmpchar2,0,64);
        memset(tmpchar3,0,64);
        memset(tmpchar4,0,64);
        memset(tmpchar5,0,64);
        memset(tmpchar6,0,64);
        memset(tmpchar7,0,64);
        memset(tmpchar8,0,64);
        i++;
    }
    DEBUG_PRINTF("==func: %s===%d===========\n",__func__,__LINE__);
    fclose(up);

    down = fopen("/tmp/.landownloadspeed","r");

    /*
    Chain Callanuploadspeed (4 references)
    pkts bytes target     prot opt in     out     source               destination
    0     0            all  --  *      *       192.168.1.100        0.0.0.0/0
    */
    if( down == NULL)
    {
        printf("====ERROR!!!=== %s==== can't open lanuploadspeed file ",__func__);
        return;
    }

    fgets(str,LANHOST_MAXLINESIZE,down);
    fgets(str,LANHOST_MAXLINESIZE,down);
    
    memset(str,0,LANHOST_MAXLINESIZE);
    i=0;
    while ( (fgets(str,LANHOST_MAXLINESIZE,down)) !=NULL )
    {
        DEBUG_PRINTF("==func: %s=====str====%s==i:%d===\n",__func__,str, i);
    
        sscanf(str,"%s %s %s %s %s %s %s %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5,tmpchar6,tmpchar7,tmpchar8);
        //DEBUG_PRINTF("===tmpchar1: %s====\n",tmpchar1);
        //DEBUG_PRINTF("===tmpchar2: %s====\n",tmpchar2);
        //DEBUG_PRINTF("===tmpchar3: %s====\n",tmpchar3);
        //DEBUG_PRINTF("===tmpchar4: %s====\n",tmpchar4);
        //DEBUG_PRINTF("===tmpchar5: %s====\n",tmpchar5);
        //DEBUG_PRINTF("===tmpchar6: %s====\n",tmpchar6);
        //DEBUG_PRINTF("===tmpchar7: %s====\n",tmpchar7);
        //DEBUG_PRINTF("===tmpchar8: %s====\n",tmpchar8);

        lanHostEntryTable[i].rx=atoi(tmpchar2);
        memset(tmpchar1,0,64);
        memset(tmpchar2,0,64);
        memset(tmpchar3,0,64);
        memset(tmpchar4,0,64);
        memset(tmpchar5,0,64);
        memset(tmpchar6,0,64);
        memset(tmpchar7,0,64);
        memset(tmpchar8,0,64);
        i++;
    }
    DEBUG_PRINTF("==func: %s===%d===========\n",__func__,__LINE__);
    fclose(down);

    //showLanHostEntryTable();
  
    

    //clean the iptables rule
    for(i=0;i<lanHostEntryTableNum;i++)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -D Callanuploadspeed -s %s",lanHostEntryTable[i].ipaddr);
        system(cmd);
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -D Callandownloadspeed -d %s",lanHostEntryTable[i].ipaddr);
        system(cmd);
    }
    
    system("iptables -D FORWARD -j Callanuploadspeed");
    system("iptables -D FORWARD -j Callandownloadspeed");
    system("iptables -Z Callanuploadspeed");
    system("iptables -Z Callandownloadspeed");
    system("iptables -X Callanuploadspeed");
    system("iptables -X Callandownloadspeed");
    

}

/**
 *  function_name: calLanSpeed2
 *                calculate lan client network upload and download speed
 *                the rule will be written into FORWARD chain / filter table
 *			2017/9/28: wjj: seems iptables capture few packets, so use ebtables 
 */

void calLanSpeed2()
{
	DEBUG_PRINTF("=====[%s]======\n",__func__);
    system("ebtables -F Callanuploadspeed");
    system("ebtables -F Callandownloadspeed");
	int i=0;
	FILE *up=NULL;
    FILE *down=NULL;
    FILE *fp = NULL;
	char str[LANHOST_MAXLINESIZE]={0};
	char cmd[256]={0};
    char tmp_char[64]={0};
	int tmp_int=0;

	for(i=0;i<lanHostEntryTableNum;i++)
    {
    	if(lanHostEntryTable[i].online != 0)
    	{
        	memset(cmd,0,256);
        	snprintf(cmd,256,"ebtables -A Callanuploadspeed -s %s",lanHostEntryTable[i].macaddr);
        	system(cmd);
        	memset(cmd,0,256);
        	snprintf(cmd,256,"ebtables -A Callandownloadspeed -d %s",lanHostEntryTable[i].macaddr);
        	system(cmd);
    	}
    }

	/*
	  Bridge table: filter

	  Bridge chain: INPUT, entries: 1, policy: ACCEPT
	  -s 4c:cc:6a:e3:5a:d5 -j CONTINUE , pcnt = 1805 -- bcnt = 17218
	*/

	
    system("ebtables -L Callanuploadspeed --Lc  | awk 'NR>3{printf $2 \" \" $12 \" \\n\"}' > /tmp/.lanuploadspeed");
    system("ebtables -L Callandownloadspeed --Lc | awk 'NR>3{printf $2 \" \" $12 \" \\n\"}' > /tmp/.landownloadspeed");

	up = fopen("/tmp/.lanuploadspeed","r");

	if( up == NULL)
    {
        printf("====ERROR!!!=== %s==== can't open lanuploadspeed file ",__func__);
        return;
    }
	
	i=0;
	while ( (fgets(str,LANHOST_MAXLINESIZE,up)) !=NULL )
    {
        DEBUG_PRINTF("==func: %s=====str====%s=====\n",__func__,str);
    	memset(tmp_char,0,sizeof(tmp_char));
        sscanf(str,"%s %d",tmp_char,&tmp_int);
    	
        for(i=0;i<lanHostEntryTableNum;i++)
        {
        	//if(strncmp(lanHostEntryTable[i].macaddr,tmp_char,strlen(tmp_char)) == 0)
            if( !(check_mac_match(lanHostEntryTable[i].macaddr,tmp_char)) )
            {
        		lanHostEntryTable[i].tx = tmp_int;
				break;
        	}
        }
		DEBUG_PRINTF("[%s]==i:%d===\n",__func__,i);
     
    }

	if(up)
	{	
		fclose(up);
	}

	
	down = fopen("/tmp/.landownloadspeed","r");

	if( down == NULL)
    {
        printf("====ERROR!!!=== %s==== can't open landownloadspeed file ",__func__);
        return;
    }
	
	i=0;
	while ( (fgets(str,LANHOST_MAXLINESIZE,down)) !=NULL )
    {
        DEBUG_PRINTF("==func: %s=====str====%s=====\n",__func__,str);
    	memset(tmp_char,0,sizeof(tmp_char));
        sscanf(str,"%s %d",tmp_char,&tmp_int);
    	
        for(i=0;i<lanHostEntryTableNum;i++)
        {
        	//if(strncmp(lanHostEntryTable[i].macaddr,tmp_char,strlen(tmp_char)) == 0)
        	if( !(check_mac_match(lanHostEntryTable[i].macaddr,tmp_char)) )
        	{
        		lanHostEntryTable[i].rx = tmp_int;
				break;
        	}
        }
		DEBUG_PRINTF("[%s]==i:%d===\n",__func__,i);
    }

	if(down)
	{	
		fclose(down);
	}
	

	for(i=0;i<lanHostEntryTableNum;i++)
    {
    	if(lanHostEntryTable[i].online != 0)
    	{
        	memset(cmd,0,256);
        	snprintf(cmd,256,"ebtables -D Callanuploadspeed -s %s",lanHostEntryTable[i].macaddr);
        	system(cmd);
        	memset(cmd,0,256);
        	snprintf(cmd,256,"ebtables -D Callandownloadspeed -d %s",lanHostEntryTable[i].macaddr);
        	system(cmd);
    	}
    }
    

}

//先检查mac出现次数，若mac出现多次，则要看该成员是否为首次上线或者ethtype是一致来决定是否要更新ethtype
void setAccessType(char *macaddr, char *ethtype)
{
    int i = 0;
    int j = 0;
    int findtimes = 0;
    int findflag = 0;
    int setflag = 0;

    //检查mac出现次数
    while(i < lanHostEntryTableNum)
    {
        if(strcasecmp(lanHostEntryTable[i].macaddr,macaddr) == 0)
        {
            findtimes++;
        }
        i++;
    }

    //mac出现多次，特殊处理
    if(findtimes > 1)
    {
        i = 0;
        while(i < lanHostEntryTableNum)
        {
            if(strcasecmp(lanHostEntryTable[i].macaddr,macaddr) == 0)
            {
                for(j = 0; j < backupLanHostEntryTableNum; j++)
                {
                    findflag = 0;
                    if((strcasecmp(lanHostEntryTable[i].macaddr, backupLanHostEntryTable[j].macaddr) == 0) && (strcasecmp(lanHostEntryTable[i].ipaddr, backupLanHostEntryTable[j].ipaddr) == 0))
                    {
                        if(strcmp(backupLanHostEntryTable[j].eth_type, ethtype) == 0)
                        {
                            strcpy(lanHostEntryTable[i].eth_type,ethtype);
                            setflag = 1;
                        }
                        findflag = 1;
                        break;
                    }
                }
                if(findflag == 0)
                {
                    strcpy(lanHostEntryTable[i].eth_type,ethtype);
                    lanHostEntryTable[i].online = 1;
                    setflag = 1;
                }
                if(setflag == 1)
                {
                    printf("set access type finished\n");
                    break;
                }
            }
            i++;
        }
    }
    //mac出现一次普通处理
    else if(findtimes == 1)
    {
        i = 0;
        while(i < lanHostEntryTableNum)
        {
            if(strcasecmp(lanHostEntryTable[i].macaddr,macaddr) == 0)
            {
                strcpy(lanHostEntryTable[i].eth_type,ethtype);
                lanHostEntryTable[i].online = 1;
                break;
            }
            i++;
        }
    }
}

/**
 *  function_name: calLanSpeed
 *                calculate lan client network upload and download speed
 *                the rule will be written into FORWARD chain / filter table
 */
void callAccessType()
{
    char macaddr[20] = {0};
    char buf[256] = {0};
    char tmp[128] = {0};
    int i = 0;
    
    FILE *ath0 = NULL;
    FILE *ath1 = NULL;
    FILE *ath01 = NULL;
    FILE *ath11 = NULL;

    /* set fefault eth_type */
    while(i < lanHostEntryTableNum)
    {
        strcpy(lanHostEntryTable[i].eth_type, "eth1");
        i++;
    }
    
    system("wlanconfig ath0 list >/tmp/ath0.txt");
    system("wlanconfig ath1 list >/tmp/ath1.txt");
    system("wlanconfig ath01 list >/tmp/ath01.txt");
    system("wlanconfig ath11 list >/tmp/ath11.txt");
    
    ath0 = fopen("/tmp/ath0.txt", "r");
    ath1 = fopen("/tmp/ath1.txt", "r");
    ath01 = fopen("/tmp/ath01.txt", "r");
    ath11 = fopen("/tmp/ath11.txt", "r");

    if (ath0 != NULL)
	{
		while (fgets(buf, 256, ath0)!= NULL)
		{
			if (sscanf(buf, "%s %s", macaddr, tmp) == 2)
            {         
                if (!util_strncmp(macaddr, "ADDR", 4))
                {
                    continue;
                }
    			setAccessType(macaddr, "ath0");
    			//fgets(buf, 100, ath0);
    			//fgets(buf, 100, ath0);
                //fgets(buf, 100, ath0);
                memset(buf, 0, 256);
            }
		}
        fclose(ath0);
	}

    if (ath1 != NULL)
	{
		while (fgets(buf, 256, ath1)!= NULL)
		{DEBUG_PRINTF("[%s][%d]buf:%s\n",__FUNCTION__, __LINE__, buf)
			if (sscanf(buf, "%s %s", macaddr, tmp) == 2)
            {       DEBUG_PRINTF("[%s][%d]macaddr:%s, length:%d\n",__FUNCTION__, __LINE__, macaddr, strlen(macaddr));
                if (!util_strncmp(macaddr, "ADDR", 4))
                {DEBUG_PRINTF("[%s][%d]continue\n",__FUNCTION__, __LINE__);
                    continue;
                }
    			setAccessType(macaddr, "ath1");
    			//fgets(buf, 100, ath1);
    			//fgets(buf, 100, ath1);
                //fgets(buf, 100, ath1);
                memset(buf, 0, 256);
            }
		}
        fclose(ath1);
	}

    if (ath01 != NULL)
	{
		while (fgets(buf, 256, ath01)!= NULL)
		{
			if (sscanf(buf, "%s %s", macaddr, tmp) == 2)
            {         
                if (!util_strncmp(macaddr, "ADDR", 4))
                {
                    continue;
                }
    			setAccessType(macaddr, "ath01");
    			//fgets(buf, 100, ath01);
    			//fgets(buf, 100, ath01);
                //fgets(buf, 100, ath01);
                memset(buf, 0, 256);
            }
		}
        fclose(ath01);
	}

    if (ath11 != NULL)
	{
		while (fgets(buf, 256, ath11)!= NULL)
		{
			if (sscanf(buf, "%s %s", macaddr, tmp) == 2)
            {         
                if (!util_strncmp(macaddr, "ADDR", 4))
                {
                    continue;
                }
    			setAccessType(macaddr, "ath11");
    			//fgets(buf, 100, ath11);
    			//fgets(buf, 100, ath11);
                //fgets(buf, 100, ath11);
                memset(buf, 0, 256);
            }
		}
        fclose(ath11);
	}
    
}

//get device type from uci qtec_capture
void setDeviceType()
{
    int i = 0;
    int j = 0;
    int ret = 0;
    char cmd[256] = {0};
    char macaddr[64] = {0};
    char devtype[64] = {0};

    for(i = 0; i < lanHostEntryTableNum; i++)
    {
        j = -1;
		ret = 0;
        lanHostEntryTable[i].devicetype = 3;
        while(ret == 0)
        {
            j++;
            memset(cmd,0,256);
            snprintf(cmd,256,"qtec_capture.@lan_entry[%d].macaddr", j);
            memset(macaddr,0,64);
            ret = rtcfgUciGet(cmd,macaddr);
            if(0 ==ret)
            {
                if(0 == strncasecmp(macaddr, lanHostEntryTable[i].macaddr, 64))
                {
                    memset(cmd,0,256);
                    snprintf(cmd,256,"qtec_capture.@lan_entry[%d].device_type", j);
                    memset(devtype,0,64);
                    ret = rtcfgUciGet(cmd,devtype);
                    //unknow
                    if(atoi(devtype) == 0)
                    {
                        lanHostEntryTable[i].devicetype = 3;
                    }
                    else if((atoi(devtype) == 1) || (atoi(devtype) == 2))
                    {
                        lanHostEntryTable[i].devicetype = 2;
                    }
                    else if(atoi(devtype) == 3)
                    {
                        lanHostEntryTable[i].devicetype = 1;
                    }
                    else if(atoi(devtype) > 3)
                    {
                        lanHostEntryTable[i].devicetype = 0;
                    }
                    
                    break;
                }
            }
        }
    }
    
}

void callFamilyFirewall()
{
    DEBUG_PRINTF("====[%s]==\n",__func__);
    int ret=0;
    int i = 0;
    int j = 0;
    int onlineflag = 1;
    char arpboundflag[16] = {0};
    ret = rtcfgUciGet("firewall.global_sw.family_firewall",arpboundflag);
    if(ret)
    {
        g_arpboundflag = 0;
        DEBUG_PRINTF("===[%s]===cannt get family_firewall===\n",__func__);
        return;
    }
    /* 打开和关闭家庭网络防火墙处理 */
    if((0 == g_arpboundflag) && (1 == atoi(arpboundflag)))
    {
        /* 打开家庭网络防火墙，对所有设备下发规则 */
        for(i = 0; i < lanHostEntryTableNum; i++)
        {
            if(1 == lanHostEntryTable[i].online)
            {
                /*设置规则*/
                add_arpbound(lanHostEntryTable[i].macaddr, lanHostEntryTable[i].ipaddr, 1);
				DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
                g_arpboundflag = 1;
            }
        }
        return;
    }
    
    if((1 == g_arpboundflag) && (0 == atoi(arpboundflag)))
    {
        /* 关闭家庭网络防火墙，对所有设备删除规则 */
        proc_family_firewall_cancel();
		DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
        g_arpboundflag = 0;
        return;
    }

    /* 上线用户配置规则，下线用户取消规则 */
    if(arpboundflag[0])
    {
        if(1 == atoi(arpboundflag))
        {
            for(i = 0; i < lanHostEntryTableNum; i++)
            {
                if(1 == lanHostEntryTable[i].online)
                {
					onlineflag = 1;
                    for(j = 0; j < backupLanHostEntryTableNum; j++)
                    {
                        if(strcmp(lanHostEntryTable[i].macaddr, backupLanHostEntryTable[j].macaddr) == 0)
                        {
                            if(1 == backupLanHostEntryTable[j].online)
                            {
                                /* 非首次上线 */
                                onlineflag = 0;
								DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
                            }
                            break;
                        }
                    }

                    if(onlineflag)
                    {
                        /*设置规则*/
                        add_arpbound(lanHostEntryTable[i].macaddr, lanHostEntryTable[i].ipaddr, 1);
						DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
                    }
                }
                if(0 == lanHostEntryTable[i].online)
                {
                    for(j = 0; j < backupLanHostEntryTableNum; j++)
                    {
                        if(strcmp(lanHostEntryTable[i].macaddr, backupLanHostEntryTable[j].macaddr) == 0)
                        {
                            if(1 == backupLanHostEntryTable[j].online)
                            {
                                /* 设备下线删除规则 */
                                del_arpbound(lanHostEntryTable[i].macaddr, lanHostEntryTable[i].ipaddr, 1);
								DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    g_arpboundflag = atoi(arpboundflag);
    return;
}
/**
 * function: workthread_main
 *          the entrance of work thread
 */
void *workthread_main()
{
    //DEBUG_PRINTF("====hello, i am work thread: %d ====\n", pthread_self());
    while(1)
    {
        DEBUG_PRINTF("==== i am work thread: %d======\n",pthread_self());
        
        lanHostMainLogic();
        
		callAccessType();
        
        setDeviceType();
        
		updateInfo();
        
		callRouterSpeed();
        
        calLanSpeed2();
        
        showLanHostEntryTable();

        pthread_mutex_lock(&mut);
        proc_specialcare();
		callFamilyFirewall();
        syncinformation();
		//showLanHostEntryTable();
        pthread_mutex_unlock(&mut);
        showLanHostEntryTable();
        pthread_mutex_lock(&mut);
        syncinformation();
        pthread_mutex_unlock(&mut);

		//because we waste 1 s  to cal lan  speed and 1 s to cal wan spped , so we no need to sleep more
		//sleep(1);
    }
}



/**
 *   func_name: handleGetInformation
 *              handle get information from dbus
 */
static int handleGetInformation(struct ubus_context *ctx, struct ubus_objec *obj,
        struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    pthread_mutex_lock(&mut);
    DEBUG_PRINTF("=========%s===========\n",__func__);
    blob_buf_init(&b,0);
    void *a = blobmsg_open_table(&b,"LanHostInfo");
    int index=0;
    for(index=0;index<backupLanHostEntryTableNum;index++)
    {
        void *i=blobmsg_open_array(&b,"LanHostEntry");
        if(0 == backupLanHostEntryTable[index].online)
        {
            blobmsg_add_string(&b,"online","offline");
        }
        else
        {
            blobmsg_add_string(&b,"online","online");
        }
        blobmsg_add_string(&b,"ipaddr",backupLanHostEntryTable[index].ipaddr);
        blobmsg_add_string(&b,"macaddr",backupLanHostEntryTable[index].macaddr);
        blobmsg_add_string(&b,"hostname",backupLanHostEntryTable[index].hostname);
        if( 0 == backupLanHostEntryTable[index].connection_type)
        {
            blobmsg_add_string(&b,"connection_type","static");
        }
        else
        {
            blobmsg_add_string(&b,"connection_type","dhcp");
        }

        if(strcmp(backupLanHostEntryTable[index].eth_type, "eth0.1") == 0)
        {
            blobmsg_add_string(&b,"eth_type","ethernet");
        }
        else
        {
            blobmsg_add_string(&b,"eth_type","wifi");
        }

        blobmsg_add_u32(&b,"rx",backupLanHostEntryTable[index].rx);
        blobmsg_add_u32(&b,"tx",backupLanHostEntryTable[index].tx);
        
        blobmsg_close_array(&b,i);
    }
    blobmsg_close_table(&b,a);
    pthread_mutex_unlock(&mut);
    ubus_send_reply(ctx,req,b.head);
    return 0;
}

static const struct ubus_method lanHostManagerMethods[]=
{
    {.name = "getLanInfo", .handler = handleGetInformation },
};

static struct ubus_object_type lanHostManager_object_type =
    UBUS_OBJECT_TYPE("lanHostManager", lanHostManagerMethods);

static struct ubus_object lanHostManager_object = 
{
    .name = "lanHostManager",
    .type = &lanHostManager_object_type,
    .methods = lanHostManagerMethods,
    .n_methods = ARRAY_SIZE(lanHostManagerMethods),
};

/**
 *  func_name: ubus_doing()
 *  connect to ubus and regiest service
 */
int ubus_doing()
{
    int ret=0;

    ctx = ubus_connect(NULL);
    if(!ctx)
    {
        printf("===ERROR!!! === %s=== Failed to connect to ubus \n",__func__);
        return -1;
    }

    ubus_add_uloop(ctx);

    ret= ubus_add_object(ctx,&lanHostManager_object);

    if(ret)
    {
        printf("====ERROR!!! == %s=== failed to add object %s\n",__func__, ubus_strerror(ret));
    }

    return ret;
}

void send_stainfo(VosMsgHeader *rcvdmsg)
{
	
	struct lanHostEntry *p = backupLanHostEntryTable;
	int index = 0; 
	char *msg;
	char ssid1[64] = {0};
    char ssid2[64] = {0};
    char cmd[64] = {0};
	int onlineTime = 0;
	long int curtime = 0;
	cJSON *array = NULL;
    cJSON *obj = NULL;
	cJSON *subJson = NULL;
	FILE *fp = NULL;
	char buffer[64] = {0};
	char tmp1[64] = {0};
	char tmp2[64] = {0};
    struct list_head *tmp=NULL;
	struct fw3_macblock_rule *rule;
    int macBlockFlag = 0;
    int macBlockNum = 0;
    
	VosMsgHeader stHead = {0};
    void *sendmsg;
    
	if (g_msgHandle == NULL)
	{
		return;
	}
    
	//form message	
	obj = cJSON_CreateObject();
    if (!obj)
    {
        DEBUG_PRINTF("Fail to create obj object!\n");
        return;
    }
	subJson = cJSON_CreateObject();
    if (!subJson)
    {
        cJSON_Delete(obj);
        DEBUG_PRINTF("Fail to create subJson object!\n");
        return;
    }
	memset(cmd,0,64);
    snprintf(cmd,64,"wireless.@wifi-iface[0].ssid");
    rtcfgUciGet(cmd,ssid1);
    memset(cmd,0,64);
    snprintf(cmd,64,"wireless.@wifi-iface[1].ssid");
    rtcfgUciGet(cmd,ssid2);
	cJSON_AddItemToObject(subJson,"lfssid",cJSON_CreateString(ssid1));
    cJSON_AddItemToObject(subJson,"hfssid",cJSON_CreateString(ssid2));
	if(NULL != p)
	{
		cJSON_AddItemToObject(subJson,"stalist",array=cJSON_CreateArray());
		for(index = 0; index < backupLanHostEntryTableNum; index++)
		{

            //get macblock list
            macBlockFlag = 0;
        	get_macblocktable();
        	tmp=global_macblock_rules.next;

        	while(tmp!=&global_macblock_rules)
            {
                 
                rule = (struct fw3_macblock_rule *)tmp;
           

        	    if(strcmp(rule->src_mac,p->macaddr)==0)
        	    {
        	    	DEBUG_PRINTF("[%s]====== dev entry mac:%s in macblock list ==== \n",__func__,p->macaddr);
                    macBlockFlag = 1;
                    macBlockNum++;
        			break;
        	    }
          
                tmp=tmp->next;
            }
            

            //free_memory
        	qtec_fw_free_list(&global_macblock_rules);
            if(macBlockFlag)
            {
                p++;
                continue;
            }
            
			cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
			if(0 != p->hostname[0])
			{
				cJSON_AddItemToObject(obj,"staname",cJSON_CreateString(p->hostname));
			}
			else
			{
				cJSON_AddItemToObject(obj,"staname",cJSON_CreateString("unknow"));
			}
			if(p->online)
			{
				fp = fopen("/proc/uptime", "r");
				if(fp)
				{
					memset(buffer, 0, 64);
					memset(tmp1, 0, 64);
					memset(tmp2, 0, 64);
					if(fgets(buffer, 64, fp) != NULL)
					{
						sscanf(buffer, "%s %s", tmp1, tmp2);
						printf("buffer:%s,tmp1:%s,tmp2:%s.\n", buffer, tmp1, tmp2);
						curtime = atoi(tmp1);
					}
					fclose(fp);
					fp = NULL;
				}
				printf("daytime:%ld, p->time:%ld.\n", curtime, p->time);
				onlineTime = curtime - p->time;
				cJSON_AddItemToObject(obj,"stastatus",cJSON_CreateNumber(onlineTime));				
			}
			else
			{
				cJSON_AddItemToObject(obj,"stastatus",cJSON_CreateNumber(0));
			}
            cJSON_AddItemToObject(obj,"ipaddr",cJSON_CreateString(p->ipaddr));
            cJSON_AddItemToObject(obj,"macaddr",cJSON_CreateString(p->macaddr));
            cJSON_AddItemToObject(obj,"devicetype",cJSON_CreateNumber(p->devicetype));
            if(0 == strcmp(p->eth_type, "eth1"))
            {
                cJSON_AddItemToObject(obj,"accesstype",cJSON_CreateString("wired"));
            }
            else if((0 == strcmp(p->eth_type, "ath0")) || (0 == strcmp(p->eth_type, "ath01")))
            {
                cJSON_AddItemToObject(obj,"accesstype",cJSON_CreateString("2.4G"));
            }
            else if((0 == strcmp(p->eth_type, "ath1")) || (0 == strcmp(p->eth_type, "ath11")))
            {
                cJSON_AddItemToObject(obj,"accesstype",cJSON_CreateString("5G"));
            }
            cJSON_AddItemToObject(obj,"tx",cJSON_CreateNumber(p->tx));
            cJSON_AddItemToObject(obj,"rx",cJSON_CreateNumber(p->rx));
            
			p++;
		}
	}
    cJSON_AddItemToObject(subJson,"devnum",cJSON_CreateNumber(backupLanHostEntryTableNum - macBlockNum));
    cJSON_AddItemToObject(subJson,"routertx",cJSON_CreateNumber(g_routertx));
	cJSON_AddItemToObject(subJson,"routerrx",cJSON_CreateNumber(g_routerrx));
	msg = cJSON_Print(subJson);

	stHead.dataLength = util_strlen(msg) + 1;
	stHead.dst = rcvdmsg->src;
	stHead.src = EID_LANHOST;
	stHead.type = VOS_MSG_HOSTLAN_GETINFO;
	stHead.flags_response = 1;
    
    sendmsg = malloc(sizeof(VosMsgHeader) + stHead.dataLength);
    memset(sendmsg, 0, sizeof(sendmsg));
    memcpy(sendmsg, &stHead, sizeof(VosMsgHeader));
    memcpy(sendmsg + sizeof(VosMsgHeader), msg, stHead.dataLength);
	vosMsg_send(g_msgHandle, sendmsg);
	free(msg);
    free(sendmsg);
    cJSON_Delete(subJson);
    
}

void send_routerstatus(VosMsgHeader *rcvdmsg)
{
	int index = 0; 
	char *msg;
	int routertx = 0;
    int routerrx = 0;
	cJSON *subJson = NULL;
	
	struct VosMsgBody stMsg = {0};
	if (g_msgHandle == NULL)
	{
		return;
	}

	//form message	

	subJson = cJSON_CreateObject();

    
    cJSON_AddItemToObject(subJson,"routertx",cJSON_CreateNumber(g_routertx));
    cJSON_AddItemToObject(subJson,"routerrx",cJSON_CreateNumber(g_routerrx));
    cJSON_AddItemToObject(subJson,"htx",cJSON_CreateNumber(g_htx));
    cJSON_AddItemToObject(subJson,"hrx",cJSON_CreateNumber(g_hrx));
    
	msg = cJSON_Print(subJson);
	memcpy(stMsg.buf, msg, strlen(msg));
	stMsg.stHead.dataLength = strlen(msg) + 1;
	stMsg.stHead.dst = rcvdmsg->src;
	stMsg.stHead.src = EID_LANHOST;
	stMsg.stHead.type = VOS_MSG_ROUTER_GETSTATUS;
	stMsg.stHead.flags_response = 1;
	vosMsg_send(g_msgHandle, &stMsg);
	free(msg);
    cJSON_Delete(subJson);
}

void send_devinfotoantiwifi(VosMsgHeader *rcvdmsg)
{
    struct lanHostEntry *p = backupLanHostEntryTable;
	int index = 0; 
	char *msg;
    char cmd[64] = {0};
    cJSON *array = NULL;
    cJSON *obj = NULL;
	cJSON *subJson = NULL;
    struct list_head *tmp=NULL;
	struct fw3_macblock_rule *rule;
    int macBlockFlag = 0;
    
	VosMsgHeader stHead = {0};
    void *sendmsg;
    
	if (g_msgHandle == NULL)
	{
		return;
	}

	//form message	
	obj = cJSON_CreateObject();
	subJson = cJSON_CreateObject();
	if(NULL != p)
	{
		cJSON_AddItemToObject(subJson,"data",array=cJSON_CreateArray());
		for(index = 0; index < backupLanHostEntryTableNum; index++)
		{
            //get macblock list
            macBlockFlag = 0;
        	get_macblocktable();
        	tmp=global_macblock_rules.next;

        	while(tmp!=&global_macblock_rules)
            {
                 
                rule = (struct fw3_macblock_rule *)tmp;
           

        	    if(strcmp(rule->src_mac,p->macaddr)==0)
        	    {
        	    	DEBUG_PRINTF("[%s]====== dev entry mac:%s in macblock list ==== \n",__func__,p->macaddr);
                    macBlockFlag = 1;
        			break;
        	    }
          
                tmp=tmp->next;
            }
            
            //free_memory
        	qtec_fw_free_list(&global_macblock_rules);
            if(macBlockFlag)
            {
                p++;
                continue;
            }
            
            if(p->online)
            {
				if(strcmp(p->eth_type, "eth1") != 0)
				{
					cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
					cJSON_AddItemToObject(obj,"staname",cJSON_CreateString(p->hostname));
			        cJSON_AddItemToObject(obj,"ipaddr",cJSON_CreateString(p->ipaddr));
			        cJSON_AddItemToObject(obj,"macaddr",cJSON_CreateString(p->macaddr));
			        cJSON_AddItemToObject(obj,"devicetype",cJSON_CreateNumber(p->devicetype));
				}
            }
			p++;
		}
	}
	msg = cJSON_Print(subJson);

	stHead.dataLength = strlen(msg) + 1;
	stHead.dst = rcvdmsg->src;
	stHead.src = EID_LANHOST;
	stHead.type = VOS_MSG_ANTIWIFI_DEVINFO_GET;
	stHead.flags_response = 1;
    
	sendmsg = malloc(sizeof(VosMsgHeader) + stHead.dataLength);
    memset(sendmsg, 0, sizeof(sendmsg));
    memcpy(sendmsg, &stHead, sizeof(VosMsgHeader));
    memcpy(sendmsg + sizeof(VosMsgHeader), msg, stHead.dataLength);
	vosMsg_send(g_msgHandle, sendmsg);
    
	free(msg);
    free(sendmsg);
    cJSON_Delete(subJson);
}

#define lanhost_logfile "/tmp/.lanhost"

static void init_log()
{
	FILE *f1;
	if(access(lanhost_logfile,F_OK) !=0)
	{	
		return;
	}
	f1 = open(lanhost_logfile, O_RDWR | O_APPEND);

	if(f1!=NULL)
	{
		dup2(f1,1);
		dup2(f1,2);

		close(f1);
	}
	
}

/**
 *  function: main
 *           the entrance of lanHostManager
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
    int sleepMs = 100;
	struct timeval tm;
    vosLog_init(EID_LANHOST);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
	ret=vosMsg_init(EID_LANHOST, &g_msgHandle);
    DEBUG_PRINTF("********lanHost Start!!!!!*%s*%d*\n", __FUNCTION__, __LINE__);
	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return;
	}

	//pthread
    pthread_mutex_init(&mut,NULL);
	init_log();
    //freopen("/tmp/lanhostoutput.txt","w",stdout);
    DEBUG_PRINTF("====hello, i am listen thread: %d ===\n", pthread_self());
    ret = pthread_create(&workthread,NULL,workthread_main,NULL);

    if(ret!=0)
    {
        printf("====ERROR!!! create workthread error === %s===\n",strerror(ret));
    }

	vosMsg_getEventHandle(g_msgHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;
    tm.tv_sec = sleepMs / MSECS_IN_SEC;
    tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
	while(1)
	{
		//printf("enter while1.\n");
		rfds = readFdsMaster;
		n = select(maxFd+1, &rfds, NULL, NULL, NULL);
		if (n < 0)
		{
			continue;
		}
		
		if (FD_ISSET(commFd, &rfds))
		{
		    DEBUG_PRINTF("%d fd is set\n", commFd);
			ret = vosMsg_receive(g_msgHandle, &msg);

			if (ret != VOS_RET_SUCCESS)
			{
			    DEBUG_PRINTF("lanhost receive msg fail, ret = %d\n", ret);
				continue;
			}
			switch(msg->type)
			{
				case VOS_MSG_HOSTLAN_GETINFO:
				{
				    DEBUG_PRINTF("receive get lan host info msg\n");
                    pthread_mutex_lock(&mut);
					send_stainfo(msg);
                    pthread_mutex_unlock(&mut);
                    DEBUG_PRINTF("receive get lan host info msg end\n");
					break;
				}
				case VOS_MSG_ROUTER_GETSTATUS:
				{
				    DEBUG_PRINTF("receive get router status msg\n");
                    pthread_mutex_lock(&mut);
					send_routerstatus(msg);
                    pthread_mutex_unlock(&mut);
                    DEBUG_PRINTF("receive get router status msg end\n");
					break;
				}
                case VOS_MSG_ANTIWIFI_DEVINFO_GET:
				{
				    pthread_mutex_lock(&mut);
					send_devinfotoantiwifi(msg);
                    pthread_mutex_unlock(&mut);
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

#if 0
    uloop_init();
    ubus_doing();
    uloop_run();
    
    ubus_free(ctx);
    uloop_done();
#endif
}


#if 0
    while (1)
    {
        lanHostMainLogic();
        showLanHostEntryTable();
       // system("echo living > /dev/console");
        sleep(10);
    }
#endif 



