#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "basic.h"

#define CR_MAX_PACKAGE_SIZE 80
#define ZB_MAX_PACKAGE_SIZE 56

#if 0
typedef struct coordinator_cmd
{
	char head;//0xAA	
	char cmdid;//0:search 1:add 2:del 3:dev
	short seq;//
	char ieee_addr[8];
	short nw_addr;
	short endpoint;//0x15
	char keyNo[16];
	char encrypt;//0:unencrypt 1:encrypt
	char cmd_type;//1.msg 2.key 3.errinfo
	short len;//payload len
	char payload[ZB_MAX_PACKAGE_SIZE];
}coordinator_cmd_t;
#endif



typedef struct coordinator_cmd
{
	char head;//0xAA	
	char cmdid;//0:search 1:add 2:del 3:dev

    short nw_addr;
	unsigned char ieee_addr[8];
	
    unsigned char device_id[16];
	short endpoint;//0x15

//	char encrypt;//0:unencrypt 1:encrypt
//	char cmd_type;//1.msg 2.key 3.errinfo
    unsigned char seq;
	unsigned char len;//payload len
//	char payload[ZB_MAX_PACKAGE_SIZE];
    unsigned char payload[CR_MAX_PACKAGE_SIZE];
}coordinator_cmd_t;

typedef struct zigbee_cmd
{
    char zb_cmdid;
    char val;
    short zb_seq;
    char secret_key_num[16];
    char encrypt;
    char cmd_type;
    short zb_len;
    char zb_payload[ZB_MAX_PACKAGE_SIZE];
}zigbee_cmd_t;

typedef struct doorlock_cmd
{
	char dl_type;//1.msg 2.get addr	
	char action;//...
	short len;//val len
	char val[0];
}doorlock_cmd_t;

typedef struct device_info2
{
	char id[8];
	char version[8];
	char model[8];
	char type;
	char name[16];
}device_info_t2;

typedef struct device_info
{
    char device_id[16];
    char device_version[8];
    char device_model[8];
    short device_type;
    char device_name[20];
}device_info_t;

typedef struct secret_key_cmd
{
	char sk_type;//0x02
	char keynum;
	short len;
	char val[0];
}secret_key_cmd_t;


typedef struct secret_key_info
{
	char keyNo[16];
	char key[16];
}secret_key_info_t;

typedef struct err_info
{
	char errtype;//1.no key 2.invalid key 3.update key failed
	char erract;
	short len;
	char val[0];
}err_info_t;

struct peerinfo{
	int requestid;
	char lwsreqno[80];
};

static int UARTfd = 1;
static unsigned int ZB_seqno = 1;
static struct peerinfo pinfos[1000];


extern int decodeUARTMsg(coordinator_cmd_t *pMsg);

//0 stand red; 1 stand gre
void DEBUG_PRINTF_ZIGBEECMD(struct coordinator_cmd *msg,int code)
{
    /*
    	char head;//0xAA	
	char cmdid;//0:search 1:add 2:del 3:dev

    short nw_addr;
	unsigned char ieee_addr[8];
	
    unsigned char device_id[16];
	short endpoint;//0x15

//	char encrypt;//0:unencrypt 1:encrypt
//	char cmd_type;//1.msg 2.key 3.errinfo
    unsigned char seq;
	unsigned char len;//payload len
//	char payload[ZB_MAX_PACKAGE_SIZE];
    unsigned char payload[CR_MAX_PACKAGE_SIZE];
    */
    if(code == 0)
    {
        DEBUG_PRINTF_RED("====[%s]============wjj debug==========\n",__func__);
        int i=sizeof(struct coordinator_cmd);
        DEBUG_PRINTF_RED("===[%s]==i:%d===\n ",__func__,i);
        int index=0;
        char tmpchar[200]={0};
        memcpy(tmpchar,msg,i);
        for(index=0;index<i;index++)
        {
            DEBUG_PRINTF_RED("%02x ",tmpchar[index]);
        }
        DEBUG_PRINTF_RED("===[%s]=========== wjj debug end========\n",__func__);
    }
    else
    {
        DEBUG_PRINTF_GRE("====[%s]============wjj debug==========\n",__func__);
        int i=sizeof(struct coordinator_cmd);
        DEBUG_PRINTF_GRE("===[%s]==i:%d===\n ",__func__,i);
        int index=0;
        char tmpchar[200]={0};
        memcpy(tmpchar,msg,i);
        for(index=0;index<i;index++)
        {
            DEBUG_PRINTF_GRE("%02x ",tmpchar[index]);
        }
        DEBUG_PRINTF_GRE("===[%s]=========== wjj debug end========\n",__func__);
    }
    
}
static char *getpeerbyreqid(int requestid)//gerpeerbyreqid
{
	return pinfos[requestid%1000].lwsreqno;
}

static int setpeerbyreqid(int requestid, char *lwsreqno)//setpeerbyreqid
{
    DEBUG_PRINTF_RED("requestid: %d    lwsreqno: %s\n", requestid, lwsreqno);
	pinfos[requestid%1000].requestid = requestid;
	memcpy(pinfos[requestid%1000].lwsreqno, lwsreqno, strlen(lwsreqno)+1);
	return 	requestid%1000;
}

static int delpeerbyreqid(int requestid)//delpeerbyreqid
{
	pinfos[requestid%1000].requestid = 0;
	memset(pinfos[requestid%1000].lwsreqno, 0, 80);
	return 0;
}

static int initZBMsg(coordinator_cmd_t *pMsg)
{
	memset(pMsg, 0, sizeof(coordinator_cmd_t));
	pMsg->head  = 0xAA;
	pMsg->endpoint = 0x08;
	//pMsg->cmd_type = 1;	
}

static int initDevMsg(doorlock_cmd_t *pMsg)
{
	memset(pMsg, 0, sizeof(doorlock_cmd_t));
	pMsg->dl_type = 1;
}

int initGdata()
{
	//daemonize();
	initData();	

    //ç­‰å¾…10s åŽä¸ŠæŠ¥å½“å‰è®¾å¤‡åˆ—è¡¨æ—¥å¿—
    //sleep(10);
    //updatedevlist();
	memset(pinfos, 0, sizeof(struct peerinfo)*1000);
	
}

