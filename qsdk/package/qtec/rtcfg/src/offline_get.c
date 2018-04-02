#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "offline_get.h"

struct offlineinfo
{
	char ipaddr[20];
	char macaddr[20];
	char name[20];
};

int getofflinelist(char *buf, int len)
{
	memset(buf, 0, len);
	struct offlineinfo devlist[50];
	FILE *fd = fopen ("/proc/net/arp", "r");
	char rdbuf[100];
	char rdbufinfo[6][20];
	int i = 0;
	if (fd != NULL)
	{
		fgets(rdbuf, 100, fd);
		memset(rdbuf, 0, 100);
		while(fgets(rdbuf, 100, fd)!= NULL)
		{
			sscanf(rdbuf, "%s %s %s %s %s %s", rdbufinfo[0], rdbufinfo[1], rdbufinfo[2], rdbufinfo[3], rdbufinfo[4], rdbufinfo[5]);
			if ((strcmp(rdbufinfo[2] , "0x0")==0)&&(strcmp(rdbufinfo[5], "br-lan")==0))
			{
				strcpy(devlist[i].ipaddr, rdbufinfo[0]);
				strcpy(devlist[i].macaddr, rdbufinfo[3]);
				i++;
			}
			memset(rdbuf, 0, 100);
			memset(rdbufinfo, 0, 120);
		}	
	}
	fclose(fd);

	fd = fopen("/tmp/dhcp.leases", "r");
	int j;
	if (fd != NULL)
	{
		while(fgets(rdbuf, 100, fd)!= NULL)
		{			
			sscanf(rdbuf, "%s %s %s %s %s", rdbufinfo[0], rdbufinfo[1], rdbufinfo[2], rdbufinfo[3], rdbufinfo[4]);
			for (j = 0; j < i; j++)
			{
				if ((strcmp(devlist[j].macaddr, rdbufinfo[1]) == 0) &&
				    (strcmp(devlist[j].ipaddr, rdbufinfo[2]) == 0))
				{
					strcpy(devlist[j].name, rdbufinfo[3]);
					break;
				}	
			}
			memset(rdbuf, 0, 100);
			memset(rdbufinfo, 0, 120);
		}
		
	}
	
	fclose(fd);
	char strres[1024] = {0};
	char offinfo[100] = {0};
	for(j = 0; j < i; j++)
	{		
		sprintf(offinfo, "{\"mac\":\"%s\", \"ip\":\"%s\", \"devname\":\"%s\"}", devlist[j].macaddr, devlist[j].ipaddr, devlist[j].name);
		if (j == 0)
			strcat(strres, offinfo);
		else
		{
			strcat(strres, ",");
			strcat(strres, offinfo);
		}		
	}
	sprintf(buf, "{\"offlinelist\":[%s]}", strres);

	return i;
	
}