#ifndef __FW_STRUCT_H
#define __FW_STRUCT_H

#include <libubox/list.h>
#include <libubox/utils.h>
#include <libubox/blobmsg.h>

struct fw3_rule
{
	struct list_head list;

	bool enabled; //开关， 默认是打开
	char name[64];  //名字

    char target[64];
    char src_mac[64];
    char src[64];
    char dest[64];
    char src_ip[64];
    char dest_ip[64];

    //childrule
    char start_time[64];
    char stop_time[64];
    char weekdays[64];

    //speedlimit
    int limit;
  
};


//arp 绑定  带*表示 必须的项目
/*
firewall.@rule[5].name=arpbound   *
firewall.@rule[5].src=lan
firewall.@rule[5].dest=wan
firewall.@rule[5].mac_src= !a8:1e:84:5c:fa:72  *
firewall.@rule[5].src_ip=192.168.1.146   *
firewall.@rule[5].target=DROP
firewall.@rule[5].family=ipv4 //暂时不考虑ipv6情形 
firewall.@rule[5].enabled=0  *
*/
struct fw3_arpbound_rule
{
	struct list_head list;

	bool enabled; //开关， 默认是打开
	//char name[64];  //名字

   // char target[64];
    char src_mac[64];
   
    char src_ip[64];
   // int rule_index;
   // int arpbound_rule_index;

};


/*
uci set firewall.@rule[-1].name=macblock
uci set firewall.@rule[-1].src=lan
uci set firewall.@rule[-1].dest=wan
uci set firewall.@rule[-1].src_mac=a8:1e:84:5c:fa:71
uci set firewall.@rule[-1].target=DROP
uci set firewall.@rule[-1].family=ipv4
uci set firewall.@rule[-1].enabled=1
*/
struct fw3_macblock_rule
{
    struct list_head list;

	bool enabled; //开关， 默认是打开
	char name[64];  //名字

   // char target[64];
    char src_mac[64];
};


struct fw3_childrule
{
    struct list_head list;
    char name[64];
    bool enabled;
    char src_mac[64];
    char start_time[64];
    char stop_time[64];
    char weekdays[64];
};

struct fw3_downlimit_rule
{
    struct list_head list;
    bool enabled;
    char dest_ip[64];
    int limit;
};

struct fw3_uplimit_rule
{
    struct list_head list;
    bool enabled;
    char src_mac[64];
    char src_ip[64];
    int limit;
};

struct fw3_redirects
{
	struct list_head list;

	bool enabled; //开关， 默认是打开
	char name[64];  //名字

    char target[64];
  
    char src[64];
    char dest[64];
    char src_ip[64];
    char dest_ip[64];
    char proto[64];
    char src_dport[64];
    char dest_port[64];
    
};

struct fw3_dmz
{
    struct list_head list;

    bool enabled;
    char dest_ip[64];
};

struct fw3_pf
{
    struct list_head list;
    bool enabled;

    char name[64];
    char src[64];
    char dest[64];
    char proto[64];
    char src_dport[64];
    char dest_ip[64];
    char dest_port[64];
};
#endif