static int writeUART(coordinator_cmd_t *pMsg)
{
	int len;
	
	if(-1 != UARTfd)
	{
		len = write(UARTfd, pMsg, sizeof(struct coordinator_cmd));		
	}
	
	return len;
}

#if 0
int ZBQKeyPassReq(char *devid, int keynum, char* idlist, char* keylist)//Qkey pass--------------to be continue
{
	struct coordinator_cmd zbMsg;
	secret_key_cmd_t *pkey;
	secret_key_info_t *pkeyinfo;
	int i;
	int len;
	
	initZBMsg(&zbMsg);
	zbMsg.cmdid = 3;
	zbMsg.cmd_type = 2;
	zbMsg.seq = ZB_seqno++;
	zbMsg.len = sizeof(secret_key_cmd_t)+sizeof(secret_key_info_t)*keynum;
	struct simpleDeviceEntry *pDev = NULL;
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	
	zbMsg.nw_addr = pDev->nw_addr;
	memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);

	pkey = zbMsg.payload;
	pkey->sk_type = 0x02;
	pkey->keynum = keynum;
	pkey->len = sizeof(secret_key_info_t)*keynum;
	pkeyinfo = pkey->val;
	for (i = 0; i < keynum; i++)
	{
		memcpy(pkeyinfo[i].keyNo, &idlist[i*16], 16);
		memcpy(pkeyinfo[i].key, &keylist[i*16], 16);
	}
	
	//len = writeUART(&zbMsg);
	return 0;
}
#endif 
int ZBQKeyPassRes()//ignore
{
	
}

//×´Ì¬²éÑ¯
int ZB_DevStatusReq(char *session, char *devid, char* in_data)
{
    DEBUG_PRINTF("[%s]===session :%s   devid: %s  =====\n",__func__,session,devid);
    
	int len = 0;
	int iRet = 0;
	struct coordinator_cmd zbMsg;
	struct zigbee_cmd *pdevmsg;

	initZBMsg(&zbMsg);

	struct simpleDeviceEntry *pDev = NULL;	
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", DEVMANAGER_DEVICE_NOT_FOUND);
        handledevicenotfound(devid);
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", DEVMANAGER_WRONG_CONNECT);
		return 0;
	}

	zbMsg.nw_addr = pDev->nw_addr;
	 //½«16½øÖÆ×Ö·û´®×ª»»³É×Ö·û
    DEBUG_PRINTF("[%s]====pDev->ieee_addr: %s=====\n",__func__,pDev->ieee_addr);
    sscanf(pDev->ieee_addr,"%02x%02x%02x%02x%02x%02x%02x%02x",&(zbMsg.ieee_addr[7]),&(zbMsg.ieee_addr[6]),&(zbMsg.ieee_addr[5]),&(zbMsg.ieee_addr[4]),&(zbMsg.ieee_addr[3]),&(zbMsg.ieee_addr[2]),&(zbMsg.ieee_addr[1]),&(zbMsg.ieee_addr[0]));
    //sscanf(pDev->ieee_addr,"%02x%02x%02x%02x%02x%02x%02x%02x",&(zbMsg.ieee_addr[0]),&(zbMsg.ieee_addr[1]),&(zbMsg.ieee_addr[2]),&(zbMsg.ieee_addr[3]),&(zbMsg.ieee_addr[4]),&(zbMsg.ieee_addr[5]),&(zbMsg.ieee_addr[6]),&(zbMsg.ieee_addr[7]));
	//memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	zbMsg.cmdid = 0x03;
	
	zbMsg.seq = ZB_seqno;
	zbMsg.len = strlen(in_data);

    memcpy(zbMsg.payload,in_data,zbMsg.len);
    setpeerbyreqid(ZB_seqno++, session);
    writeUART(&zbMsg);

    #if 0
	pdevmsg = zbMsg.payload;
	pdevmsg->zb_cmdid= 0x03;
	pdevmsg->val = 0x04;
	//pdevmsg->len = 1;
	//pdevmsg->val[0] = 0x01;
	pdevmsg->len = strlen(usrid)+1;
	memcpy(pdevmsg->val, usrid, pdevmsg->len);
	
	//len = writeUART(&zbMsg);
	setpeerbyreqid(ZB_seqno++, session);
	decodeUARTMsg(&zbMsg);

	pdevmsg->action = 0x10;
	pdevmsg->val[0] = 0;
	pdevmsg->val[1] = 1;
	memcpy(&pdevmsg->val[2], usrid, strlen(usrid)+1);
	decodeUARTMsg(&zbMsg);

	pdevmsg->val[0] = 2;
	pdevmsg->val[1] = 4;
	decodeUARTMsg(&zbMsg);

	return iRet;
    #endif 

    return 0;
}

#if 0
int ZBDevStatusReq(char *session, char *devid)//get lockstatus
{
	int len = 0;
	int iRet = 0;
	coordinator_cmd_t zbMsg;
	doorlock_cmd_t *pdevmsg;

	initZBMsg(&zbMsg);

	struct simpleDeviceEntry *pDev = NULL;
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", 1);
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", 1);
		return 0;
	}

	zbMsg.nw_addr = pDev->nw_addr;
	memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	zbMsg.cmdid = 3;
	zbMsg.seq = ZB_seqno;
	zbMsg.len = 1;
	
	pdevmsg = zbMsg.payload;
	pdevmsg->dl_type = 1;
	pdevmsg->action = 0x39;
	pdevmsg->len = 1;
	pdevmsg->val[0]=1;
	
	//len = writeUART(&zbMsg);
	setpeerbyreqid(ZB_seqno++, session);
	decodeUARTMsg(&zbMsg);

	return iRet;
}

static int ZBDevStatusRes(coordinator_cmd_t *pMsg, char * session)
{
	doorlock_cmd_t *pdevmsg;
	char * time;
	int opratetype;
	char * usrid;
	int status;
	char str[20];
	struct simpleDeviceEntry *pDev = NULL;	
	pDev = GetDevInfoByAddr(global_ManagedDeviceEntryList_head, pMsg->ieee_addr, pMsg->nw_addr);
	if (pDev != NULL)
	{
		pdevmsg = pMsg->payload;
		status = pdevmsg->val[0];
		//opratetype = pdevmsg->val[1];
		//usrid = pdevmsg->val[2];
		sprintf(str, "\\\"status\\\":%d", status);
		dodevres(session, str, "ok", 0);
	}

	return 0;	
}
#endif

