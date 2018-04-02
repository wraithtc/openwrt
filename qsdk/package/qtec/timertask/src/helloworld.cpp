#include <iostream>
#include <string>
#include <vector>
#include "timertask.h"
extern "C"{
#include "fwk.h"
#include "librtcfg.h"
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
}

using namespace std;
void *g_msgHandle;
map<int, string> g_testMap;
string logRemoteDest;

void wifi_handler(int isIn)
{
    char wifiStatusNow[BUFLEN_16] = {0};

    rtcfgUciGet("wireless.timertaskstatus", wifiStatusNow);
    
    if (isIn)
    {
        cout << "wifi down" << endl;
        if (util_strlen(wifiStatusNow) == 0 || util_strcmp(wifiStatusNow, "on") == 0)
        {
            UTIL_DO_SYSTEM_ACTION("wifi down");
            rtcfgUciSet("wireless.timertaskstatus=off");
            rtcfgUciCommit("wireless");
        }
    }
    else
    {
        cout << "wifi up" << endl;
        if (util_strcmp(wifiStatusNow, "off") == 0)
        {
            UTIL_DO_SYSTEM_ACTION("wifi up");
            rtcfgUciSet("wireless.timertaskstatus=on");
            rtcfgUciCommit("wireless");
        }
    }
}

int addTask(VosMsgHeader *msg, timertask &t)
{
    TASK_INFO_STRU *pstTaskInfo;
    vector<int> weekDay;
    string weekDayStr;
    unsigned int pos;
    int i = 0;
    int weekDayFinal = 0;
    TASK_DURATION_INFO di;

    if (!msg)
    {
        cout << "msg is NULL , return" << endl;
        return -1;
    }

    pstTaskInfo = (TASK_INFO_STRU *)(msg+1);
    if (!pstTaskInfo)
    {
        cout << "msg data error, return" << endl;
        return -1;
    }

    weekDayStr = pstTaskInfo->aucWeekDay;
    while ((pos = weekDayStr.find_first_of(",")) != string::npos)
    {
        string dayStr(weekDayStr.c_str(), pos);
        weekDay.push_back(atoi(dayStr.c_str()));
        weekDayStr = weekDayStr.substr(pos + 1, weekDayStr.length() - pos);
        cout <<"weekDayStr:"<<weekDayStr<<", dayStr:"<<dayStr<<endl;
    }
    weekDay.push_back(atoi(weekDayStr.c_str()));
    for (i = 0; i < weekDay.size(); i ++)
    {
        weekDayFinal |= 1 << weekDay[i];
    }
    cout <<"weekDayFinal:"<<weekDayFinal<<endl;
    di.day = weekDayFinal;
    di.duration = TASK_DURATION (TIME_INFO (pstTaskInfo->ucStartHour,pstTaskInfo->ucStartMin), TIME_INFO (pstTaskInfo->ucStopHour, pstTaskInfo->ucStopMin));
    di.enable = pstTaskInfo->ucEnable;
    di.taskId = pstTaskInfo->usTaskId;

    t.AddTask(weekDayFinal, di, "wifi", wifi_handler);
    t.printTask();
    return 0;
}

int delTask(VosMsgHeader *msg, timertask &t)
{
    int ret;
    
    ret = t.DelTask(msg->wordData, "wifi");
    t.printTask();
    return ret;
}

int editTask(VosMsgHeader *msg, timertask &t)
{
    TASK_INFO_STRU *pstTaskInfo;
    vector<int> weekDay;
    string weekDayStr;
    unsigned int pos;
    int i = 0;
    int weekDayFinal = 0;
    TASK_DURATION_INFO di;

    if (!msg)
    {
        cout << "msg is NULL , return" << endl;
        return -1;
    }

    pstTaskInfo = (TASK_INFO_STRU *)(msg+1);
    if (!pstTaskInfo)
    {
        cout << "msg data error, return" << endl;
        return -1;
    }

    weekDayStr = pstTaskInfo->aucWeekDay;
    while ((pos = weekDayStr.find_first_of(",")) != string::npos)
    {
        string dayStr(weekDayStr.c_str(), pos);
        weekDay.push_back(atoi(dayStr.c_str()));
        weekDayStr = weekDayStr.substr(pos + 1, weekDayStr.length() - pos);
        cout <<"weekDayStr:"<<weekDayStr<<", dayStr:"<<dayStr<<endl;
    }
    weekDay.push_back(atoi(weekDayStr.c_str()));
    for (i = 0; i < weekDay.size(); i ++)
    {
        weekDayFinal |= 1 << weekDay[i];
    }
    cout <<"weekDayFinal:"<<weekDayFinal<<endl;
    di.day = weekDayFinal;
    di.duration = TASK_DURATION (TIME_INFO (pstTaskInfo->ucStartHour,pstTaskInfo->ucStartMin), TIME_INFO (pstTaskInfo->ucStopHour, pstTaskInfo->ucStopMin));
    di.enable = pstTaskInfo->ucEnable;
    di.taskId = pstTaskInfo->usTaskId;

    t.EditTask(pstTaskInfo->usTaskId, "wifi", di, wifi_handler);
    t.printTask();
    return 0;
}

