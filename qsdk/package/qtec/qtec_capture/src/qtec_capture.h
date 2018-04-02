#ifndef QTEC_CAPTURE_H
#define QTEC_CAPTURE_H


#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include<fcntl.h>
#include <uci.h>

#include <libubox/list.h>
#include <libubox/utils.h>
#include <libubox/blobmsg.h>



#define DEBUG_PRINTF(format,...)   printf(format, ##__VA_ARGS__);
#define DEBUG_PRINTF_RED(format,...) printf("\e[1;31m"format"\e[0m",##__VA_ARGS__)
#define DEBUG_PRINTF_GRE(format,...) printf("\e[1;32m"format"\e[0m",##__VA_ARGS__)

/*Ethernet addresses are 6 bytes*/
#define ETHER_ADDR_LEN 6

//Ethernet header
struct sniff_ethernet {
    u_char ether_dhost[ETHER_ADDR_LEN]; //Destination host address
    u_char ether_shost[ETHER_ADDR_LEN]; //Source host address
    u_short ether_type; //IP*ARP*RARP*etc
};

//IP header
struct sniff_ip{
    u_char ip_vhl;       //version <<4 | header length >> 2
    u_char ip_tos;       //type of service
    u_short ip_len;      //total length
    u_short ip_id;       //identification
    u_short ip_off;      //fragment offset field
#define  IP_RF 0x8000    //reserved fragment flag
#define  IP_DF 0x4000    //dont fragment flag
#define  IP_MF 0x2000    //more fragments flag
#define  IP_OFFMASK 0x1fff  //mask for fragmenting bits
    u_char ip_ttl;       //time to live
    u_char ip_p;         //protocol
    u_short ip_sum;      //checksum
    struct in_addr ip_src,ip_dst;   //source ip and dest ip address(4byte*2)  
};
#define IP_HL(ip)       (((ip)->ip_vhl)&0x0f)
#define IP_V(ip)        (((ip)->ip_vhl) >> 4)


//TCP header
typedef u_int tcp_seq;

struct sniff_tcp{
    u_short th_sport;   //source port
    u_short th_dport;   //destination port
    tcp_seq th_seq;     //sequence number
    tcp_seq th_ack;     //acknowledgement number
    u_char th_offx2;    //data offset, rsvd
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0)>>4)
    u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win;    //window
    u_short th_sum;    //checksum
    u_short th_urp;    //urgent pointer
};

/*ethernet headers are always exactly 14 bytes*/
#define SIZE_ETHERNET 14




//系统类型
enum system_type{
    system_unknown  = 0,
    system_iphone   = 1,
    system_ipad     = 2,
    system_android  = 3,
    system_linux    = 4,
    system_windows  = 5,
    system_mac      = 6,
    system_sunos    = 7,
};


//lan侧设备信息，存储mac地址，设备类型，帮助信息
struct deviceEntry{
    char macaddr[64];
    int device_type;
    char type_message[256];
//    int capture_status;  //获取状态，正在获取（0），或者获取完成（1） 
//    pcap_t *handle;  //用来存储正在获取时的handle
};


#define qtec_capture_logfile "/tmp/.qtec_capture_log"
#define max_device_entrynum 256




int syncArpEntryTable();
//从uci里获取
//void qtec_capture_load_databse()

void work_thread_main(void *arg);


#endif

