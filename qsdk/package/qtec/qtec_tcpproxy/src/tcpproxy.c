#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "fwk.h"
#include "librtcfg.h"
#include "cJSON.h"
#include "thread_pool.h"

#define DEBUG_FILE    "/tmp/tcpproxy"
#define THREAD 32
#define QUEUE  256

void *g_mcHandle =NULL;
int g_fd2server = -1;
int g_fd2router = -1;
pthread_mutex_t lock;
int taskNum = 0;

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};

typedef struct{
    char appUser[128];
    char appChannelId[128];
    char serverIp[128];
    int port;
}TCP_PROXY_INFO;

typedef struct{
    int fd2server;
    int fd2samba;
}SOCK_INFO;

void _write_log(const char *fmt, ...)
{
    char buf[4096] = {0};
    va_list paraList;
    FILE *fp;

    va_start(paraList, fmt);
    vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    fp = fopen(DEBUG_FILE, "a");
    if (fp)
    {
        fprintf(fp,"%s", buf);
        fclose(fp);
    }
}

void write_log(const char *fmt, ...)
{

    if (access(DEBUG_FILE, F_OK) != 0)
        return;
    
    char buf[4096] = {0};
    va_list paraList;
    FILE *fp;

    va_start(paraList, fmt);
    vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    fp = fopen(DEBUG_FILE, "a");
    if (fp)
    {
        fprintf(fp,"%s", buf);
        fclose(fp);
    }
}

void write_hex_log(const char *hexBuf, int bufLen)
{
    int i;
    if (access(DEBUG_FILE, F_OK) != 0)
        return;

    for (i = 0; i < bufLen; i++)
        write_log("%02x ", hexBuf[i]);
    write_log("\n");
}

static int ns_is_error(int n)
{
    return n == 0 ||
    (n < 0 && errno != EINTR && errno != EINPROGRESS &&
    errno != EAGAIN && errno != EWOULDBLOCK);
}


int proc_server_to_samba(SOCK_INFO *skInfo)
{
    char buffer[40960] = {0};
    int i = 0, ret = 0;
    fd_set readset, testset;
    int sendLen = 0;
    
    write_log("fd2samba=%u, fd2server=%u\n", skInfo->fd2samba,  skInfo->fd2server);
    if (skInfo->fd2samba == -1 || skInfo->fd2server == -1)
    {
        write_log("socket not ready\n");
        return -1;
    }
    
    if(fcntl(skInfo->fd2server, F_SETFL, O_NONBLOCK))
    {
        write_log("Fail to set server fd to non block, errno=%d, threadId=%u\n", errno, pthread_self());
        goto cleanup;
    }
    while (1)
    {
        struct timeval tm;
        int sleepMs = 10;
        
        FD_ZERO( & readset);
        FD_SET(skInfo->fd2server,  & readset);
        testset  =  readset;
        tm.tv_sec = sleepMs / MSECS_IN_SEC;
        tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
        ret  =  select(skInfo->fd2server+1 ,  & testset, NULL, NULL, &tm);
        while (1)
        {
            int bufLen = 0;
            memset(buffer, 0, sizeof(buffer));
            bufLen = recv(skInfo->fd2server, buffer, sizeof(buffer), 0);
            if (ns_is_error(bufLen))
            {
                write_log("socket error or server disconnect, errno = %d, threadId=%u\n", errno, pthread_self());
                goto cleanup;
            }
            else if (bufLen > 0)
            {
                write_log("receive request from server, buflen=%d, threadId=%u\n", bufLen, pthread_self());
                write_log("request buffer:\n");
                //write_hex_log(buffer, bufLen);
                write_log("\n");
                sendLen=0;
                while (sendLen != bufLen)
                {
                    ret = send(skInfo->fd2samba, buffer+sendLen, bufLen-sendLen, 0);
                    if (ret <= 0) 
                    {
                        if (errno  ==  EAGAIN  ||  errno  ==  EWOULDBLOCK)
                        {
                            usleep(1000);
                            continue;
                        }
                        else
                        {
                            write_log("send to samba error, errno=%d, threadId=%u\n", errno, pthread_self());
                            goto cleanup;
                        }
                    }
                    else
                    {
                        sendLen+=ret;
                        write_log("send request to samba, buflen=%d, threadId=%u\n", ret, pthread_self());
                    }
                }
            }
            else if (bufLen < 0 && (errno  ==  EAGAIN  ||  errno  ==  EWOULDBLOCK))
            {
                //write_log("recv finished\n");
                break;
            }
        }
    }
cleanup:
    if (skInfo->fd2server != -1)
    {
        write_log("close fd2server, func=%s\n", __func__);
        close(skInfo->fd2server);
        skInfo->fd2server = -1;
    }
    if (skInfo->fd2samba != -1)
    {
        write_log("close fd2samba, func=%s\n", __func__);
        close(skInfo->fd2samba);
        skInfo->fd2samba = -1;
    }
    pthread_detach(pthread_self());
}

