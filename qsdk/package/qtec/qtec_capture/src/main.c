#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "qtec_capture.h"
#include <libubox/list.h>
#include <libubox/utils.h>
#include <libubox/blobmsg.h>
#include <librtcfg.h>

struct deviceEntry global_device_entry[max_device_entrynum]; //全局已知设备信息存储表
int global_device_entry_index;  //全局索引

int need_work_tag=0; //标志位，察觉是否需要开启snipper工作函数
int working_tag=0; //标志位，1代表snipper工作函数正在工作。

void qtec_capture_load_entry()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    struct uci_package *p=NULL;
    struct uci_section *s;
    struct uci_element *e;
    struct uci_context *global_uci;

    global_uci=uci_alloc_context();
    int index=0;
    if(!global_uci)
    {
        DEBUG_PRINTF("[%s]====out of memory===\n",__func__);
        exit(1);
    }

    if(uci_load(global_uci,"qtec_capture",&p))
    {
        uci_perror(global_uci,NULL);
        DEBUG_PRINTF("[%s]===failed to load /etc/config/qtec_capture",__func__);
        exit(1);
    }

    uci_foreach_element(&p->sections,e)
    {
        s=uci_to_section(e);
        if(strncmp(s->type,"lan_entry",strlen("lan_entry")))
        {
            continue;
        }

        struct uci_element *e1;
        struct uci_option *o1;

        uci_foreach_element(&s->options,e1)
        {
            o1=uci_to_option(e1);

            if(!strncmp(e1->name,"macaddr",strlen("macaddr")))
            {
                strncpy(global_device_entry[index].macaddr,o1->v.string,sizeof(global_device_entry[index].macaddr));
            }
            else if(!strncmp(e1->name,"device_type",strlen("device_type")))
            {
                global_device_entry[index].device_type=atoi(o1->v.string);
            }
            else if(!strncmp(e1->name,"type_message",strlen("type_message")))
            {
                strncpy(global_device_entry[index].type_message,o1->v.string,sizeof(global_device_entry[index].type_message));
            }
        }

        index++;
    }
    global_device_entry_index=index;

    uci_free_context(global_uci);
    
    
    
}



static void init_log()
{
    FILE *f1;
	if(access(qtec_capture_logfile,F_OK) !=0)
	{	
		return;
	}
	f1 = open(qtec_capture_logfile, O_RDWR | O_APPEND);

	if(f1!=NULL)
	{
		dup2(f1,1);
		dup2(f1,2);

		close(f1);
	}
}


#define ARP_FILE "/proc/net/arp"
/**
 *  通过arp表check 在线的设备，如果在线设备还没有抓取到设备类型并且还没开始抓取，则开启新的线程去抓取包               
 */
int syncArpEntryTable()
{
    DEBUG_PRINTF("===[%s]===\n",__func__);
    FILE *fp=NULL;
    if( (fp=fopen(ARP_FILE,"r"))==NULL )
    {
        printf("==ERROR!!!==%s === can't open FILE: "ARP_FILE"====\n",__func__);
        return -1;
    }
    
    char str[1024]={0};
    
    //first line: IP address    HW type     Flags       HW address      Mask        Device
    fgets(str,1024,fp);
    
    //DEBUG_PRINTF("===str: %s=====\n",str);
    char macaddr[64];
    char ifname[64];
    int flags; //0x02 stand for online; 0x04 stand for permannet entry
    memset(str,0,1024);
    int index =0;
    int i=0;
    char tmpchar1[20]={0};
    char tmpchar2[20]={0};
    char tmpchar3[20]={0};
    char tmpchar4[20]={0};
    char tmpchar5[20]={0};
    char *point=NULL;
    while( (fgets(str,1024,fp)) !=NULL ) 
    {
        //DEBUG_PRINTF("=====str: %d  %s==\n",strlen(str),str);
        
        sscanf(str,"%s %s %s %s * %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5);

        //praseArpString(str,tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5);
        //DEBUG_PRINTF("len: %d tmpchar1:%s====\n",strlen(tmpchar1),tmpchar1);
        //DEBUG_PRINTF("len: %d tmpchar2:%s====\n",strlen(tmpchar2),tmpchar2);
        //DEBUG_PRINTF("len: %d tmpchar3:%s====\n",strlen(tmpchar3),tmpchar3);
        //DEBUG_PRINTF("len: %d tmpchar4:%s====\n",strlen(tmpchar4),tmpchar4);
        //DEBUG_PRINTF("len: %d tmpchar5:%s====\n",strlen(tmpchar5),tmpchar5);
        #if 0
        strncpy(outputArray[i].ipaddr,tmpchar1, sizeof(outputArray[i].ipaddr));
        outputArray[i].HWType=atoi(&(tmpchar2[2]));
        outputArray[i].flags=atoi(&(tmpchar3[2]));
        strncpy(outputArray[i].macaddr,tmpchar4, sizeof(outputArray[i].macaddr));
        strncpy(outputArray[i].device,tmpchar5, sizeof(outputArray[i].device)); 
        i++;
        #endif
        memset(macaddr,0,sizeof(macaddr));
        strncpy(macaddr,tmpchar4,sizeof(macaddr));

        memset(ifname,0,sizeof(ifname));
        strncpy(ifname,tmpchar5,sizeof(ifname));

        flags=atoi(&(tmpchar3[2]));
        if((flags &2) &&(strncmp(ifname,"br-lan",strlen("br-lan"))==0) )
        {
            //this is a online device
            for(i=0;i<global_device_entry_index;i++)
            {
                if(strcasecmp(macaddr,global_device_entry[i].macaddr)==0)
                {
                    break;
                }
            }

            if(i==global_device_entry_index)
            {
                DEBUG_PRINTF("[%s] device %s haven't detected system type\n",__func__,macaddr);
                //开启新的线程开始干活
                need_work_tag=1;
                fclose(fp);
                return 0;
                
            }
            
        }
        
    }
    need_work_tag=0;
    fclose(fp);
    return 0;
}



int main(int agrc, char** argv)
{
    init_log();
    qtec_capture_load_entry();
    int i=0;
    for(i=0;i<global_device_entry_index;i++)
    {
        DEBUG_PRINTF("marked device:\n");
        DEBUG_PRINTF("mac:%s   device_type:%d\n",global_device_entry[i].macaddr,global_device_entry[i].device_type);
    }
    while(1)
    {
        syncArpEntryTable();
        if( (1==need_work_tag) && (0==working_tag))
        {
            pthread_t pid;
            pthread_create(&pid,NULL,work_thread_main,NULL);
            pthread_detach(pid);
        }
        //rtcfgUciCommit("qtec_capture");
        sleep(1);
    }
    return 0;
  
}