#if 0
static int ZBDevlog(coordinator_cmd_t *pMsg)//update log
{
	doorlock_cmd_t *pdevmsg;	
	char timestr[100];
	int opratetype;
	char * usrid;
	int status;
	char str[20];	
	struct simpleDeviceEntry *pDev = NULL;

	pDev = GetDevInfoByAddr(global_ManagedDeviceEntryList_head, pMsg->ieee_addr, pMsg->nw_addr);
	if (pDev != NULL)
	{
		time_t now;
		time(&now);
		struct tm *tm_now = localtime(&now);
		strftime(timestr, 100, "%Y/%m/%d %H:%M:%S",tm_now);
		
		pdevmsg = pMsg->payload;
		status = pdevmsg->val[0];
		opratetype = pdevmsg->val[1];
		usrid = &pdevmsg->val[2];
		updatelog(timestr, opratetype, usrid, status, pDev->deviceid);
	}

	return 0;	
}
#endif 
static int ZB_uplog(coordinator_cmd_t *pMsg)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    char str[2048]={0};
	int len;
    char origndata[2048]={0};
	char data[2048]={0};
    
    int i=0;
    for(i=0;i<(int)pMsg->len;i++)
    {
        DEBUG_PRINTF("[%s]===%02x===\n",__func__,(pMsg->payload)[i]);
    }
    DEBUG_PRINTF("[%s]====len: %d ===\n",__func__, (int)pMsg->len);
    memcpy(origndata,pMsg->device_id,sizeof(pMsg->device_id));
    memcpy(origndata+sizeof(pMsg->device_id),pMsg->payload,(int)pMsg->len);
    base64_encode(origndata, data, ((int)pMsg->len + sizeof(pMsg->device_id)));
    DEBUG_PRINTF("[%s]===data:%s===\n",__func__,data);
	char *format = "{\"method\":\"request\", \"data\":{\"url\":\"/route/routerupload/informationreport\", \"loginfo\":\"%s\"}}";
	sprintf(str, format,data);

	dmwriteback(str);
		
	return 0;
}

//Í¸´«
int ZB_DevAction(char *session, char *devid, char* in_data, int len)
{
    DEBUG_PRINTF("[%s]===session :%s   devid: %s  =====\n",__func__,session,devid);
  
	int iRet = 0;
	struct coordinator_cmd zbMsg;
	struct zigbee_cmd *pdevmsg;

	initZBMsg(&zbMsg);

	struct simpleDeviceEntry *pDev = NULL;	
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", DEVMANAGER_DEVICE_NOT_FOUND);
        handledevicenotfound(devid);
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", DEVMANAGER_WRONG_CONNECT);
		return 0;
	}

	zbMsg.nw_addr = pDev->nw_addr;
    memcpy(zbMsg.device_id,pDev->deviceid,sizeof(zbMsg.device_id));
    
	 //½«16½øÖÆ×Ö·û´®×ª»»³É×Ö·û
    DEBUG_PRINTF("[%s]====pDev->ieee_addr: %s=====\n",__func__,pDev->ieee_addr);
    int tmp_a[8]={0};
    //½«16½øÖÆ×Ö·û´®×ª»»³É×Ö·û
    //DEBUG_PRINTF("[%s]====pDev->ieee_addr: %s=====\n",__func__,pDev->ieee_addr);
    sscanf(pDev->ieee_addr,"%02x%02x%02x%02x%02x%02x%02x%02x",&(tmp_a[0]),&(tmp_a[1]),&(tmp_a[2]),&(tmp_a[3]),&(tmp_a[4]),&(tmp_a[5]),&(tmp_a[6]),&(tmp_a[7]));

    zbMsg.ieee_addr[0]=tmp_a[0];
 
    zbMsg.ieee_addr[1]=tmp_a[1];

    zbMsg.ieee_addr[2]=tmp_a[2];
    
    zbMsg.ieee_addr[3]=tmp_a[3];
   
    zbMsg.ieee_addr[4]=tmp_a[4];
   
    zbMsg.ieee_addr[5]=tmp_a[5];

    zbMsg.ieee_addr[6]=tmp_a[6];

    zbMsg.ieee_addr[7]=tmp_a[7];
   
    int i=0;
    for(i=0;i<8;i++)
    {
        DEBUG_PRINTF("[%s]===%02x===\n",__func__,zbMsg.ieee_addr[i]);
    }
	char tmp_char[17]={0};
	snprintf(tmp_char,sizeof(tmp_char),"%02x%02x%02x%02x%02x%02x%02x%02x",zbMsg.ieee_addr[0],zbMsg.ieee_addr[1],zbMsg.ieee_addr[2],zbMsg.ieee_addr[3],zbMsg.ieee_addr[4],zbMsg.ieee_addr[5],zbMsg.ieee_addr[6],zbMsg.ieee_addr[7]);
    
    DEBUG_PRINTF("[%s]====tmp_char: %s====\n",__func__,tmp_char);
	
	zbMsg.cmdid = 0x05;
	
	zbMsg.seq = ZB_seqno;
	zbMsg.len = len;

    memcpy(zbMsg.payload,in_data,len);
    setpeerbyreqid(ZB_seqno, session);
    ZB_seqno=(++ZB_seqno)%255;
    DEBUG_PRINTF_ZIGBEECMD(&zbMsg,0);
    writeUART(&zbMsg);

    #if 0
	pdevmsg = zbMsg.payload;
	pdevmsg->zb_cmdid= 0x03;
	pdevmsg->val = 0x04;
	//pdevmsg->len = 1;
	//pdevmsg->val[0] = 0x01;
	pdevmsg->len = strlen(usrid)+1;
	memcpy(pdevmsg->val, usrid, pdevmsg->len);
	
	//len = writeUART(&zbMsg);
	setpeerbyreqid(ZB_seqno++, session);
	decodeUARTMsg(&zbMsg);

	pdevmsg->action = 0x10;
	pdevmsg->val[0] = 0;
	pdevmsg->val[1] = 1;
	memcpy(&pdevmsg->val[2], usrid, strlen(usrid)+1);
	decodeUARTMsg(&zbMsg);

	pdevmsg->val[0] = 2;
	pdevmsg->val[1] = 4;
	decodeUARTMsg(&zbMsg);

	return iRet;
    #endif 

    return 0;
}