int proc_samba_to_server(SOCK_INFO *skInfo)
{
    char buffer[4096] = {0};
    int i = 0, ret = 0;
    fd_set readset, testset;
        
    write_log("fd2samba=%u, fd2server=%u\n", skInfo->fd2samba,  skInfo->fd2server);
    if (skInfo->fd2samba == -1 || skInfo->fd2server == -1)
    {
        write_log("socket not ready\n");
        return -1;
    }
    /*
    FD_ZERO( & readset);
    FD_SET(skInfo->fd2samba,  & readset);
    testset  =  readset;
    ret  =  select(skInfo->fd2samba+1 ,  & testset, NULL, NULL, NULL);
    write_log("fd2samba got msg!\n");
    */
    while (1)
    {
        int bufLen = 0;
        memset(buffer, 0, sizeof(buffer));
        bufLen = recv(skInfo->fd2samba, buffer, sizeof(buffer), 0);
        if (ns_is_error(bufLen) && bufLen != EBADF)
        {
            write_log("socket error or samba disconnect, errno = %d, threadId=%u\n", errno, pthread_self());
            break;
        }
        else if (bufLen > 0)
        {
            write_log("receive response from samba, buflen=%d, threadId=%u\n", bufLen, pthread_self());
            write_log("response buffer:\n");
            for (i = 0; i < bufLen; i++)
            {
                write_log("%02x ", buffer[i]);
            }
            write_log("\n");
            ret = send(skInfo->fd2server, buffer, bufLen, 0);
            if (ret <= 0)
            {
                write_log("send to server error, errno=%d, threadId=%u\n", errno, pthread_self());
                break;
            }
            write_log("send response to server, buflen=%d, threadId=%u\n", bufLen, pthread_self());
        }
        else
        {
            continue;
        }
    } 
    if (skInfo->fd2server != -1)
    {
        write_log("close fd2server, func=%s\n", __func__);
        close(skInfo->fd2server);
        skInfo->fd2server = -1;
    }
    if (skInfo->fd2samba != -1)
    {
        write_log("close fd2samba, func=%s\n", __func__);
        close(skInfo->fd2samba);
        skInfo->fd2samba = -1;
    }
    pthread_detach(pthread_self());
}


