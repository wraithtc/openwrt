#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "librtcfg.h"
#include "lanhost.h"
#include "rtcfg_uci.h"

#define ARP_FILE "/proc/net/arp"
#define LANHOST_MAXLINESIZE 256
#define LAN_IFNAME "br-lan"
#define DHCP_FILE "/tmp/dhcp.leases"


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
        DEBUG_PRINTF("len: %d tmpchar1:%s====\n",strlen(tmpchar1),tmpchar1);
        DEBUG_PRINTF("len: %d tmpchar2:%s====\n",strlen(tmpchar2),tmpchar2);
        DEBUG_PRINTF("len: %d tmpchar3:%s====\n",strlen(tmpchar3),tmpchar3);
        DEBUG_PRINTF("len: %d tmpchar4:%s====\n",strlen(tmpchar4),tmpchar4);
        DEBUG_PRINTF("len: %d tmpchar5:%s====\n",strlen(tmpchar5),tmpchar5);
       
        strcpy(outputArray[i].ipaddr,tmpchar1);
        outputArray[i].HWType=atoi(&(tmpchar2[2]));
        outputArray[i].flags=atoi(&(tmpchar3[2]));
        strcpy(outputArray[i].macaddr,tmpchar4);
        strcpy(outputArray[i].device,tmpchar5); 
        i++;
    }
    
    fclose(fp);
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
        if(strncmp(arpEntryTable[i].device,LAN_IFNAME,strlen(LAN_IFNAME))==0)
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
        if(strncmp(arpEntryTable[i].device,LAN_IFNAME,strlen(LAN_IFNAME))==0)
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
        DEBUG_PRINTF("==tmpchar1: %s====\n",tmpchar1);
        DEBUG_PRINTF("==tmpchar2: %s====\n",tmpchar2);
        DEBUG_PRINTF("==tmpchar3: %s====\n",tmpchar3);
        DEBUG_PRINTF("==tmpchar4: %s====\n",tmpchar4);
        DEBUG_PRINTF("==tmpchar5: %s====\n",tmpchar5);
            
        for(i=0;i<lanHostEntryTableNum;i++)
        {
            if ( strncmp(lanHostEntryTable[i].macaddr,tmpchar2,strlen(tmpchar2))==0 )
            {
                memset(lanHostEntryTable[i].hostname,0,sizeof(lanHostEntryTable[i].hostname));
                strcpy(lanHostEntryTable[i].hostname,tmpchar4);
                lanHostEntryTable[i].connection_type=1;
                break;
            }
        }
        memset(str,0,LANHOST_MAXLINESIZE);
    }

    fclose(fp);
    return 0;
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
    if(arpEntryTable!=NULL)
    {
        free(arpEntryTable);
    }

    lanHostEntryTableNum=0;
    if(lanHostEntryTable !=NULL)
    {
        free(lanHostEntryTable);
    }

    getArpEntryTableNum(&arpEntryTableNum);
    DEBUG_PRINTF("=====arpEntryTableNum: %d ====\n", arpEntryTableNum);

    if(arpEntryTableNum !=0 )
    {
        arpEntryTable=malloc(arpEntryTableNum * sizeof(struct arpEntry));
        memset(arpEntryTable,0,(arpEntryTableNum * sizeof(struct arpEntry)));
    }
    else
    {
        printf("===warning===there are no arp entry===\n");
    }

    getArpEntryTable(arpEntryTable,&arpEntryTableNum);
    int i=0;
    for(i=0;i<arpEntryTableNum;i++)
    {
        DEBUG_PRINTF("arpEntryArray[%d].ipaddr : %s ====\n",i, arpEntryTable[i].ipaddr);
        DEBUG_PRINTF("arpEntryArray[%d].HWType : %d ====\n",i, arpEntryTable[i].HWType);
        DEBUG_PRINTF("arpEntryArray[%d].flags:   %d ====\n",i, arpEntryTable[i].flags);
        DEBUG_PRINTF("arpEntryArray[%d].macaddr: %s ====\n",i, arpEntryTable[i].macaddr);
        DEBUG_PRINTF("arpEntryArray[%d].device:  %s ====\n",i, arpEntryTable[i].device);
    }
#if 0
    int i=0;
    int j=0;
    while(i<arpEntryTableNum)
    {
        if(strncmp(arpEntryTable[i].device,LAN_IFNAME,strlen(LAN_IFNAME))==0)
        {
            i++;
            j++;   
        }
        else
        {
            i++;
        }
    }
#endif 
    getLanHostEntryTableNum(&lanHostEntryTableNum);
    DEBUG_PRINTF("====lanHostEntryTableNum: %d===\n", lanHostEntryTableNum);
    if(lanHostEntryTableNum != 0)
    {
        lanHostEntryTable=malloc(lanHostEntryTableNum * sizeof(struct lanHostEntry));
        memset(lanHostEntryTable,0,(lanHostEntryTableNum * sizeof(struct lanHostEntry)));
    }
    else
    {
        printf("====WARNING=== no lanhostentry====\n");
    }

    getLanHostEntryTable(lanHostEntryTable,&lanHostEntryTableNum);
    
    praseDhcpFile();

    for(i=0;i<lanHostEntryTableNum;i++)
    {
        DEBUG_PRINTF("lanHostEntryTable[%d].online: %d ===\n",i,lanHostEntryTable[i].online);
        DEBUG_PRINTF("lanHostEntryTable[%d].ipaddr: %s ===\n",i,lanHostEntryTable[i].ipaddr);
        DEBUG_PRINTF("lanHostEntryTable[%d].macaddr: %s ===\n",i,lanHostEntryTable[i].macaddr);
        DEBUG_PRINTF("lanHostEntryTable[%d].hostname: %s ===\n",i,lanHostEntryTable[i].hostname);
        DEBUG_PRINTF("lanHostEntryTable[%d].connect_type: %d ===\n",i,lanHostEntryTable[i].connection_type);
    }
    return 0;
}

/**
 * func_name: outputAllLanHostInfo
 *          output all lanhost info
 *
 */
struct lanHostEntry * outputAllLanHostInfo(int *arraynum)
{
    struct lanHostEntry *outputArray=NULL;
    int num=0;
    num = lanHostEntryTableNum;
    *arraynum =num;
    int i=0;
    if( num != 0)
    {
        outputArray=malloc(num * sizeof(struct lanHostEntry));
        memset(outputArray,0,(num*sizeof(struct lanHostEntry)));
        for(i=0;i<num;i++)
        {
            outputArray[i].online=lanHostEntryTable[i].online;
            strcpy(outputArray[i].ipaddr,lanHostEntryTable[i].ipaddr);
            strcpy(outputArray[i].macaddr,lanHostEntryTable[i].macaddr);
            strcpy(outputArray[i].hostname,lanHostEntryTable[i].hostname);
            outputArray[i].connection_type=lanHostEntryTable[i].connection_type;
        }
    }
    else
    {
        printf("===WARNING!  there is no lan host ===\n");
    }
    return outputArray;
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
                outputArray[j].connection_type=lanHostEntryTable[i].connection_type;
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