//Í¸´«
int ZB_DevUnlockReq(char *session, char *devid, char* in_data)//door open
{
    DEBUG_PRINTF("[%s]===session :%s   devid: %s  =====\n",__func__,session,devid);
    
	int len = 0;
	int iRet = 0;
	struct coordinator_cmd zbMsg;
	struct zigbee_cmd *pdevmsg;

	initZBMsg(&zbMsg);

	struct simpleDeviceEntry *pDev = NULL;	
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", DEVMANAGER_DEVICE_NOT_FOUND);
        handledevicenotfound(devid);
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", DEVMANAGER_WRONG_CONNECT);
		return 0;
	}

	zbMsg.nw_addr = pDev->nw_addr;
    
	 //½«16½øÖÆ×Ö·û´®×ª»»³É×Ö·û
    DEBUG_PRINTF("[%s]====pDev->ieee_addr: %s=====\n",__func__,pDev->ieee_addr);
    sscanf(pDev->ieee_addr,"%02x%02x%02x%02x%02x%02x%02x%02x",&(zbMsg.ieee_addr[7]),&(zbMsg.ieee_addr[6]),&(zbMsg.ieee_addr[5]),&(zbMsg.ieee_addr[4]),&(zbMsg.ieee_addr[3]),&(zbMsg.ieee_addr[2]),&(zbMsg.ieee_addr[1]),&(zbMsg.ieee_addr[0]));
    //sscanf(pDev->ieee_addr,"%02x%02x%02x%02x%02x%02x%02x%02x",&(zbMsg.ieee_addr[0]),&(zbMsg.ieee_addr[1]),&(zbMsg.ieee_addr[2]),&(zbMsg.ieee_addr[3]),&(zbMsg.ieee_addr[4]),&(zbMsg.ieee_addr[5]),&(zbMsg.ieee_addr[6]),&(zbMsg.ieee_addr[7]));
	//memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	
	zbMsg.cmdid = 0x03;
	
	zbMsg.seq = ZB_seqno;
	zbMsg.len = strlen(in_data);

    memcpy(zbMsg.payload,in_data,zbMsg.len);
    setpeerbyreqid(ZB_seqno++, session);
    writeUART(&zbMsg);

    #if 0
	pdevmsg = zbMsg.payload;
	pdevmsg->zb_cmdid= 0x03;
	pdevmsg->val = 0x04;
	//pdevmsg->len = 1;
	//pdevmsg->val[0] = 0x01;
	pdevmsg->len = strlen(usrid)+1;
	memcpy(pdevmsg->val, usrid, pdevmsg->len);
	
	//len = writeUART(&zbMsg);
	setpeerbyreqid(ZB_seqno++, session);
	decodeUARTMsg(&zbMsg);

	pdevmsg->action = 0x10;
	pdevmsg->val[0] = 0;
	pdevmsg->val[1] = 1;
	memcpy(&pdevmsg->val[2], usrid, strlen(usrid)+1);
	decodeUARTMsg(&zbMsg);

	pdevmsg->val[0] = 2;
	pdevmsg->val[1] = 4;
	decodeUARTMsg(&zbMsg);

	return iRet;
    #endif 

    return 0;
}
 
#if 0
int ZBDevUnlockReq(char *session, char *devid, char *usrid)//door open
{
	int len = 0;
	int iRet = 0;
	coordinator_cmd_t zbMsg;
	doorlock_cmd_t *pdevmsg;

	initZBMsg(&zbMsg);

	struct simpleDeviceEntry *pDev = NULL;	
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", 1);
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", 1);
		return 0;
	}

	zbMsg.nw_addr = pDev->nw_addr;
	memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	zbMsg.cmdid = 3;
	zbMsg.cmd_type = 1;
	zbMsg.seq = ZB_seqno;
	zbMsg.len = 1;
	
	pdevmsg = zbMsg.payload;
	pdevmsg->dl_type = 1;
	pdevmsg->action = 0x32;
	//pdevmsg->len = 1;
	//pdevmsg->val[0] = 0x01;
	pdevmsg->len = strlen(usrid)+1;
	memcpy(pdevmsg->val, usrid, pdevmsg->len);
	
	//len = writeUART(&zbMsg);
	setpeerbyreqid(ZB_seqno++, session);
	decodeUARTMsg(&zbMsg);

	pdevmsg->action = 0x10;
	pdevmsg->val[0] = 0;
	pdevmsg->val[1] = 1;
	memcpy(&pdevmsg->val[2], usrid, strlen(usrid)+1);
	decodeUARTMsg(&zbMsg);

	pdevmsg->val[0] = 2;
	pdevmsg->val[1] = 4;
	decodeUARTMsg(&zbMsg);

	return iRet;
}

