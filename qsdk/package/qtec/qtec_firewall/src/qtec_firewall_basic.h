#ifndef __QTEC_FIREWALL_BASIC_H
#define __QTEC_FIREWALL_BASIC_H

#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <ctype.h>
#include <string.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>

#include <time.h>

#include <uci.h>

#include <libubox/list.h>
#include <libubox/utils.h>
#include <libubox/blobmsg.h>
#include <librtcfg.h>
#include "fw_struct.h"



//调试用的函数
#undef DEBUG
#define DEBUG 1
#ifdef DEBUG 

#undef DEBUG_PRINTF

#define DEBUG_PRINTF(format,...)   printf(format, ##__VA_ARGS__); fflush(stdout);
#else
#define DEBUG_PRINTF(format,...)
#endif




//定义全局变量
extern struct uci_context *global_uci;
extern struct list_head global_rules;
extern struct list_head global_redirects;
extern int global_rules_num;
extern struct list_head global_arpbound_rules;
extern int global_arpbound_num;
extern struct list_head global_macblock_rules;
extern struct list_head global_childrules;
extern struct list_head global_downlimit_rules;
extern struct list_head global_uplimit_rules;
extern struct list_head global_dmz_rules;
extern struct list_head global_pf_rules;

//函数
struct fw3_rule* alloc_rule();
struct fw3_arpbound_rule* alloc_arpbound_rule();

//basic.c
void printf_rule(struct fw3_rule *input);
void printf_arpbound_rule(struct fw3_arpbound_rule *input);
void restart_fw(int flag);

//arpbound.c
int get_arpboundtable();
int add_arpbound(char *input_mac, char *input_ip, bool input_enabled);
int del_arpbound(char *input_mac, char *input_ip, bool input_enabled);
int mod_arpbound(char *old_input_mac, char *old_input_ip, bool old_input_enabled,char *new_input_mac, char *new_input_ip, bool new_input_enabled);
void proc_family_firewall_cancel();

//macblock.c
int get_macblocktable();
int add_macblock(char *input_name,char *input_mac, bool input_enabled);
int del_macblock(char *input_name,char *input_mac, bool input_enabled);
int mod_macblock(char *input_name,char *old_input_mac,  bool old_input_enabled,char *new_input_mac,  bool new_input_enabled);

//ddos.c
int set_ddos(bool enable);
int get_ddos(bool *enable);

//childrule.c
int get_childruletable();
int add_childrule(char *input_name, char *input_mac,char *input_start_time, char *input_stop_time, char *input_weekdays, bool input_enabled);
int del_childrule(char *input_name,char *input_mac,char *input_start_time, char *input_stop_time, char *input_weekdays, bool input_enabled);
int mod_childrule(char *old_input_name, char *old_input_mac, char *old_input_start_time, char *old_input_stop_time, char *old_input_weekdays, bool old_input_enabled,char *new_input_name, char *new_input_mac,char *new_input_start_time, char* new_input_stop_time, char *new_input_weekdays,  bool new_input_enabled);

//speedlimit.c
int get_downlimit(char *input_mac, int *output_enabled);
int add_downlimit_rule(char *input_mac,char *input_destip, int input_limit, bool input_enabled);
int del_downlimit_rule(char *input_mac,char *input_destip, int input_limit, bool input_enabled);
int mod_downlimit_rule(char *input_mac, char *input_destip, int old_limit,  bool old_enabled, int new_limit,  bool new_enabled);

int get_uplimit(char *input_mac, int *output_enabled);
int add_uplimit_rule(char *input_srcmac,int input_limit, bool input_enabled);
int del_uplimit_rule(char *input_srcmac,int input_limit, bool input_enabled);
int mod_uplimit_rule(char *input_mac, int old_limit,  bool old_enabled, int new_limit,  bool new_enabled);


//dmz.c
int get_dmz_rule(struct fw3_dmz* result_dmz);
int set_dmz_rule(char *input_dest_ip, bool enabled);

//portforward.c
int get_pftable();
int add_pf(char *input_name,char *input_proto,char *input_src_dport, char *input_dest_ip,char *input_dest_port, bool input_enabled);
int del_pf(char *input_name,char *input_proto,char *input_src_dport, char *input_dest_ip,char *input_dest_port, bool input_enabled);
int mod_pf(char *input_name,char *input_proto,char *input_src_dport, char *input_dest_ip,char *input_dest_port, bool input_enabled,char *new_input_name,char *new_input_proto, char *new_input_src_dport, char *new_input_dest_ip,char *new_input_dest_port,bool new_input_enabled);

void qtec_fw_free_list(struct list_head *head);

#define qtec_firewall_DHCP_FILE "/tmp/dhcp.leases"
#define qtec_firewall_max_device_num 128
#define qtec_firewall_hostname_len  64

struct Qtec_firewall_DeviceEntry
{
    char mac[64];
    char hostname[qtec_firewall_hostname_len];
};


void qtec_firewall_pase_dhcp_file(struct Qtec_firewall_DeviceEntry *array, int *num);



#endif 