int setTaskSw(VosMsgHeader *msg, timertask &t)
{
    TASK_SW_INFO *pstTaskSwInfo = (TASK_SW_INFO *)(msg + 1);
    int i = 0;

    if (!pstTaskSwInfo)
    {
        cout << "msg error!" << endl;
        return -1;
    }
    t.SetTaskGlobalSw("wifi", pstTaskSwInfo->ucGlobalSw);

    for (i = 0; i < pstTaskSwInfo->ucTaskNum; i++)
    {
        t.SetTaskSw((int)pstTaskSwInfo->stRuleSwInfo[i].usTaskId, "wifi", (int)pstTaskSwInfo->stRuleSwInfo[i].ucEnable);
    }
    return 0;
}

void init_log()
{
	int f1;
    system("ulimit -c unlimited");

    UTIL_DO_SYSTEM_ACTION("touch /tmp/timertask");

	f1 = open("/tmp/timertask", O_RDWR | O_APPEND);

	if(f1)
	{
		dup2((int)f1,1);
		dup2((int)f1,2);

		close((int)f1);
	}
	
}

void send_log()
{
    int f1;

	f1 = open("/dev/null", O_RDWR | O_APPEND);

	cout << "remote ip: " << logRemoteDest << endl;
    UTIL_DO_SYSTEM_ACTION("cd /tmp;tftp -p -l timertask %s", logRemoteDest.c_str());
    unlink("/tmp/timertask");
    if(f1)
	{
		dup2((int)f1,1);
		dup2((int)f1,2);

		close((int)f1);
	}
}

int initTaskOnBoot(timertask &t)
{
    TIMER_TASK_MNG_STRU *pTaskMngInfo;
    TIMER_TASK_STRU *pStartTaskInfo, *pStopTaskInfo;
    int pos, npos;
    TASK_INFO_STRU stTaskInfo;
    vector<int> weekDay;
    string weekDayStr;
    int weekDayFinal;
    int i =0;
    TASK_DURATION_INFO di;
    
    TimerTaskLoadConfig();
    pTaskMngInfo = TimerTaskGet();
    t.SetTaskGlobalSw("wifi", pTaskMngInfo->ulEnable);
    
    cout << "taskNum:"<< pTaskMngInfo->ulTaskNum << endl;
    for (pos = 1; pos < pTaskMngInfo->ulTaskNum + 1; pos += 2)
    {
        weekDay.clear();
        weekDayStr.clear();
        weekDayFinal = 0;
        
        pStartTaskInfo = &pTaskMngInfo->stTaskInfo[pos];
        pStopTaskInfo = &pTaskMngInfo->stTaskInfo[pos+1];
        
        stTaskInfo.ucEnable = pStartTaskInfo->ucEnable;
        stTaskInfo.usTaskId = pStartTaskInfo->usTaskId;
        stTaskInfo.ucStartHour = pStartTaskInfo->ucTimerHour;
        stTaskInfo.ucStartMin = pStartTaskInfo->ucTimerMin;
        stTaskInfo.ucStopHour = pStopTaskInfo->ucTimerHour;
        stTaskInfo.ucStopMin = pStopTaskInfo->ucTimerMin;
        UTIL_STRNCPY(stTaskInfo.aucWeekDay, pStartTaskInfo->aucTimeWeek, sizeof(stTaskInfo.aucWeekDay));

        weekDayStr = stTaskInfo.aucWeekDay;
        while ((npos = weekDayStr.find_first_of(",")) != string::npos)
        {
            string dayStr(weekDayStr.c_str(), npos);
            weekDay.push_back(atoi(dayStr.c_str()));
            weekDayStr = weekDayStr.substr(npos + 1, weekDayStr.length() - npos);
            cout <<"weekDayStr:"<<weekDayStr<<", dayStr:"<<dayStr<<endl;
        }
        weekDay.push_back(atoi(weekDayStr.c_str()));
        for (i = 0; i < weekDay.size(); i ++)
        {
            weekDayFinal |= 1 << weekDay[i];
        }
        cout <<"weekDayFinal:"<<weekDayFinal<<endl;
        di.day = weekDayFinal;
        di.duration = TASK_DURATION (TIME_INFO (stTaskInfo.ucStartHour,stTaskInfo.ucStartMin), TIME_INFO (stTaskInfo.ucStopHour, stTaskInfo.ucStopMin));
        di.enable = stTaskInfo.ucEnable;
        di.taskId = stTaskInfo.usTaskId;
        t.AddTask(weekDayFinal, di, "wifi", wifi_handler);
    }
}