static int ZBDevUnlockRes(coordinator_cmd_t *pMsg, char *session)
{
	struct simpleDeviceEntry *pDev = NULL;	
	pDev = GetDevInfoByAddr(global_ManagedDeviceEntryList_head, pMsg->ieee_addr, pMsg->nw_addr);
	if (pDev != NULL)
	{		
		dodevres(session, "", "ok", 0);	
	}
	else
		dodevres(session, "", "unlock err", 1);
	return 0;
}
#endif
#if 0
int ZBDevSearchReq(char *session)//dev search
{
	int len = 0;
	coordinator_cmd_t zbMsg;	
	
	initZBMsg(&zbMsg);	

	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", 1);
		return 0;
	}

	//zbMsg.seq = ZB_seqno++;
	
	
	//len = write(UARTfd, &zbMsg, sizeof(zbMsg));
	ClearDevSearchList();
	//dodevres(session, "", "ok", 0);

	doorlock_cmd_t *plock = zbMsg.payload;
	device_info_t *pdevinfo = plock->val;
	zbMsg.cmdid = 3;
	zbMsg.len = 3;
	
	plock->dl_type = 1;
	plock->action = 0x38;
		
	zbMsg.nw_addr = 1;
	memcpy(zbMsg.ieee_addr, "addr1", 6);
	memcpy(pdevinfo->id, "dev1",8);
	memcpy(pdevinfo->model, "mod1",8);
	memcpy(pdevinfo->name, "name1",16);
	memcpy(pdevinfo->version, "ver1",8);
	pdevinfo->type = 1;
	decodeUARTMsg(&zbMsg);
	zbMsg.nw_addr = 2;
	memcpy(zbMsg.ieee_addr, "addr2", 6);
	memcpy(pdevinfo->id, "dev2",8);
	memcpy(pdevinfo->model, "mod2",8);
	memcpy(pdevinfo->name, "name2",16);
	memcpy(pdevinfo->version, "ver2",8);
	pdevinfo->type = 2;
	decodeUARTMsg(&zbMsg);
	zbMsg.nw_addr = 3;
	memcpy(zbMsg.ieee_addr, "addr3", 6);
	memcpy(pdevinfo->id, "dev3",8);
	memcpy(pdevinfo->model, "mod3",8);
	memcpy(pdevinfo->name, "name3",16);
	memcpy(pdevinfo->version, "ver3",8);
	pdevinfo->type = 3;
	decodeUARTMsg(&zbMsg);
	return 0;
}
#endif 
#if 0
static int ZBDevSearchRes(coordinator_cmd_t *pMsg)
{
	doorlock_cmd_t *plock = pMsg->payload;
	device_info_t *pdevinfo = plock->val;
	struct simpleDeviceEntry devinfo;
	
	memset(&devinfo, 0, sizeof(struct simpleDeviceEntry));
	memcpy(devinfo.deviceid, pdevinfo->id, 8);
	memcpy(devinfo.name, pdevinfo->name, 16);
	memcpy(devinfo.version, pdevinfo->version, 8);
	memcpy(devinfo.model, pdevinfo->model, 8);
	devinfo.type = pdevinfo->type;
	memcpy(devinfo.ieee_addr, pMsg->ieee_addr, 8);
	devinfo.nw_addr = pMsg->nw_addr;

	AddDeviceInfo2SearchedDeviceList(&devinfo);
	return 0;
}
#endif

static int ZB_addEntry2SearchedList(coordinator_cmd_t *pMsg)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    char tmp_char[17]={0};
	zigbee_cmd_t *pzigbee_cmd = pMsg->payload;
	struct device_info *pdevinfo = pzigbee_cmd->zb_payload;
	struct simpleDeviceEntry devinfo;
	
	memset(&devinfo, 0, sizeof(struct simpleDeviceEntry));
	memcpy(devinfo.deviceid, pdevinfo->device_id, 16);
	memcpy(devinfo.name, pdevinfo->device_name, 32);
	memcpy(devinfo.version, pdevinfo->device_version, 8);
	memcpy(devinfo.model, pdevinfo->device_model, 8);
	devinfo.type = pdevinfo->device_type;
    snprintf(tmp_char,sizeof(tmp_char),"%02x%02x%02x%02x%02x%02x%02x%02x",pMsg->ieee_addr[0],pMsg->ieee_addr[1],pMsg->ieee_addr[2],pMsg->ieee_addr[3],pMsg->ieee_addr[4],pMsg->ieee_addr[5],pMsg->ieee_addr[6],pMsg->ieee_addr[7]);
    DEBUG_PRINTF("[%s]====tmp_char: %s====\n",__func__,tmp_char);
    //memcpy(devinfo.ieee_addr, pMsg->ieee_addr, 8);
    strncpy(devinfo.ieee_addr,tmp_char,sizeof(devinfo.ieee_addr));
	devinfo.nw_addr = pMsg->nw_addr;

	
    /*
    struct simpleDeviceEntry * pDev = NULL;
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev != NULL)
	{
		DEBUG_PRINTF("[%s]==== managedDevice entryList have it===\n",__func__);
		updatedevlist();  
		//return 0;   
	}
	*/

    struct simpleDeviceEntry * pDev=NULL;
    pDev = GetDevInfoById(global_searchedDeviceEntryList_head,devinfo.deviceid);
    if(pDev == NULL)
    {
        AddDeviceInfo2SearchedDeviceList(&devinfo);
    }
    else
    {
        memcpy(pDev->name,devinfo.name,32);
        memcpy(pDev->version,devinfo.version,8);
        memcpy(pDev->model,devinfo.model,8);
        pDev->type=devinfo.type;
        memcpy(pDev->ieee_addr,devinfo.ieee_addr,sizeof(devinfo.ieee_addr));
        pDev->nw_addr=devinfo.nw_addr;
    }
    
	return 0;
}

int ZBDevAddReq(char* session, char *devid)//add dev
{
    DEBUG_PRINTF("[%s]=====session:%s====devid:%s====\n",__func__,session,devid);
	int len = 0;
	struct coordinator_cmd zbMsg;
	initZBMsg(&zbMsg);
	
	struct simpleDeviceEntry * pDev = NULL;
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev != NULL)
	{
		DEBUG_PRINTF("[%s]==== managedDevice entryList have it, so remove it firstly===\n",__func__);
        DelDeviceInfo2ManagedDeviceList(devid);
		updatedevlist();  
		//return 0;   
	}
    
    /*
	pDev = GetDevInfoById(global_searchedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", 1);
		updatedevlist();
		return 0;
	}
	*/


    
    
    ClearDevSearchList();

    
    zbMsg.cmdid=0x00;
    zbMsg.len=0;
    
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", DEVMANAGER_WRONG_CONNECT);
		return 0;
	}
    DEBUG_PRINTF("[%s]====open the zigbee network===\n",__func__);
    DEBUG_PRINTF_ZIGBEECMD(&zbMsg,0);
    len=write(UARTfd,&zbMsg,sizeof(zbMsg));
    
    sleep(30);

    DEBUG_PRINTF("[%s]====stop search====\n",__func__);
  
    pDev = GetDevInfoById(global_searchedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		DEBUG_PRINTF("[%s]===not find match device====\n",__func__);
		//updatedevlist();
		
	}
    else
    {
        //Èç¹ûËÑÑ°µ½£¬ÔòÌí¼Óµ½¹ÜÀíÁÐ±í
        AddDeviceInfo2ManagedDeviceList(pDev);
        DEBUG_PRINTF("[%s]====already add entry into managed device table===\n",__func__);
       // updatedevlist();
    }
    DEBUG_PRINTF("[%s]===wjianjia === goto remove more dev===\n",__func__);
    //½«¶àÓàµÄÉè±¸É¾³ý³ö×éÍø
    struct simpleDeviceEntry *tmp=global_searchedDeviceEntryList_head;
    while(tmp!=NULL)
    {
        pDev=GetDevInfoById(global_ManagedDeviceEntryList_head, tmp->deviceid);
        if(pDev == NULL)
        {
            //Èç¹û²»ÔÚ¹ÜÀíÁÐ±íÀï£¬Ôò½â°ó¸ÃÉè±¸
            ZB_DevDelReq(tmp->deviceid, 0);
            
        }
        tmp=tmp->root.next;
    }

    updatedevlist();
    
