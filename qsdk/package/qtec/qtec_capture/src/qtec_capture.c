#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "qtec_capture.h"

extern struct deviceEntry global_device_entry[max_device_entrynum]; //全局已知设备信息存储表
extern int global_device_entry_index;  //全局索引

extern int need_work_tag; //标志位，察觉是否需要开启snipper工作函数

extern int working_tag; //标志位，1代表snipper工作函数正在工作。


static const char* strstri(const char* str, const char* subStr)
{
    int len = strlen(subStr);
    if(len==0)
    {
        return NULL;
    }

    while(*str)
    {
        if(strncasecmp(str,subStr,len)==0)
        {
            return str;
        }

        ++str;
    }

    return NULL;
}

/*
　　操作系统标识
　　FreeBSD
　　X11; FreeBSD (version no.) i386
　　X11; FreeBSD (version no.) AMD64
　　Linux
　　X11; Linux ppc
　　X11; Linux ppc64
　　X11; Linux i686
　　X11; Linux x86_64
　　Mac
　　Macintosh; PPC Mac OS X
　　Macintosh; Intel Mac OS X
　　Solaris
　　X11; SunOS i86pc
　　X11; SunOS sun4u
　　Windows:
　　Windows NT 6.1 对应操作系统 windows 7
　　Windows NT 6.0 对应操作系统 windows vista
　　Windows NT 5.2 对应操作系统 windows 2003
　　Windows NT 5.1 对应操作系统 windows xp
　　Windows NT 5.0 对应操作系统 windows 2000
　　Windows ME
　　Windows 98

*/
static int get_system_type(const char* input_string,int* type)
{
    DEBUG_PRINTF("[%s]====input_string:%s====\n",__func__,input_string);
    //优先选择出手机(iphone 和android)

    *type=0;

    //strstr 是区别大小写的，所以我们要加个不区别大小的strstr    函数 ====> strstri
    if(strstri(input_string,"iphone") != NULL)
    {
        *type= system_iphone;
    }
    else if(strstri(input_string,"iPad")!=NULL)
    {
        *type= system_ipad;
    }
    else if(strstri(input_string,"Android")!=NULL)
    {
        *type= system_android;
    }
    else if(strstri(input_string,"FreeBSD")!=NULL)
    {   
        *type= system_linux;
    }
    else if(strstri(input_string,"Linux")!=NULL)
    {
        *type= system_linux;
    }
    else if(strstri(input_string,"Mac")!=NULL)
    {
        *type= system_mac;
    }
    else if(strstri(input_string,"Windows")!=NULL)
    {
        *type= system_windows;
    }
    else if(strstri(input_string,"SunOS")!=NULL)
    {
        *type= system_sunos;
    }
    else 
    {
        DEBUG_PRINTF_RED("[%s]===cannot get PC system type from string:%s===\n",__func__,input_string);
        return -1;
    }

    DEBUG_PRINTF_GRE("[%s]===get PC system type: %d ===\n",__func__,*type);
    return 0;
    
}

