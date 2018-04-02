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


void main()
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