#if 0
	zbMsg.nw_addr = pDev->nw_addr;
	memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	zbMsg.cmdid = 1;
	zbMsg.seq = ZB_seqno;
	
	//len = write(UARTfd, &zbMsg, sizeof(zbMsg));
	setpeerbyreqid(ZB_seqno++, session);
#endif
	//decodeUARTMsg(&zbMsg);
		
	return 0;
}

static int ZBDevAddRes(coordinator_cmd_t *pMsg, char *session)
{	
	struct simpleDeviceEntry * pDev = NULL;
	pDev = GetDevInfoByAddr(global_searchedDeviceEntryList_head, pMsg->ieee_addr, pMsg->nw_addr);
	if (pDev != NULL)
	{
		AddDeviceInfo2ManagedDeviceList(pDev);
		dodevres(session, "", "ok", 0);
		updatedevlist();
	}
	else
		dodevres(session, "", "dev add err", DEVMANAGER_INTER_WRONG);
	return 0;
}

#if 0
int ZBDevDelReq(char *session, char *devid)//del dev
{
	int len = 0;
	coordinator_cmd_t zbMsg;
	initZBMsg(&zbMsg);

	struct simpleDeviceEntry * pDev = NULL;
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", 1);
		updatedevlist();
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", 1);
		return 0;
	}

	zbMsg.nw_addr = pDev->nw_addr;
	memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	zbMsg.cmdid = 2;
	zbMsg.seq = ZB_seqno;
	
	//len = writeUART(&zbMsg, zbmsg, sizeof(zbMsg));
	setpeerbyreqid(ZB_seqno++, session);
	decodeUARTMsg(&zbMsg);

	return 0;
}
#endif

// Èç¹ûÉè±¸id´¦ÓÚÉè±¸µÄ¹ÜÀíÁÐ±íÀï£¬Ôò·µ»Ø1£¬ ·ñÔò·µ»Ø0
int ZB_DevCheck(char *devid)
{
    DEBUG_PRINTF("[%s]========devid: %s ==========\n",__func__,devid);
    struct simpleDeviceEntry * pDev = NULL;
    pDev = GetDevInfoById(global_ManagedDeviceEntryList_head,devid);
    if(pDev == NULL )
        return 0;
    else
        return 1;
}
int qtec_test()
{
    DEBUG_PRINTF("[%s]=========\n",__func__);
    
    int len = 0;
	struct coordinator_cmd zbMsg;
	initZBMsg(&zbMsg);

	zbMsg.cmdid = 2;
    
	zbMsg.seq = 0x00;

    DEBUG_PRINTF_ZIGBEECMD(&zbMsg,0);
	len = writeUART(&zbMsg);
    DEBUG_PRINTF("[%s]===len: %d====\n",__func__,len);
    return 0; 
}


