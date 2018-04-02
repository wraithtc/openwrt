#ifndef NETWORKSET_H
#define NETWORKSET_H
#include "stdio.h"

/*
 *static protocol
 *1.ipaddr (ip address) (ipv4) (temporary no need to consider ipv6
 *2.netmask (netmask)
 *3.gateway (ip address)
 *4.dns (list of ip addresses)
 * 
 */
struct wanStaticConfig{
	char ipaddr[24]; //ex:192.168.0.1
	char netmask[24]; //ex:255.255.255.255
	char gateway[24];
	char dns[256]; //ex:192.168.0.1 192.168.0.2 192.168.0.3
};


/*
 * pppoe protocol
 * 1.username(string) - username for PAP/CHAP authentication
 * 2.password(string) - password for RAP/CHAP authentication
 */
struct wanPppoeConfig{
    char username[256];
    char password[256];
};


int wanStaticConfigSet(struct wanStaticConfig *input);
int wanDhcpConfigSet();
int wanPppoeConfigSet(struct wanPppoeConfig *input);
int getWanConnectionType(int *result_type);
int wanPppoeConfigGet(struct wanPppoeConfig *input);
int wanStaticConfigGet(struct wanStaticConfig *input);








#endif