//handle my packet
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    
    DEBUG_PRINTF("===[%s]:====\n",__func__);
    DEBUG_PRINTF("jacked a packet with length of [%d]\n",header->len);

   
    char cmd[256]={0};
    pcap_t *handle=(pcap_t *)args;
    DEBUG_PRINTF("===handle:%x===\n",handle);
    

    struct sniff_ethernet *ethernet; //the ethernet header
    struct sniff_ip *ip; //the ip header
    struct sniff_tcp *tcp; //the tcp header
    char *payload;  //packet payload

    u_int size_ip;
    u_int size_tcp;


    ethernet = (struct sniff_ethernet*)(packet);
    ip=(struct sniff_ip *)(packet + SIZE_ETHERNET);
    size_ip=IP_HL(ip)*4;

    if(size_ip<20){
        DEBUG_PRINTF_RED("*Invalid IP header length：%u bytes\n",size_ip);
        return;
    }

    tcp=(struct sniff_tcp*)(packet+SIZE_ETHERNET+size_ip);
    size_tcp=TH_OFF(tcp)*4;

    if(size_tcp<20){
        DEBUG_PRINTF_RED("*Invalid TCP length:%u bytes\n",size_tcp);
        return;
    }

    payload=(u_char *)(packet+SIZE_ETHERNET+size_ip+size_tcp);
    char srcmac[64]={0};
    char dstmac[64]={0};
    snprintf(srcmac,sizeof(srcmac),"%02x:%02x:%02x:%02x:%02x:%02x",ethernet->ether_shost[0],ethernet->ether_shost[1],ethernet->ether_shost[2],ethernet->ether_shost[3],ethernet->ether_shost[4],ethernet->ether_shost[5]);
    snprintf(dstmac,sizeof(dstmac),"%02x:%02x:%02x:%02x:%02x:%02x",ethernet->ether_dhost[0],ethernet->ether_dhost[1],ethernet->ether_dhost[2],ethernet->ether_dhost[3],ethernet->ether_dhost[4],ethernet->ether_dhost[5]);
    DEBUG_PRINTF_GRE("macsrc:%s   macdst:%s    portsrc:%d   portdst:%d   \n",srcmac,dstmac,ntohs(tcp->th_sport),ntohs(tcp->th_dport));
    int i=0;
    for(i=0;i<global_device_entry_index;i++)
    {
        if(strncasecmp(srcmac,global_device_entry[i].macaddr,sizeof(srcmac))==0)
        {
            DEBUG_PRINTF_GRE("this mac has been deteced, drop it\n===");
            return ;
        }
    }
    //now lets analyze http packet
    char *ptr;
    ptr = strtok(payload,"\r\n");
    while( (ptr !=NULL)&&(strlen(ptr)!=0) )
    {
        DEBUG_PRINTF("ptr=%s\n",ptr);
        //提取User_Agent
        if(strncmp(ptr,"User-Agent:",strlen("User-Agent:"))==0)
        {
            char output[256]={0};
            memcpy(output,ptr+strlen("User-Agent"),256);
            DEBUG_PRINTF_RED("output:%s  \n",output);

            int result=0;
            int type;
            result =get_system_type(output,&type);
            
            if(result == 0)
            {
                rtcfgUciAdd("qtec_capture","lan_entry");
                strncpy(global_device_entry[global_device_entry_index].macaddr,srcmac,sizeof(srcmac));
                global_device_entry[global_device_entry_index].device_type=type;
                strncpy(global_device_entry[global_device_entry_index].type_message,output,sizeof(output));
                global_device_entry_index++;
                    
                memset(cmd,0,sizeof(cmd));
                snprintf(cmd,256,"qtec_capture.@lan_entry[-1].macaddr=%s",srcmac);
                rtcfgUciSet(cmd);
                
                memset(cmd,0,sizeof(cmd));
                snprintf(cmd,256,"qtec_capture.@lan_entry[-1].device_type=%d",type);
                rtcfgUciSet(cmd);

                memset(cmd,0,sizeof(cmd));
                snprintf(cmd,256,"qtec_capture.@lan_entry[-1].type_message=%s",output);
                rtcfgUciSet(cmd);

                rtcfgUciCommit("qtec_capture");
                syncArpEntryTable();
                if(need_work_tag==0)
                {
                    pcap_close(handle);
                    working_tag=0;
                    pthread_exit(0);
                }
            }
        }
        ptr=strtok(NULL,"\r\n");
    }
    
}

void work_thread_main(void *arg)
{
    char cmd[256]={0};    
    pcap_t *handle;   //Session handle
    struct bpf_program fp; //the compiled filter expression
    //char filter_exp[]= "ether src f4:31:c3:10:87:ec and tcp port 80 and (tcp[20:2]=0x504F or tcp[20:2]=0x4745)"; //the filter expression
    char filter_exp[256]="tcp port 80 and (tcp[20:2]=0x504F or tcp[20:2]=0x4745)";
    bpf_u_int32 mask;    // the netmask of our sniffing device
    bpf_u_int32 net;     // the ip of our sniffing device
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    struct pcap_pkthdr header; //the header that pcap gives us
    const u_char *packet; //the actual packet
    
    DEBUG_PRINTF("===qtec_capture start=======\n");
    working_tag=1;


    
#if 0
    dev=pcap_lookupdev(errbuf);

    if(dev==NULL){
        DEBUG_PRINTF_RED("Counldn't find default device:%s\n",errbuf);
        return 2;
    }
#endif
    dev="br-lan";
    //dev="ath1";
 
    
    if(pcap_lookupnet(dev,&net,&mask,errbuf) == -1)
    {
        DEBUG_PRINTF_RED("cannot get netmask for device %s\n",dev);
        net=0;
        mask=0;
    }

    handle=pcap_open_live(dev,262144,1,1000,errbuf);

    if(handle==NULL)
    {
        DEBUG_PRINTF_RED("Couldnt open device %s\n",dev);
        pthread_exit(2);
    }
    
   
    //当接口不支持Ethernet headers  退出
    if(pcap_datalink(handle) != DLT_EN10MB)
    {
        DEBUG_PRINTF_RED("Device %s donesnt provide Ethernet headers - not supported\n",dev);
       // return 2;
    }

    if(pcap_compile(handle, &fp, filter_exp, 0, net)==-1)
    {
        DEBUG_PRINTF_RED("Couldnot parse filter %s:%s \n",filter_exp,pcap_geterr(handle));
        pcap_close(handle);
        pthread_exit(2);
    }
    else
    {
        DEBUG_PRINTF("success parse filter\n");
    }

    if(pcap_setfilter(handle,&fp)==-1)
    {
        DEBUG_PRINTF_RED("Couldnot install filter %s: %s\n",filter_exp,pcap_geterr(handle));
        pcap_close(handle);
        pthread_exit(2);
    }
    else
    {
        DEBUG_PRINTF("success set filter\n");
    }

    
    pcap_loop(handle,-1,got_packet,handle);
    pcap_close(handle);
    return 0;
    
}




