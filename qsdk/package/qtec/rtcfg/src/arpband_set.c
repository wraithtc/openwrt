#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "offline_get.h"

void macaddrchange(char *macsrc, char *macdst)
{
	int mac[6];
	sscanf(macsrc, "%x-%x-%x-%x-%x-%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
	sprintf(macdst, "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int setarpband(char *mac, char *ip)
{
	char cmd[200];
	char macaddr[20];
	macaddrchange(mac, macaddr);
	sprintf(cmd, "iptables -I FORWARD -s %s -m mac ! --mac-source %s -j DROP", ip, macaddr);
	system(cmd);
	return 0;
}

int delarpband(char *mac, char *ip)
{
	char cmd[200];
	char macaddr[20];
	macaddrchange(mac, macaddr);
	sprintf(cmd, "iptables -D FORWARD -s %s -m mac ! --mac-source %s -j DROP", ip, macaddr);
	system(cmd);
	return 0;

}
int cleararpband()
{
	system("iptables -F FORWARD");
	system("iptables -I FORWARD -j delegate_forward");
}
int getarpband(char *bufout, int len)
{
	system("iptables -S FORWARD >arpband.txt");
	
	FILE *fd = fopen ("arpband.txt", "r");
	char buf[100];
	char ipaddr[20];
	char ip[20];
	char macaddr[20];
	char mac[20];
	int macl[6];
	int ipl[4];
	char str[2000];
	char ipmac[40];
	int bf = 0;
	memset(str, 0, 2000);
	if (fd != NULL)
	{
		fgets(buf, 100, fd);
		memset(buf, 0, 100);
		while(fgets(buf, 100, fd)!= NULL)
		{
			sscanf(buf, "-A FORWARD -s %s -m mac ! --mac-source %s -j DROP", ipaddr, macaddr);
			if (ipaddr[0] != 0)
			{
				sscanf(ipaddr, "%d.%d.%d.%d/32", &ipl[0],&ipl[1],&ipl[2],&ipl[3]);
				sprintf(ip,"%d.%d.%d.%d", ipl[0],ipl[1],ipl[2],ipl[3]);
				sscanf(macaddr, "%x:%x:%x:%x:%x:%x", &macl[0], &macl[1],&macl[2],&macl[3],&macl[4],&macl[5]);
				sprintf(mac, "%x-%x-%x-%x-%x-%x", macl[0],macl[1],macl[2],macl[3],macl[4],macl[5]);	
				//printf("ipband macaddr= %s , ipaddr = %s\r\n", macaddr, ipaddr);
				//printf("ipband mac= %s , ip = %s\r\n", mac, ip);
				sprintf(ipmac, "{\"ip\":\"%s\", \"mac\":\"%s\"}", ip, mac);
				if (bf == 0)
				{
					strcat(str, ipmac);
					bf = 1;
				}
				else
				{
					strcat(str,",");
					strcat(str, ipmac);
				}
				memset(ipaddr, 0, 20);
				memset(macaddr, 0, 20);
			}
			memset(buf, 0, 100);
		}	
	}
	fclose(fd);
	sprintf(bufout, "{\"arplist\":[%s]}", str);
	return 0;	
}