int proc_connect_to_server(struct VosMsgBody *msg)
{
    char server_ip[128] = {0};
    int server_port = 0;
    int fd2server = -1;
    int fd2samba = -1;
    int i;
    int sendLen = 0;
    TCP_PROXY_INFO *msgData = (TCP_PROXY_INFO *)msg->buf;
    struct sockaddr_in serverAddr= {0}; 
    int ret;
    struct systemInfo sysInfo;
    char buffer[40960] = {0};
    char *firstMsgFormat = "{\"url\":\"qtecRouterConAuth\",\"clientId\":\"%s\",\"token\":\"%s\"}";
    char rawKey[32] = {0};
    char keyStrBuf[64] = {0};
    SOCK_INFO skInfo = {-1,-1};
    pthread_t id_1;
    fd_set readset, testset;
    struct timeval timeNow;
    if (!msgData)
    {
        write_log("connetc to server msg error!\n");
        goto cleanup;
    }

    fd2server = socket(PF_INET, SOCK_STREAM, 0);  
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(msgData->port);  
    serverAddr.sin_addr.s_addr = inet_addr(msgData->serverIp);
    write_log("connect to %s:%d\n", msgData->serverIp, msgData->port);
    ret = connect(fd2server, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if (ret != 0)
    {
        write_log("connect to %s:%d failed, errno=%d\n", msgData->serverIp, msgData->port, errno);
        goto cleanup;
    }
    #if 0
    getSystemInfo(&sysInfo);
    LoadData(rawKey, sizeof(rawKey));
    snprintf(keyStrBuf, sizeof(keyStrBuf), "%02x%02x%02x%02x%02x%02x%02x%02x", 
        (unsigned int)rawKey[0], (unsigned int)rawKey[1], (unsigned int)rawKey[2], (unsigned int)rawKey[3], 
        (unsigned int)rawKey[4], (unsigned int)rawKey[5], (unsigned int)rawKey[6], (unsigned int)rawKey[7]);
    snprintf(buffer, sizeof(buffer), firstMsgFormat, sysInfo.serialnum, keyStrBuf);
    write_log("send to server:[%s]\n", buffer);
    ret = send(fd2server, buffer, strlen(buffer)+1, 0);
    if (ret < 0)
    {
        write_log("fail to send serialnum to server, errno=%d\n", errno);
        goto cleanup;
    }

    do
    {
        int bufLen = 0;
        memset(buffer, 0, sizeof(buffer));
        bufLen = recv(fd2server, buffer, sizeof(buffer), 0);
        if (bufLen <= 0)
        {
            write_log("socket error or server disconnect, errno = %d\n", errno);
            goto cleanup;
        }
        write_log("receive from server:[%s]\n", buffer);
        if (strstr(buffer, "success") == NULL)
        {
            write_log("auth failed\n");
            goto cleanup;
        }
    }while(0);
#endif 
    int sambaPort = 445;
    char *sambaIp = "127.0.0.1";
    if (access("/tmp/tpdebug", F_OK) == 0)
    {
        sambaPort = 80;
        sambaIp = "192.168.1.10";
    }

    fd2samba = socket(PF_INET, SOCK_STREAM, 0); 
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(sambaPort);  
    serverAddr.sin_addr.s_addr = inet_addr(sambaIp);
    write_log("connect to samba\n");
    ret = connect(fd2samba, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if (ret != 0)
    {
        write_log("connect to samba, errno=%d\n", errno);
        goto cleanup;
    }

    skInfo.fd2samba = fd2samba;
    skInfo.fd2server = fd2server;
    write_log("fd2samba=%u, fd2server=%u\n", skInfo.fd2samba,  skInfo.fd2server);
    ret=pthread_create(&id_1,NULL,(void  *) proc_server_to_samba,(void *)&skInfo);  
    if(ret!=0)  
    {  
        write_log("Create pthread server to samba error!\n");  
        goto cleanup ;  
    } 
    write_log("Create pthread server to samba , threadid=%u!\n", id_1); 
#if 1
    if(fcntl(fd2samba, F_SETFL, O_NONBLOCK))
    {
        write_log("Fail to set server fd to non block, errno=%d, threadId=%u\n", errno, pthread_self());
        goto cleanup;
    }
    while (1)
    {
        struct timeval tm;
        int sleepMs = 10;
        
        FD_ZERO( & readset);
        FD_SET(fd2samba,  & readset);
        testset  =  readset;
        tm.tv_sec = sleepMs / MSECS_IN_SEC;
        tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
        ret  =  select(fd2samba+1 ,  & testset, NULL, NULL, &tm);
        while (1)
        {
            int bufLen = 0;
            memset(buffer, 0, sizeof(buffer));
            bufLen = recv(fd2samba, buffer, sizeof(buffer), 0);
            if (ns_is_error(bufLen))
            {
                write_log("socket error or samba disconnect, errno = %d\n", errno);
                goto cleanup;
            }
            else if (bufLen > 0)
            {
                write_log("receive response from samba, buflen=%d\n", bufLen);
                write_log("request buffer:\n");
                write_log("\n");
                sendLen=0;
                while (sendLen != bufLen)
                {
                    ret = send(fd2server, &buffer[sendLen], bufLen-sendLen, 0);
                    if (ret <= 0) 
                    {
                        if (errno  ==  EAGAIN  ||  errno  ==  EWOULDBLOCK)
                        {
                            usleep(1000);
                            continue;
                        }
                        else
                        {
                            write_log("send to server error, errno=%d\n", errno);
                            goto cleanup;
                        }
                    }
                    else
                    {
                        sendLen+=ret;
                        write_log("send response to server, buflen=%d\n", ret);
                    }
                }
            }
            else if (bufLen < 0 && (errno  ==  EAGAIN  ||  errno  ==  EWOULDBLOCK))
            {
                break;
            }
        }
    }
    #endif
    
cleanup:
    if (fd2server != -1)
    {
        write_log("close fd2server, func=%s\n", __func__);
        close(fd2server);
        fd2server=-1;
    }
    if (fd2samba != -1)
    {
        write_log("close fd2samba, func=%s\n", __func__);
        close(fd2samba);
        fd2samba=-1;
    }
    pthread_mutex_lock(&lock);
    taskNum--;
    write_log("task num decrease, taskNum=%d\n", taskNum);
    pthread_mutex_unlock(&lock);
    pthread_detach(pthread_self());
    return 0;
}


int main(int argc, char *argv[])
{
	int ret = 0;
    int i;
    struct sockaddr_in serverAddr;  
    int commFd = -1;
	int maxFd = -1;
	int fd, rv, n;
	fd_set readFdsMaster,rfds;
	VosMsgHeader *msg = NULL;
    struct timeval tm;
    int sleepMs = 200;
    pthread_t id_1;
    threadpool_t *pool;
    
    vosLog_init(EID_TCP_PROXY);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
	ret = vosMsg_init(EID_TCP_PROXY, &g_mcHandle);

    if (ret != VOS_RET_SUCCESS)
    {
        write_log("Fail to connect to smd, exit\n");
        vosLog_cleanup();
        return -1;
    }

    pthread_mutex_init(&lock, NULL);
    pool = threadpool_create(THREAD, QUEUE, 0);
    if (!pool)
    {
        write_log("Fail to create threadpool!\n");
        return -1;
    }
    vosMsg_getEventHandle(g_mcHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;

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
    			    case VOS_MSG_TCPPROXY_CONNET_SERVER_REQ:
                        write_log("receive set up tcpproxy msg\n");
                        pthread_mutex_lock(&lock);                     
                        if (1)//(taskNum < 8)
                        {
                            ret = threadpool_add(pool, (void *)proc_connect_to_server, (void *)msg, 0);
                            if (ret != 0)
                            {
                                write_log("thread pool add failed, ret = %d\n", ret);
                            }
                            else
                            {
                                taskNum++;
                                write_log("Added %d tasks\n", taskNum);
                            }
                            
                        }
                        pthread_mutex_unlock(&lock);
                        break;

                    default:
                        break;
                }
            }
        }
    }

    threadpool_destroy(pool, 0);
    vosMsg_cleanup(&g_mcHandle);
}
