#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "fwk.h"
#include "cJSON.h"

#define DEBUG_FILE    "/tmp/scrctrl"
#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0

#define SEVENSEG_DIGITS 5


void *g_mcHandle =NULL;
time_t g_firstKeyDisplayTime = -300;
time_t g_lastWanSpeedDisplayTime = -5;

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};

void write_log(const char *fmt, ...)
{
    int ret = 0;
    char buf[4096] = {0};
    char *cmd = NULL;
    char *allocBuf = NULL;
    va_list paraList;
    FILE *fp;

    if (access(DEBUG_FILE, F_OK) != 0)
        return;
    
    va_start(paraList, fmt);
    vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    fp = fopen(DEBUG_FILE, "a");
    fprintf(fp,"%s", buf);
    fclose(fp);
}


void setBlink(int devid, int blinkType)
{
    char cmd[128] = {0};

    char value = HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (blinkType << 1);

    snprintf(cmd, sizeof(cmd), "i2cset -y 0 0x%.2x 0x%.2x",devid, value);
    system(cmd);
}

void setBrightness(int devid, int brightness)
{
    char cmd[128] = {0};

    char value = HT16K33_CMD_BRIGHTNESS | brightness;

    snprintf(cmd, sizeof(cmd), "i2cset -y 0 0x%.2x 0x%.2x",devid, value);
    system(cmd);
}


void begin(int devid)
{
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), "i2cset -y 0 0x%.2x 0x21",devid);
    system(cmd);
    setBlink(devid, 0);
    setBrightness(devid, 15);
}


void proc_first_key_display(VosMsgHeader *msg)
{
    char disBuffer[8] = {0};
    char cmd[128] = {0};
    struct timeval timenow;

    UTIL_STRNCPY(disBuffer, (char *)(msg+1), sizeof(disBuffer));
    snprintf(cmd, sizeof(cmd), "i2c_ctrl %s", disBuffer);
    printf("%s\n", cmd);
    system(cmd);
    gettimeofday(&timenow, NULL);
    g_firstKeyDisplayTime = timenow.tv_sec;
}

void proc_wan_speed_display()
{
    struct VosMsgBody stMsg={0};
	VosMsgHeader *pstReplyMsg;
    cJSON *subJson, *data;
    int ret, routertx, routerrx;
    char cmd[128] = {0};
    struct timeval timenow;
    
    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_LANHOST;
    stMsg.stHead.src = EID_SCRCTRL;
    stMsg.stHead.type = VOS_MSG_ROUTER_GETSTATUS;
	stMsg.stHead.flags_request = 1;

	ret = vosMsg_sendAndGetReplyBufWithTimeout(g_mcHandle, (const VosMsgHeader *)&stMsg, &pstReplyMsg, MSECS_IN_SEC);
	if(ret != VOS_RET_SUCCESS)
	{
		write_log("get reply msg failed, ret= %d", ret);
		return;
	}

	if(pstReplyMsg)
	{
		if(stMsg.stHead.type == VOS_MSG_ROUTER_GETSTATUS)
		{
			subJson = cJSON_Parse(((struct VosMsgBody *)pstReplyMsg)->buf);
            if (subJson)
            {
                routertx = cJSON_GetObjectItem(subJson, "routertx")?cJSON_GetObjectItem(subJson, "routertx")->valueint:0;
                routerrx = cJSON_GetObjectItem(subJson, "routerrx")?cJSON_GetObjectItem(subJson, "routerrx")->valueint:0;
                
    			write_log("proc_wan_speed_display: pstReplyMsg->buf is %s, routertx:%d\n", ((struct VosMsgBody *)pstReplyMsg)->buf, routerrx);		

                if (routerrx < 1024)
                {
                    system("i2c_ctrl 000ffk");
                }
                else if (routerrx >= 1024 && routerrx < 1024*1024)
                {
                    routerrx = routerrx/1024;
                    snprintf(cmd, sizeof(cmd), "i2c_ctrl %03dffk", routerrx);
                    write_log("cmd:[%s]\n", cmd);
                    system(cmd);
                }
                else
                {
                    routerrx = routerrx/(1024*1024);
                    snprintf(cmd, sizeof(cmd), "i2c_ctrl %03dffm", routerrx);
                    write_log("cmd:[%s]\n", cmd);
                    system(cmd);
                }

                gettimeofday(&timenow, NULL);
                g_lastWanSpeedDisplayTime = timenow.tv_sec;
                cJSON_Delete(subJson);
            }
		}
        VOS_MEM_FREE_BUF_AND_NULL_PTR(pstReplyMsg);
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
    int i;
    int commFd = -1;
	int maxFd = -1;
	int fd, rv, n;
	fd_set readFdsMaster,rfds;
	VosMsgHeader *msg = NULL;
    struct timeval timenow;
    struct timeval tm;
    int sleepMs = 200;
    
    
    vosLog_init(EID_SCRCTRL);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
	vosMsg_init(EID_SCRCTRL, &g_mcHandle);

    vosMsg_getEventHandle(g_mcHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;

    begin(0x70);
    begin(0x71);
    while(1)
	{
		rfds = readFdsMaster;
        tm.tv_sec = sleepMs / MSECS_IN_SEC;
        tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
		n = select(maxFd+1, &rfds, NULL, NULL, &tm);
        if (n < 0)
        {
            printf("msg queue error, exit!");
            break;
        }
        else if (n > 0)
        {
    		if (FD_ISSET(commFd, &rfds))
    		{
    			ret = vosMsg_receive(g_mcHandle, &msg);
    			if (ret != VOS_RET_SUCCESS)
    			{
    				continue;
    			}
    			switch(msg->type)
    			{
    			    case VOS_MSG_FIRST_KEY_DISPLAY_REQ:
                        proc_first_key_display(msg);
                        break;

                    default:
                        break;
                }
            }
        }
        gettimeofday(&timenow, NULL);
        
        if (timenow.tv_sec - g_firstKeyDisplayTime > 300 && timenow.tv_sec - g_lastWanSpeedDisplayTime > 3)
        {
            proc_wan_speed_display();
        }
    }

    vosMsg_cleanup(&g_mcHandle);
    vosLog_cleanup();
}