int main()
{
    timertask t;
    VOS_RET_E ret;
    struct timeval tm;
    int commFd = -1;
	int maxFd = -1;
	int fd, rv;
	fd_set readFdsMaster,rfds;
    UtilTimestamp nowTs;
    int sleepMs = 10000;
    VosMsgHeader *msg = NULL;
    
    ret = vosMsg_init(EID_TIMER_TASK, &g_msgHandle);
    if (ret != VOS_RET_SUCCESS)
    {
        cout << "Fail to init msg handler, ret = " << ret << endl;
        return -1;
    }

    initTaskOnBoot(t);
    t.printTask();
    /* set our bit masks according to the master */
    vosMsg_getEventHandle(g_msgHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;
    tm.tv_sec = sleepMs / MSECS_IN_SEC;
    tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
    while (1)
    {
        /* pend, waiting for one or more fds to become ready */
        tm.tv_sec = sleepMs / MSECS_IN_SEC;
        tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
        rfds = readFdsMaster;
        rv = select(maxFd + 1, &rfds, NULL, NULL, &tm);
        if (rv <= 0)
        {
            t.taskRun();
            continue;
        }
        if (FD_ISSET(commFd, &rfds))
		{
			ret = vosMsg_receive(g_msgHandle, &msg);
			if (ret != VOS_RET_SUCCESS)
			{
				continue;
			}
            if (msg)
            {
    			switch(msg->type)
    			{				
    				case VOS_MSG_ADD_WIFI_TIMER_TASK:
    				{
    					cout << "receive add wifi timer task msg" << endl;
                        addTask(msg, t);
    					break;
    				}
                    case VOS_MSG_DEL_WIFI_TIMER_TASK:
                    {
                        cout << "receive del wifi timer task msg" << endl;
                        delTask(msg, t);
                        break;
                    }

                    case VOS_MSG_EDIT_WIFI_TIMER_TASK:
                    {
                        cout << "receive edit wifi timer task msg" << endl;
                        editTask(msg, t);
                    }

                    case VOS_MSG_SET_WIFI_TIMER_TASK_SW:
                    {
                        cout << "receive set wifi timer task switch msg" << endl;
                        setTaskSw(msg, t);
                        break;
                    }

                    case VOS_MSG_SET_WIFI_TIMER_TASK_GLOBAL_SW:
                    {
                        cout << "receive set wifi timer task global switch msg" << endl;
                        t.SetTaskGlobalSw("wifi", msg->wordData);
                        break;
                    }

                    case VOS_MSG_LOG_REDIRECT:
                    {
                        char destip[BUFLEN_32] = {0};
                        cout << "receive log redirect msg" << endl;
                        init_log();
                        memcpy(destip, (char *)(msg+1), sizeof(destip));
                        string tmpString(destip);
                        logRemoteDest = tmpString;
                        cout << "destip:" << destip << "logRemoteDest:" << logRemoteDest<<endl;
                        break;
                    }

                    case VOS_MSG_LOG_REDIRECT_END:
                    {
                        cout << "receive log redirect end msg" << endl;
                        send_log();
                        break;
                    }

                    default:
                        cout << "unkown msg" << endl;
                        break;
                }

                VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
            }
        }
        
    }

    vosMsg_cleanup(&g_msgHandle);
    return 0;
}