#include "basic.h"
#include <fwk.h>
#include <pthread.h>

void proc_offline_get(cJSON *jsonvalue,cJSON *jsonOut)
{
    char buf[1024]= {0};
	cJSON *list;
	getofflinelist(buf, 1024);
	list = cJSON_Parse(buf);
	cJSON_AddItemToObject(jsonOut, "data", list);
	return;	
}


int onlinetag = 0;

void proc_onlinetag_get(cJSON * jsonValue, cJSON * jsonOut)
{
	char buf[1024];
	sprintf(buf, "{\"onlinetag\":\"%d\"}", onlinetag);	
	cJSON_AddItemToObject(jsonOut, "data", cJSON_Parse(buf));
	return;
}

/*struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[2048];
};*/

//void *g_onlinemsgHandle;

static void addmac2list(char list[50][20], char *mac)
{
	int i;
	for (i = 0; i<50; i++)
	{
		if (list[i][0] == 0)
		{
			strcpy(list[i], mac);
			break;
		}
	}
}

static void checkmac2list(char list[50][20], char *mac)
{
	int i;
	int index = -1;
	FILE *dhcpfd;
	char rdbuf[100];
	char dhcpinfo[5][20];
	for (i = 0; i<50; i++)
	{
		if (strcmp(list[i], mac) == 0)
			break;		
	}

	if (i >= 50)
	{
		dhcpfd = fopen("/tmp/dhcp.leases", "r");
		if (dhcpfd != NULL)
		{
			while(fgets(rdbuf, 100, dhcpfd)!=NULL)
			{
				sscanf(rdbuf, "%s %s %s %s %s", dhcpinfo[0], dhcpinfo[1], dhcpinfo[2], dhcpinfo[3], dhcpinfo[4]);
				if (strcmp(dhcpinfo[1], mac)==0)
				{
					DEBUG_PRINTF("===========================================\r\n");
					DEBUG_PRINTF("new dev online: mac:%s ip:%s name:%s\r\n", mac, dhcpinfo[2], dhcpinfo[3]);
					DEBUG_PRINTF("===========================================\r\n");
					break;
				}
			}
		}
	}	
}

static void *pthread_routine(void *arg)
{
	int ret;
	int bf = 0;
	/*vosLog_init(EID_WEBCGI);
	vosLog_setDestination(VOS_LOG_DEST_STDERR);
	vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);
	ret = vosMsg_init(EID_WEBCGI, &g_onlinemsgHandle);
	if (ret != VOS_RET_SUCCESS)
	{
		return;
	}*/

	char rdbuf[100];
	char macaddr[20];
	char maclist[50][20];
	char maclisttmp[50][20];

	memset(maclist, 0, 1000);
	
	while(onlinetag)
	{
		memcpy(maclisttmp, maclist, 1000);
		memset(maclist, 0, 1000);
		system("iwinfo ath0 assoclist >onlineinfo.txt");
		FILE *onlinefd = fopen("onlineinfo.txt", "r");
		if (onlinefd != NULL)
		{
			while (fgets(rdbuf, 100, onlinefd)!= NULL)
			{
				sscanf(rdbuf, "%s", macaddr);
				if (bf == 0)
					addmac2list(maclist, macaddr);
				else
				{
					checkmac2list(maclisttmp, macaddr);
					addmac2list(maclist, macaddr);
				}
				fgets(rdbuf, 100, onlinefd);
				fgets(rdbuf, 100, onlinefd);
				fgets(rdbuf, 100, onlinefd);
			}
			bf = 1;
		}
		sleep(5);
		
	}
}

void proc_online_update()
{
    int ret;
	pthread_t pid;

	DEBUG_PRINTF("proc_online_update\r\n");

	pthread_create(&pid, NULL, pthread_routine, NULL);
	pthread_detach(pid);

    //stMsg.stHead.dataLength = 0;
    //stMsg.stHead.dst = EID_MYWEBSOCKET;
    //stMsg.stHead.src = EID_WEBCGI;
    //stMsg.stHead.type = 0x10009876;
	//stMsg.stHead.flags_event = 1;

	//ret = vosMsg_send(g_msgHandle, &stMsg);	

	return;
}

void proc_onlinetag_set(cJSON *jsonValue,cJSON *jsonOut)
{
	int old = onlinetag;
	onlinetag = cJSON_GetObjectItem(jsonValue, "tag")?cJSON_GetObjectItem(jsonValue, "tag")->valueint:0;
	if ((onlinetag == 1) && (old == 0))
		proc_online_update();
	return;
}

void proc_onlinetag(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_onlinetag_get(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
        proc_onlinetag_set(jsonValue, jsonOut);
    }    
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
    }
}