//Èç¹ûopt ÊÇ0,Ôò´ÓÉè±¸ËÑÑ°ÁÐ±íÀïÕÒ³öµØÖ·£¬¼ÓÒÔÉ¾³ý
//Èç¹ûopt ÊÇ1£¬Ôò´ÓÉè±¸¹ÜÀíÁÐ±íÀïÕÒ³öµØÖ·£¬¼ÓÒÔÉ¾³ý
int ZB_DevDelReq(char *devid,int opt)
{
    DEBUG_PRINTF("[%s]=====devid:%s    opt is %d ====\n",__func__,devid,opt);
    
    int len = 0;
	struct coordinator_cmd zbMsg;
	initZBMsg(&zbMsg);

	struct simpleDeviceEntry * pDev = NULL;
    if(opt == 1)
    {
	    pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
    }
    else if(opt == 0)
    {
        pDev = GetDevInfoById(global_searchedDeviceEntryList_head,devid);
    }
    
	if (pDev == NULL)
	{
		DEBUG_PRINTF("[%s]====cound not find match device===\n",__func__);
		updatedevlist();
		return 0;
	}
	if (-1 == UARTfd)
	{
		DEBUG_PRINTF("[%s]====cound not write to UARTfd====\n",__func__);
		return 0;
	}
    DEBUG_PRINTF("[%s]====pDev->deviceid: %s  size:%d===\n",__func__,pDev->deviceid,sizeof(zbMsg.device_id));
    memcpy(zbMsg.device_id,pDev->deviceid,sizeof(zbMsg.device_id));
    DEBUG_PRINTF("[%s]===zbMsg.device_id: %s === \n",__func__, zbMsg.device_id);
    
    DEBUG_PRINTF("==[%s]====pDev->nw_addr: %x===\n",__func__,pDev->nw_addr);
	zbMsg.nw_addr = pDev->nw_addr;
    #if 1
    int tmp_a[8]={0};
    //½«16½øÖÆ×Ö·û´®×ª»»³É×Ö·û
    DEBUG_PRINTF("[%s]====pDev->ieee_addr: %s=====\n",__func__,pDev->ieee_addr);
    sscanf(pDev->ieee_addr,"%02x%02x%02x%02x%02x%02x%02x%02x",&(tmp_a[0]),&(tmp_a[1]),&(tmp_a[2]),&(tmp_a[3]),&(tmp_a[4]),&(tmp_a[5]),&(tmp_a[6]),&(tmp_a[7]));

   
    
    
    
    
    
   DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
    zbMsg.ieee_addr[0]=tmp_a[0];
    DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
     zbMsg.ieee_addr[1]=tmp_a[1];
     DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
     zbMsg.ieee_addr[2]=tmp_a[2];
     DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
     zbMsg.ieee_addr[3]=tmp_a[3];
     DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
     zbMsg.ieee_addr[4]=tmp_a[4];
     DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
     zbMsg.ieee_addr[5]=tmp_a[5];
     DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
     zbMsg.ieee_addr[6]=tmp_a[6];
     DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
      zbMsg.ieee_addr[7]=tmp_a[7];
      DEBUG_PRINTF("[%s]= %d==zbMsg.head: %x====\n",__func__,__LINE__,zbMsg.head);
    int i=0;
    for(i=0;i<8;i++)
    {
        DEBUG_PRINTF("[%s]===%02x===\n",__func__,zbMsg.ieee_addr[i]);
    }
	char tmp_char[17]={0};
	snprintf(tmp_char,sizeof(tmp_char),"%02x%02x%02x%02x%02x%02x%02x%02x",zbMsg.ieee_addr[0],zbMsg.ieee_addr[1],zbMsg.ieee_addr[2],zbMsg.ieee_addr[3],zbMsg.ieee_addr[4],zbMsg.ieee_addr[5],zbMsg.ieee_addr[6],zbMsg.ieee_addr[7]);
    
    DEBUG_PRINTF("[%s]====tmp_char: %s====\n",__func__,tmp_char);
    #endif
	zbMsg.cmdid = 2;
    
	zbMsg.seq = 0x00;
    DEBUG_PRINTF("[%s]===zbMsg.head: %x====\n",__func__,zbMsg.head);
    DEBUG_PRINTF("[%s]=wjianjia==zbMsg.device_id: %s === \n",__func__, zbMsg.device_id);
    DEBUG_PRINTF_ZIGBEECMD(&zbMsg,0);
	len = writeUART(&zbMsg);
    DEBUG_PRINTF("[%s]===len: %d====\n",__func__,len);
    return 0;
}


#if 0
static int ZBDevDelRes(coordinator_cmd_t *pMsg, char *session)
{		
	struct simpleDeviceEntry * pDev = NULL;
	pDev = GetDevInfoByAddr(global_ManagedDeviceEntryList_head, pMsg->ieee_addr, pMsg->nw_addr);
	if (pDev != NULL)
	{		
		DelDeviceInfo2ManagedDeviceList(pDev->deviceid);
		dodevres(session, "", "ok", 0);
		updatedevlist();
	}
	else
		dodevres(session, "", "dev del err", 1);
	return 0;
}
#endif 

//´Ó¹ÜÀíÁÐ±íÀïÉ¾³ýÉè±¸
static int ZB_DelFromManagedDeviceList(coordinator_cmd_t * pMsg)
{
    DEBUG_PRINTF("[%s]======\n",__func__);
	struct simpleDeviceEntry * pDev = NULL;
	//pDev = GetDevInfoByAddr(global_ManagedDeviceEntryList_head, pMsg->ieee_addr, pMsg->nw_addr);
	char device_id[64]={0};
    memcpy(device_id,pMsg->device_id,sizeof(pMsg->device_id));
    pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, device_id);
	if (pDev != NULL)
	{		
		DelDeviceInfo2ManagedDeviceList(pDev->deviceid);
		//dodevres(session, "", "ok", 0);
	}
	else
	{
        DEBUG_PRINTF("[%s]====ZB del entry from manageddevicelist fail ===\n",__func__);
	}
    updatedevlist();
	return 0;
}

int ZBAddFPReq(char *session, char *usrid, char *devid)//add fingerprint
{
	int len = 0;
	coordinator_cmd_t zbMsg;
	doorlock_cmd_t *plock;
	char fpid[10];
	static int i = 1;
	initZBMsg(&zbMsg);

	struct simpleDeviceEntry * pDev = NULL;
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", DEVMANAGER_DEVICE_NOT_FOUND);
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", DEVMANAGER_WRONG_CONNECT);
		return 0;
	}
	
	zbMsg.nw_addr = pDev->nw_addr;
	memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	zbMsg.cmdid = 3;
	zbMsg.seq = ZB_seqno;
	zbMsg.len = sizeof(doorlock_cmd_t);
	plock = zbMsg.payload;
	initDevMsg(plock);
	plock->action = 0x36;
	plock->len = 30;//
	memcpy(plock->val, devid, strlen(devid)+1);//pass devid
	memcpy(&plock->val[10], usrid, strlen(usrid)+1);//pass usrid
	sprintf(fpid, "fp%d", i++);
	memcpy(&plock->val[50], fpid, strlen(fpid)+1);
	
	//len = write(UARTfd, &zbMsg, sizeof(zbMsg));
	setpeerbyreqid(ZB_seqno++, session);
	decodeUARTMsg(&zbMsg);
	
	return 0;
}

static int ZBAddFPRes(coordinator_cmd_t *pMsg, char *session)
{
	char str[50];
	char devid[10];
	char usrid[40];
	char fpid[10];	
	doorlock_cmd_t *plock = pMsg->payload;
	
	if (plock->len > 0)
	{
		memcpy(devid, plock->val, 10);//get devid
		memcpy(usrid, &plock->val[10], 40);//get usrid
		memcpy(fpid,  &plock->val[50], 10);//get fpid		
	
		sprintf(str, "\\\"fpid\\\" : \\\"%s\\\"", fpid);		
		AddFingerPrint(devid, usrid, fpid);
		dodevres(session, str, "ok", 0);
		printf("usrid:%s add fingerprint:%s in devid:%s\r\n", usrid, fpid, devid);
	}
}

int ZBDelFPReq(char *session, char *devid, char *usrid, char *fpid)//del fingerprint
{
	int len = 0;
	coordinator_cmd_t zbMsg;
	doorlock_cmd_t *plock;

	initZBMsg(&zbMsg);

	struct simpleDeviceEntry * pDev = NULL;
	pDev = GetDevInfoById(global_ManagedDeviceEntryList_head, devid);
	if (pDev == NULL)
	{
		dodevres(session, "", "dev not found", 1);
		return 0;
	}
	if (-1 == UARTfd)
	{
		dodevres(session, "", "cant connect to dev", 1);
		return 0;
	}	
	
	zbMsg.nw_addr = pDev->nw_addr;
	memcpy(zbMsg.ieee_addr, pDev->ieee_addr, 8);
	zbMsg.cmdid = 3;
	zbMsg.seq = ZB_seqno;
	zbMsg.len = sizeof(doorlock_cmd_t);
	plock = zbMsg.payload;
	initDevMsg(plock);
	plock->action = 0x33;
	plock->len = 30;
	memcpy(plock->val, devid, strlen(devid)+1);//pass devid
	memcpy(&plock->val[10], usrid, strlen(usrid)+1);//pass usrid
	memcpy(&plock->val[50], fpid, strlen(fpid)+1);//pass fpid
	
	//len = write(UARTfd, &zbMsg, sizeof(zbMsg));
	setpeerbyreqid(ZB_seqno++, session);		
	decodeUARTMsg(&zbMsg);
	return 0;
}

static int ZBDelFPRes(coordinator_cmd_t *pMsg, char *session)
{
	char fpid[10];
	char usrid[40];
	char devid[10];
	
	doorlock_cmd_t *plock = pMsg->payload;
	if (plock->len > 0)
	{
		memcpy(devid, plock->val, 10);//get devid
		memcpy(usrid, &plock->val[10], 40);//get usrid
		memcpy(fpid,  &plock->val[50], 10);//get fpid

		DelFingerPrint(devid, usrid, fpid);
		dodevres(session, "", "ok", 0);
		printf("usrid:%s del fingerprint:%s in devid:%s\r\n", usrid, fpid, devid);
	}
}

static int ZBErrcode(coordinator_cmd_t *pMsg, char *session)
{
	char *msg = NULL;
	err_info_t *perror = NULL;
	perror = pMsg->payload;
	if (pMsg->len > 0)
	{
		if (perror->errtype == 1)
			msg = "no key";
		else if (perror->errtype == 2)
			msg = "invalid key";
		else if (perror->errtype == 3)
			msg = "update key err";
		
		dodevres(session, "", msg, DEVMANAGER_KEY_WRONG);
	}
}

static int check_flag=0;
//è§£æžä»Žä¸²å£è¿‡æ¥çš„æ•°æ®
int decodeUARTMsg(coordinator_cmd_t * pMsg)
{
    DEBUG_PRINTF("[%s]====cmdid is %d=\n",__func__,pMsg->cmdid);
    DEBUG_PRINTF_ZIGBEECMD(pMsg,1);

    
    if( pMsg->cmdid == 0x00)
    {
        ZB_addEntry2SearchedList(pMsg); 
        check_flag=1;
    }
    else if(pMsg->cmdid == 0x01)
    {
        DEBUG_PRINTF("[%s]===ignore it===\n");
    }
    else if(pMsg->cmdid == 0x02)
    {
      
        ZB_DelFromManagedDeviceList(pMsg);
        check_flag=1;
    }
    else if(pMsg->cmdid == 0x03)
    {
        //ÔÝÊ±²»ÓÃ
        
    }
    else if(pMsg->cmdid == 0x04)
    {
       
        
        ZB_uplog(pMsg);
        check_flag=1;
    }
    else if(pMsg->cmdid == 0x05)
    {
      
        char *session;
        session = getpeerbyreqid(pMsg->seq);
        DEBUG_PRINTF("[%s]==cmdid:0x05==session:%s ====\n", __func__, session);
        DEBUG_PRINTF("[%s]===pMsg->len:%d=====\n",__func__,(int)pMsg->len);
        //¶ÔpMsg->payload ½øÐÐbase64¼ÓÂë
        char data[2048]={0};
        char data2[2048]={0};
        base64_encode(pMsg->payload, data, (int)pMsg->len);
        DEBUG_PRINTF("[%s]===data:%s===\n",__func__,data);

        snprintf(data2,sizeof(data2),"\\\"len\\\":%d,\\\"encrypdata\\\":\\\"%s\\\"",(int)pMsg->len,data);
        dodevres(session,data2,"ok",0);
        check_flag=1;
    }

    if(check_flag==1)
    {
        system("touch /tmp/.zigbeecheck");
    }

	return 0;	
}

int readUART()
{
	int len;
	coordinator_cmd_t zbMsg;

	initZBMsg(&zbMsg);
	while (-1 != UARTfd)
	{
		len = read(UARTfd, &zbMsg, sizeof(zbMsg));
		if (0 < len)
		{
			decodeUARTMsg(&zbMsg);
		}
	}
	
	return 0;
}

int openUART()
{
	struct termios opt, optold;

	UARTfd = open("/dev/ttyQHS0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (-1 == UARTfd)
	{
		return;
	}

	fcntl(UARTfd,F_SETFL,0);
	
	isatty(STDIN_FILENO);	

	tcgetattr(UARTfd, &optold);

	bzero(&opt, sizeof(opt));
	
	cfsetispeed(&opt, B38400);
	cfsetospeed(&opt, B38400);

	opt.c_cflag |= CLOCAL | CREAD;

	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |= CS8;

	opt.c_cflag &= ~CSTOPB;

	opt.c_cflag &= ~PARENB;
	
	//opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	//opt.c_iflag &= ~(INLCR | IGNCR | ICRNL);
	
	//opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	//opt.c_oflag &= ~OPOST;

	opt.c_cc[VTIME] = 10;//10 = 1s
	opt.c_cc[VMIN] = 0;

	tcflush(UARTfd, TCIOFLUSH);
	
	tcsetattr(UARTfd, TCSANOW, &opt);	
}

int closeUART()
{
	if (-1 != UARTfd)
		close(UARTfd);
	return 0;
}

