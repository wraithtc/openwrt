#include "fwk.h"
#include <errno.h>
#include <sys/un.h>
#include "smd.h"
#include <sys/sem.h>

#define SMD_UPDATE_MAXFD(f)    (g_maxFd = (f > g_maxFd) ? f : g_maxFd)

int g_maxFd;

fd_set g_readFdsMaster;
static int sg_ipcListenFd;
int g_keepLooping = 1;
SMD_LOCK_CONTEXT smdLckCtx = {0};
/** Number of sempahores in the semphore array to create.
 * 
 * We only need 1.
 */
#define NUM_SEMAPHORES  1


/** Linux kernel semaphore identifier. */
static SINT32 semid = -1;

/** Index into semaphore array.
 *
 * We created a semaphore array with only 1 element, so the index is always 0.
 */
static SINT32 semIndex = 0;


DLIST_HEAD(sg_dlsInfoHead);
DLIST_HEAD(sg_evtInterestHead);



static void smd_initLoggingFromConfig(void)
{
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
}

static int smd_initUnixDomainServerSocket()
{
    struct sockaddr_un serverAddr;
    SINT32 fd, rc;

    /* mwang_todo:this is rather brute force, if we see the file here, maybe 
     * another smd is running...
     */
    unlink(SMD_MESSAGE_ADDR);


    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        vosLog_error("Could not create socket");
        return fd;
    }

    /*
     * Bind my server address and listen.
     */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sun_family = AF_LOCAL;
    strncpy(serverAddr.sun_path, SMD_MESSAGE_ADDR, sizeof(serverAddr.sun_path));

    rc = bind(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (rc != 0)
    {
        vosLog_error("bind to %s failed, rc=%d errno=%d", SMD_MESSAGE_ADDR, rc, errno);
        close(fd);
        return UTIL_INVALID_FD;
    }

    rc = listen(fd, SMD_MESSAGE_BACKLOG);
    if (rc != 0)
    {
        vosLog_error("listen to %s failed, rc=%d errno=%d", SMD_MESSAGE_ADDR, rc, errno);
        close(fd);
        return UTIL_INVALID_FD;
    }

    vosLog_notice("smd msg socket opened and ready (fd=%d)", fd);

    return fd;
}


static void smd_launchApp(SMD_DLS_INFO_T * dInfo)
{
    SpawnProcessInfo spawnInfo;
    SpawnedProcessInfo procInfo;
    char exeBuf[BUFLEN_1024];
    char argsBuf[BUFLEN_1024];
    VOS_RET_E ret;
    VosEntityInfo *eInfo = NULL;

    if (NULL == dInfo->eInfo)
    {
        vosLog_error("NULL == dInfo->eInfo");
        return;
    }

    dInfo->pid = UTIL_INVALID_PID;

    if (!dInfo->eInfo->isFeatureCompiledIn)
    {
        vosLog_notice("launch %s is not support", dInfo->eInfo->name);
        return;
    }

    fprintf(stderr, "Launch app %s\n", dInfo->eInfo->name);

    if (dInfo->eInfo->flags & EIF_MONITOR)
    {
        if (dInfo->eInfo->lifeNum < 0)
        {
            vosLog_error("%s game over, reboot system...", dInfo->eInfo->name);
            UTIL_DO_SYSTEM_ACTION("reboot");
        }
        else
        {
            eInfo = (VosEntityInfo *)dInfo->eInfo;
            eInfo->lifeNum--;
            vosLog_notice("%s left %d lifes", dInfo->eInfo->name, dInfo->eInfo->lifeNum);
        }
    }

    UTIL_SNPRINTF(argsBuf, sizeof(argsBuf), "%s", dInfo->eInfo->runArgs);

    UTIL_SNPRINTF(exeBuf, sizeof(exeBuf), "%s", dInfo->eInfo->path);

    if (dInfo->optArgs[0] != 0)
    {
        UINT32 idx, remaining, wrote;

        idx = strlen(argsBuf);
        remaining = sizeof(argsBuf) - idx;
        wrote = snprintf(&(argsBuf[idx]), remaining, " %s", dInfo->optArgs);
        if (wrote >= remaining)
        {
            vosLog_error("args buf overflow, wrote=%d remaining=%d (%s)", wrote, remaining, argsBuf);
            /* the app will probably not start correctly because the command line
             * is messed up.  All we can do is report it at this point.  */
        }
    }

#ifdef DESKTOP_LINUX
    if ((dInfo->eInfo->flags & EIF_DESKTOP_LINUX_CAPABLE) == 0)
    {
        vosLog_notice("pretend to launch %s, args=%s", exeBuf, argsBuf);
        dInfo->state = DLS_RUNNING;
        sg_desktopFakePid++;
        dInfo->pid = sg_desktopFakePid;
        if (dInfo->eInfo->flags & EIF_MULTIPLE_INSTANCES)
        {
            dInfo->specificEid = MAKE_SPECIFIC_EID(sg_desktopFakePid, dInfo->eInfo->eid);
        }
        return;
    }

    /*
     * We need to change the exeBuf path to point to the CommEngine
     * build directory.
     */
    ret = util_getBaseDir(exeBuf, sizeof(exeBuf));
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("could not determine baseDir");
        return;
    }

    /* Most apps that we spawn in the DESKTOP are here */
    UTIL_SNPRINTF(&(exeBuf[strlen(exeBuf)]), sizeof(exeBuf) - strlen(exeBuf),
        "/usr/S304/bin/%s", dInfo->eInfo->name);

    if (!utilFil_isFilePresent(exeBuf))
    {
        vosLog_debug("not at %s", exeBuf);

        /* check alternate locations for the desktop app */
        util_getBaseDir(exeBuf, sizeof(exeBuf));
        UTIL_SNPRINTF(&(exeBuf[strlen(exeBuf)]), sizeof(exeBuf) - strlen(exeBuf),
            "/bin/%s", dInfo->eInfo->name);

        if (!utilFil_isFilePresent(exeBuf))
        {
            vosLog_debug("could not find %s at %s", dInfo->eInfo->name, exeBuf);

            util_getBaseDir(exeBuf, sizeof(exeBuf));
            UTIL_SNPRINTF(&(exeBuf[strlen(exeBuf)]), sizeof(exeBuf) - strlen(exeBuf),
                "/usr/local/bin/%s", dInfo->eInfo->name);

            if (!utilFil_isFilePresent(exeBuf))
            {
                vosLog_error("could not find %s at any location, give up",
                            dInfo->eInfo->name);
                return;
            }
        }
    }
#endif

    spawnInfo.exe = exeBuf;
    spawnInfo.args = argsBuf;
    spawnInfo.spawnMode = SPAWN_AND_RETURN;
    spawnInfo.stdinFd = 0;
    spawnInfo.stdoutFd = 1;
    spawnInfo.stderrFd = 2;
    spawnInfo.serverFd = dInfo->serverFd;
    spawnInfo.maxFd = g_maxFd;
    spawnInfo.inheritSigint = TRUE;

    vosLog_debug("spawning %s args %s", spawnInfo.exe, spawnInfo.args);
    ret = prctl_spawnProcess(&spawnInfo, &procInfo);
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("could not spawn child %s args %s", dInfo->eInfo->path, argsBuf);
        /* smd timer will detect the child is not running */
    }
    else
    {
        dInfo->pid = procInfo.pid;
        if (dInfo->eInfo->flags & EIF_MULTIPLE_INSTANCES)
        {
            dInfo->specificEid = MAKE_SPECIFIC_EID(procInfo.pid, dInfo->eInfo->eid);
        }
        /* why not set specificEid to the generic eid when the app does not support multiple instances?
         * It will reduce a lot of checking elsewhere in the code.
         */

        vosLog_debug("%s launched, pid %d", dInfo->eInfo->name, dInfo->pid);
        //vosLog_debug("%s eid=0x%x,serverFd=%d,commFd=%d",dInfo->eInfo->eid,dInfo->serverFd,dInfo->commFd);
    }


    if (dInfo->eInfo->flags & EIF_MESSAGING_CAPABLE)
    {
        dInfo->state = DLS_LAUNCHED;
    }
    else
    {
        vosLog_debug("%s is not message capable, mark it as RUNNING without conf.", dInfo->eInfo->name);
        dInfo->state = DLS_RUNNING;
    }


    /* once we launch an app, stop monitoring its fd */
    if ((dInfo->eInfo->flags & EIF_SERVER) ||
        (dInfo->eInfo->eid == EID_CONSOLED))
    {
        FD_CLR(dInfo->serverFd, &g_readFdsMaster);
    }

    return;
}



void SMD_launchOnBoot(UINT32 stage)
{
    SMD_DLS_INFO_T *dInfo;

    vosLog_notice("stage=%d", stage);

    dlist_for_each_entry(dInfo, &sg_dlsInfoHead, dlist)
    {
        if (NULL == dInfo->eInfo)
        {
            vosLog_error("dInfo->eInfo is NULL");
            continue;
        }

        if (((stage == 1) && (dInfo->eInfo->flags & EIF_LAUNCH_IN_STAGE_1)) ||
            ((stage == 2) && (dInfo->eInfo->flags & EIF_LAUNCH_ON_BOOT)))
        {
            VosMsgHeader *msg;

            /*
             * queue a SYSTEM_BOOT message on the msg queue for the process.
             * this will be delivered to the process once is connects back to smd.
             */
            if (dInfo->eInfo->flags & EIF_MESSAGING_CAPABLE)
            {
                msg = (VosMsgHeader *) VOS_MALLOC_FLAGS(sizeof(VosMsgHeader), ALLOC_ZEROIZE);
                if (msg == NULL)
                {
                    vosLog_error("malloc of msg header failed.");
                    break;
                }

                /* fill in the message */
                msg->type = VOS_MSG_SYSTEM_BOOT;
                msg->src = EID_SMD;
                msg->dst = dInfo->eInfo->eid;
                msg->flags_event = 1;

                /*
                 * An app which was marked as launch-on-boot during stage 2 could
                 * have been already launched during cmc's call to mdm_init.
                 * So must use smd_sendMessageByState to deliver the message, which
                 * will either queue or send the message depending on the app's state.
                 */
                smd_sendMessageByState(dInfo, &msg);
            }

            /* see comment above */
            if (dInfo->state == DLS_NOT_RUNNING)
            {
                smd_launchApp(dInfo);
            }
            else if (dInfo->state == DLS_TERMINATE_REQUESTED)
            {
                /* Hmm, this is a weird situation.  Should we start another copy now? */
                vosLog_error("app in terminate requested state.  launching app again?");
                smd_launchApp(dInfo);
            }
        }
    }

    return;
}


VOS_RET_E SMD_eventInit(void)
{
    UINT32 stage = 1;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    FD_ZERO(&g_readFdsMaster);

    /*
     * initialize my own Unix domain listening socket for other apps to connect
     * to me.
     */
    if ((sg_ipcListenFd = smd_initUnixDomainServerSocket()) < 0)
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    FD_SET(sg_ipcListenFd, &g_readFdsMaster);
    SMD_UPDATE_MAXFD(sg_ipcListenFd);
#if 0
    /*
     * Initialize dynamic launch entries for the stage 1 apps.
     */
    if ((ret = smd_initDls(stage)) != VOS_RET_SUCCESS)
    {
        close(sg_ipcListenFd);
        return ret;
    }
#endif
    return VOS_RET_SUCCESS;
}   /* End of SMD_eventInit() */


static VOS_RET_E smd_init(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
#if 0
    if ((ret = SMD_systemInit()) != VOS_RET_SUCCESS)
    {
        return ret;
    }

    if ((ret = SMD_prmgrInit()) != VOS_RET_SUCCESS)
    {
        return ret;
    }
#endif

    if ((ret = SMD_eventInit()) != VOS_RET_SUCCESS)
    {
        return ret;
    }

    if ((ret = smd_lockInit(FALSE)) != VOS_RET_SUCCESS)
    {
        return ret;
    }

#ifdef SUPPORT_DEBUG_TOOLS
    if ((ret = SMD_sysmonInit()) != VOS_RET_SUCCESS)
    {
        return ret;
    }
#endif
#if 0
    /*
     * Start the first stage of launch on boot.  In the
     * first stage, we only launch ssk, which will initialize
     * the MDM.  Once that happens, smd will get an event msg,
     * which allows it execute the second stage of launchOnBoot.
     */
    

    SMD_schedWatchdog();
#endif
    SMD_launchOnBoot(2);
    vosLog_notice("done, ret=%d", ret);

    return ret;
}

static SMD_DLS_INFO_T *smd_getDlsInfoByServerFd(SINT32 serverFd)
{
    SMD_DLS_INFO_T *dInfo;

    dlist_for_each_entry(dInfo, &sg_dlsInfoHead, dlist)
    {
        if (dInfo->serverFd == serverFd)
        {
            return dInfo;
        }
    }

    return NULL;
}

static SMD_DLS_INFO_T *smd_getDlsInfoByCommFd(SINT32 commFd)
{
    SMD_DLS_INFO_T *dInfo;

    dlist_for_each_entry(dInfo, &sg_dlsInfoHead, dlist)
    {
        if (dInfo->commFd == commFd)
        {
            return dInfo;
        }
    }

    return NULL;
}

static SMD_DLS_INFO_T *smd_getExistingDlsInfo(VosEntityId eid)
{
    SMD_DLS_INFO_T *dInfo;

    dlist_for_each_entry(dInfo, &sg_dlsInfoHead, dlist)
    {
        if (NULL == dInfo->eInfo)
        {
            vosLog_error("NULL == dInfo->eInfo");
            continue;
        }

        if (dInfo->eInfo->flags & EIF_MULTIPLE_INSTANCES)
        {
            /*
             * This entity can have multiple instances, so must 
             * match on the specificEid field (which includes the pid).
             */
            if (dInfo->specificEid == eid)
            {
                return dInfo;
            }
        }
        else
        {
            if (dInfo->eInfo->eid == eid)
            {
                return dInfo;
            }
        }
    }

    return NULL;
}


/** A common send response function used by various processXXX msg functions.
 *
 * Sends a reply using the same msg buffer that held the request and frees
 * the msg buffer.
 */
static void smd_processMsgSimpleFooter(SMD_DLS_INFO_T * dInfo, VOS_RET_E rv, VosMsgHeader ** msg)
{
    VOS_RET_E ret;
    UINT32 tmpSrc;

    tmpSrc = (*msg)->src;
    (*msg)->src = (*msg)->dst;
    (*msg)->dst = tmpSrc;

    (*msg)->flags_request = 0;
    (*msg)->flags_response = 1;
    (*msg)->dataLength = 0;
    (*msg)->wordData = rv;

    ret = oalVosMsg_send(dInfo->commFd, (*msg));
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("send response for msg 0x%x failed, ret=%d", (*msg)->type, ret);
    }
    else
    {
        vosLog_debug("sent response for msg 0x%x dst=%d data=0x%x",
                     (*msg)->type, (*msg)->dst, (*msg)->wordData);
    }

    return;
}


/***************************************************************************
 * Function:
 *    static SINT32 initSocket(SINT32 side, SINT32 port, SINT32 type, SINT32 backlog)
 * Description:
 *    This function creates and initializes a TCP or UDP listening socket
 *    for an application.
 * Parameters:
 *    side     (IN) specifies whether it is a client-side socket or 
 *                  server-side socket.
 *    port     (IN) the application TCP or UDP port.
 *    type     (IN) the socket type, either SOCK_STREAM or SOCK_DGRAM.
 *    backlog  (IN) number of connections to queue. 
 * Returns:
 *    the socket file descriptor
 ***************************************************************************/
int smd_initInetServerSocket(SINT32 domain, SINT32 port, SINT32 type, SINT32 backlog)
{
    int sFd;
    SINT32 optVal;

    /* Create a TCP or UDP based socket */
    if ((sFd = socket(domain, type, 0)) < 0)
    {
        vosLog_error("socket errno=%d port=%d", errno, port);
        return UTIL_INVALID_FD;
    }

    /* Set socket options */
    optVal = 1;
    if (setsockopt(sFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0)
    {
        vosLog_error("setsockopt errno=%d port=%d fd=%d", errno, port, sFd);
        close(sFd);
        return UTIL_INVALID_FD;
    }

    /* Set up the local address */
    if (domain == AF_INET)
    {
        struct sockaddr_in serverAddr;

        if (type == SOCK_DGRAM)
        {
            /* set option for getting the to ip address. */
            if (setsockopt(sFd, IPPROTO_IP, IP_PKTINFO, &optVal, sizeof(optVal)) < 0)
            {
                vosLog_error("setsockopt errno=%d port=%d fd=%d", errno, port, sFd);
                close(sFd);
                return UTIL_INVALID_FD;
            }
        }

        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        /* Bind socket to local address */
        if (bind(sFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            vosLog_error("bind errno=%d port=%d fd=%d", errno, port, sFd);
            close(sFd);
            return UTIL_INVALID_FD;
        }
    }
    else
    {
        struct sockaddr_in6 serverAddr;

        if (type == SOCK_DGRAM)
        {
            /* set option for getting the to ip address. */
#ifdef IPV6_RECVPKTINFO
            if (setsockopt(sFd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &optVal, sizeof(optVal)) < 0)
#else
            if (setsockopt(sFd, IPPROTO_IPV6, IPV6_PKTINFO, &optVal, sizeof(optVal)) < 0)
#endif
            {
                vosLog_error("setsockopt errno=%d port=%d fd=%d", errno, port, sFd);
                close(sFd);
                return UTIL_INVALID_FD;
            }
        }

        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_port = htons(port);
        serverAddr.sin6_addr = in6addr_any;

        /* Bind socket to local address */
        if (bind(sFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            vosLog_error("bind errno=%d port=%d fd=%d", errno, port, sFd);
            close(sFd);
            return UTIL_INVALID_FD;
        }
    }

    if (type == SOCK_STREAM)
    {
        /* Enable connection to SOCK_STREAM socket */
        if (listen(sFd, backlog) < 0)
        {
            vosLog_error("listen errno=%d port=%d fd=%d", errno, port, sFd);
            close(sFd);
            return UTIL_INVALID_FD;
        }
    }

    return (sFd);
}



/** Allocate a new SMD_DLS_INFO_T entry for the specified eid, initialize its
 *  fields, including creating server sockets if appropriate, and link it
 *  into the dlsInfo dlist.
 */
static SMD_DLS_INFO_T *smd_insertDlsInfoEntry(VosEntityId eid)
{
    int socketType;
    SMD_DLS_INFO_T *dInfo;

    vosLog_debug("eid=%d (0x%x)", eid, eid);

    if ((dInfo = VOS_MALLOC_FLAGS(sizeof(SMD_DLS_INFO_T), ALLOC_ZEROIZE)) == NULL)
    {
        vosLog_error("could not allocate SMD_DLS_INFO_T for eid=%d", eid);
        return NULL;
    }

    dInfo->eInfo = vosEid_getEntityInfo(GENERIC_EID(eid));
    if (dInfo->eInfo == NULL)
    {
        vosLog_error("unrecognized eid %d", eid);
        VOS_FREE(dInfo);
        return NULL;
    }

    dInfo->state = DLS_NOT_RUNNING;
    dInfo->commFd = UTIL_INVALID_FD;
    dInfo->serverFd = UTIL_INVALID_FD;
    dInfo->pid = UTIL_INVALID_PID;

    if (eid == EID_CONSOLED)
    {
        /* consoled is a special case, it monitors stdin at fd=0 */
        dInfo->serverFd = 0;
        vosLog_debug("consoled monitoring fd 0");
        FD_SET(0, &g_readFdsMaster);
    }
    else if (dInfo->eInfo->flags & EIF_SERVER)
    {
        SINT32 domain;

        socketType = (dInfo->eInfo->flags & EIF_SERVER_TCP) ? SOCK_STREAM : SOCK_DGRAM;
        domain = (dInfo->eInfo->flags & EIF_IPV6) ? AF_INET6 : AF_INET;

        dInfo->serverFd = smd_initInetServerSocket(domain,
                                               dInfo->eInfo->port,
                                               socketType,
                                               dInfo->eInfo->backLog);
        if (dInfo->serverFd == UTIL_INVALID_FD)
        {
            VOS_FREE(dInfo);
            return NULL;
        }
        else
        {
            vosLog_debug("server socket for %s opened at port %d fd=%d",
                         dInfo->eInfo->name,
                         dInfo->eInfo->port,
                         dInfo->serverFd);

            FD_SET(dInfo->serverFd, &g_readFdsMaster);
            SMD_UPDATE_MAXFD(dInfo->serverFd);
        }
    }

    /* prepending to the head is equivalent to append to tail */
    dlist_prepend((DlistNode *) dInfo, &sg_dlsInfoHead);

    return dInfo;
}


static VOS_RET_E smd_initDls(UINT32 stage)
{
    SMD_DLS_INFO_T *dInfo = NULL;
    const VosEntityInfo *eInfo = NULL;
    UINT32 numEntries, i = 0;

    /*
     * Populate dlsInfo dlist in stages because a side effect of adding
     * an entry in the dlist is that we start monitoring the server fd
     * (if the app has one).  In stage 1, we only want to launch
     * ssk and let ssk finish initializing the MDM.  Once that is complete,
     * we can start monitoring server fd's and accepting connections from
     * other apps.  (This fixes a very small race condition where httpd may
     * connect to smd before ssk has finished initializing the MDM.)
     *
     * All apps which have server fd's or which are to be launched on boot
     * must be put into this list in this function.  Apps which are launched
     * via a request from the rcl handler functions will get an entry put in
     * for them on an as needed basis by smd.
     */


    /*
     * Go through the entire entityInfoArray from the first entity,
     * and look for entities that have the LAUNCH_IN_STAGE_1 flag set.
     * Doing it this way technically violates the information hiding of the
     * entityInfoArray, but now users do not have to modify this code when
     * they add a new entity.
     */
    eInfo = vosEid_getFirstEntityInfo();
    numEntries = vosEid_getNumberOfEntityInfoEntries();

    if (stage == 1)
    {
        for (i = 0; i < numEntries; i++)
        {
            if (eInfo->isFeatureCompiledIn && (eInfo->flags & EIF_LAUNCH_IN_STAGE_1))
            {
                vosLog_debug("inserting stage 1 entity: %s (%d)", eInfo->name, eInfo->eid);
                dInfo = smd_insertDlsInfoEntry(eInfo->eid);
                if (dInfo == NULL)
                {
                    vosLog_error("failed to initialize dInfo for %s", eInfo->name);
                    return VOS_RET_INTERNAL_ERROR;
                }
            }

            eInfo++;
        }
    }
    else
    {
        /* initialize stage 2 apps (which include the launch-on-boot apps) */
        for (i = 0; i < numEntries; i++)
        {
            if ((eInfo->isFeatureCompiledIn) &&
                ((eInfo->flags & (EIF_LAUNCH_ON_BOOT|EIF_SERVER)) ||
                 (eInfo->eid == EID_CONSOLED)))
            {
                if ((dInfo = smd_getExistingDlsInfo(eInfo->eid)) != NULL)
                {
                    /*
                     * A server app, such as tr69c, could have been launched
                     * during ssk initialization of the MDM.  So if there is
                     * a dInfo structure already, don't insert it again.
                     */
                    vosLog_debug("dInfo entry for %s already inserted", eInfo->name);
                }
                else
                {
                    vosLog_debug("inserting stage 2 entity: %s (%d)", eInfo->name, eInfo->eid);

                    dInfo = smd_insertDlsInfoEntry(eInfo->eid);
                    if (dInfo == NULL)
                    {
                        vosLog_error("failed to initialize dInfo for %s", eInfo->name);
                        return VOS_RET_INTERNAL_ERROR;
                    }
                }
            }

            eInfo++;
        }

    }   /* end of if (stage == 2) */

    return VOS_RET_SUCCESS;
}


static SMD_DLS_INFO_T *smd_getNewDlsInfo(VosEntityId eid)
{
    vosLog_notice("Creating dynamically allocated dlsInfo entry for eid=0x%x", eid);
    return (smd_insertDlsInfoEntry(eid));
}


static SMD_DLS_INFO_T *smd_getDlsInfo(VosEntityId eid)
{
    SMD_DLS_INFO_T *dInfo;

    if ((dInfo = smd_getExistingDlsInfo(eid)) != NULL)
    {
        return dInfo;
    }

    /*
     * this eid does not have an entry created at startup.  No problem.
     * create one now and add it to the list.
     */
    return smd_getNewDlsInfo(eid);
}


/** Queue the message onto the dInfo structure of the specified app.
 *
 * @param dInfo (IN)  The dInfo structure for the receiving app.
 * @param msg   (IN)  The message to be queued.  Since the message is
 *                    queued for later delivery, this function
 *                    will take ownership of this message. So the caller
 *                    must pass in a VOS_MALLOC_FLAGS'd buffer, not one from
 *                    the stack.  On return, the caller must not touch
 *                    or free this message.
 *
 */
static void smd_queueMsg(SMD_DLS_INFO_T * dInfo, VosMsgHeader ** msg)
{

    VosMsgHeader *msgHeader = dInfo->msgQueue;

    (*msg)->next = NULL;

    if (msgHeader == NULL)
    {
        /* This is the first msg in the queue */
        dInfo->msgQueue = (*msg);
    }
    else
    {
        /* Need to apppend msg to the end of the queue */
        while (msgHeader->next != NULL)
        {
            msgHeader = msgHeader->next;
        }
        msgHeader->next = (*msg);
    }

    *msg = NULL;

    return;
}



/** Send a message to the given dInfo structure.  
 *
 * How we send the message, and what actions we take are based on
 * the state of the receiving app, as indicated in the dInfo structure.
 *
 * @param dInfo (IN)  The dInfo structure for the receiving app.
 * @param msg   (IN)  The message that needs to be sent.  This function
 *                    will take ownership of this message, either queueing
 *                    it for freeing it.  So the caller must pass in a 
 *                    VOS_MALLOC_FLAGS'd buffer, not one from the stack.
 *
 */
void smd_sendMessageByState(SMD_DLS_INFO_T * dInfo, VosMsgHeader ** msg)
{

    if (NULL == dInfo->eInfo)
    {
        vosLog_error("NULL == dInfo->eInfo");
        VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
        return;
    }

    /* mwang_todo: this looks a lot like logic in processEventMessage and routemessage
     * and smd_distributeEventMessage, consolidate */
    switch (dInfo->state)
    {
    case DLS_NOT_RUNNING:
        if (EID_TR69C == dInfo->eInfo->eid)
        {
            vosLog_notice("tr69c been closed");
            VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
        }
        else
        {
            if (dInfo->eInfo->isFeatureCompiledIn)
            {
                char cmd[128] = {0};
                snprintf(cmd, sizeof(cmd), "%s&", dInfo->eInfo->path);
                
                vosLog_notice("launching %s to receive msg 0x%x",
                         dInfo->eInfo->name, (*msg)->type);
                smd_queueMsg(dInfo, msg);
                system(cmd);
            }
            else
            {
                vosLog_error("launching %s is not support", dInfo->eInfo->name);
                VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
            }
        }
        break;

    case DLS_LAUNCHED:
        vosLog_notice("%s already launched but waiting for confirm, just queue msg 0x%x",
                      dInfo->eInfo->name, (*msg)->type);
        smd_queueMsg(dInfo, msg);
        break;

    case DLS_RUNNING:
        vosLog_notice("%s is already running, send message 0x%x now",
                      dInfo->eInfo->name, (*msg)->type);
#ifdef DESKTOP_LINUX
        if (dInfo->eInfo->flags & EIF_DESKTOP_LINUX_CAPABLE)
        {
            /*
             * only send the msg if the app is desktop capable.  otherwise it is
             * not actually running.
             */
            oalVosMsg_send(dInfo->commFd, *msg);
        }
#else
        oalVosMsg_send(dInfo->commFd, *msg);
#endif
        VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
        break;

    case DLS_TERMINATE_REQUESTED:
        /* are we shutting down the system? No need to send event messages?
         * mwang_todo: No, actually, we should use an "ACK" mechanism to make sure
         * that the app receives the event message.  It could be something
         * important that the app needs to act upon. */
        vosLog_error("%s is in terminate requested state, cannot deliver msg 0x%x lost!",
                     dInfo->eInfo->name, (*msg)->type);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
        break;

    default:
        vosLog_error("%s is unknown state %u, cannot send message 0x%x",
                      dInfo->eInfo->name, dInfo->state, (*msg)->type);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
        break;
    }

    return;
}



/** Send a request/response message to another process.
 *
 * @param msg (IN) Message to route.  This function will steal the message
 *                 from the caller.  Caller does not need to deal with/free
 *                 the message after this.
 *
 */
static void smd_routeMessage(VosMsgHeader ** msg)
{
    VosEntityId dstEid = (*msg)->dst;
    SMD_DLS_INFO_T *dstDInfo;

    vosLog_notice("routing msg 0x%x from 0x%x to 0x%x (req=%d resp=%d event=%d)",
                  (*msg)->type, (*msg)->src, (*msg)->dst,
                  (*msg)->flags_request, (*msg)->flags_response, (*msg)->flags_event);


    dstDInfo = smd_getExistingDlsInfo(dstEid);
    if ((*msg)->flags_bounceIfNotRunning)
    {
        if ((dstDInfo == NULL) ||
            (dstDInfo->state == DLS_NOT_RUNNING) ||
            (dstDInfo->state == DLS_TERMINATE_REQUESTED))
        {
            /* sender does not want us to launch the app if not running */
            if ((*msg)->flags_event)
            {
                /*
                 * The bounceIfNotRunning bit and the event bit are both set,
                 * so sender does not want us to launch the app, and also does not
                 * expect a response.  So just drop the message.
                 */
                vosLog_notice("bouncing event msg by dropping it");
                VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
            }
            else
            {
                SMD_DLS_INFO_T *srcDinfo = smd_getExistingDlsInfo((*msg)->src);

                if (srcDinfo)
                {
                    vosLog_notice("bouncing msg to eid=0x%x back to %s",
                                  dstEid,
                                  srcDinfo->eInfo ? srcDinfo->eInfo->name : "");
                    smd_processMsgSimpleFooter(srcDinfo, VOS_RET_MSG_BOUNCED, msg);
                }
                else
                {
                    vosLog_error("bouncing msg to eid=0x%x from 0x%x",
                                 dstEid, (*msg)->src);
                }
            }
            return;
        }
    }


    if (dstDInfo == NULL)
    {
        /*
         * we might need to dynamically create a dInfo for this msg dst.
         * This is the "launch app to receive message case".
         */
        if ((dstDInfo = smd_getDlsInfo(dstEid)) == NULL)
        {
            vosLog_error("Cannot find dest eid %d, drop msg", dstEid);
            VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
            return;
        }

        if (NULL == dstDInfo->eInfo)
        {
            vosLog_error("NULL == dstDInfo->eInfo");
            VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
            return;
        }

        /* We successfully created a new dlsInfo struct, but
         * does it make sense to launch a multiple instance app to receive a msg?
         * print a warning if this condition is detected.
         */
        if (dstDInfo->eInfo->flags & EIF_MULTIPLE_INSTANCES)
        {
            vosLog_error("launching multiple instance app (dst=0x%x) to receive msg (src=0x%x, msgType=0x%x)", dstEid, (*msg)->src, (*msg)->type);
            if (dstDInfo->eInfo->eid == EID_PPP)
            {
                vosLog_error("This is a staled pppd message and just ignore it so that pppd will not be lanched in smd_sendMessageByState below");
            }
            if ((*msg)->flags_request)
            {
                SMD_DLS_INFO_T *srcDinfo = smd_getExistingDlsInfo((*msg)->src);

                smd_processMsgSimpleFooter(srcDinfo, VOS_RET_NO_MORE_INSTANCES, msg);
            }
            else
            {
                VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
            }
            dlist_unlink((DlistNode *) (dstDInfo));
            VOS_MEM_FREE_BUF_AND_NULL_PTR((dstDInfo));
            return;
        }
    }

    smd_sendMessageByState(dstDInfo, msg);

    return;
}


SMD_EVT_INTEREST_T *SMD_interestFindType(VosMsgType msgType)
{
    SMD_EVT_INTEREST_T *evtInterest;

    dlist_for_each_entry(evtInterest, &sg_evtInterestHead, dlist)
    {
        if (evtInterest->type == msgType)
        {
            return evtInterest;
        }
    }

    return NULL;
}



/** Given an event msg, send a copy of this event message to
 *  all apps which have registered an interest in this event msg.
 *
 * We do not send the same msg to the creator/sender of the initial event msg.
 *
 * @param msg      (IN) The received event msg.  This function will distribute
 *                      a copy of this msg to all interested parties and
 *                      also free the msg at the end of this function.
 * @param srcDInfo (IN) The sender of the msg.  srcDinfo may be NULL if
 *                      the event was generated by smd itself.
 */
static void smd_distributeEventMessage(VosMsgHeader ** msg, const SMD_DLS_INFO_T * srcDInfo)
{
    VosEntityId eventSrc;
    SMD_DLS_INFO_T *targetDInfo;
    SMD_EVT_INTEREST_T *evtInterest;
    SMD_EVT_INTEREST_INFO_T *evtInfo;
    VosMsgHeader *newMsg = NULL;

    if (srcDInfo != NULL)
    {
        if (NULL == srcDInfo->eInfo)
        {
            vosLog_error("NULL == dInfo->eInfo");
            VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);
            return;
        }

        vosLog_notice("eventType=0x%x from %s", (*msg)->type, srcDInfo->eInfo->name);
        eventSrc = (*msg)->src;
    }
    else
    {
        vosLog_notice("eventType=0x%x from smd", (*msg)->type);
        eventSrc = EID_SMD;
    }


    /*
     * Look up evtInterest for this event msg type
     * loop through all interested parties,
     *  --if they are currently running, send it to them,
     *  --if they are not currently running, queue the message and launch them.
     * 
     */
    evtInterest = SMD_interestFindType((*msg)->type);
    if (evtInterest != NULL)
    {
        dlist_for_each_entry(evtInfo, &(evtInterest->evtInfoHead), dlist)
        {
            targetDInfo = smd_getDlsInfo(evtInfo->eid);

            if (targetDInfo == NULL)
            {
                vosLog_error("cannot find targetDInfo for eid 0x%x, skip it", evtInfo->eid);
            }
            else if ((srcDInfo != NULL) && (targetDInfo == srcDInfo))
            {
                /* source and destination dInfo match, don't send event back to sender */
                /* weird, why would an app register for an event, but then send out that event?
                 * Is this an error, or should I actually send the event to the sender anyways?
                 */
                vosLog_debug("targetDinfo and srcDinfo are the same, don't send");
            }
            else if (evtInfo->matchData && (((*msg)->dataLength == 0) ||
                                            util_strcmp(evtInfo->matchData, ((char *) ((*msg)+1)))))
            {
                /* interested party specified additional match data, which this event did not match */
                vosLog_debug("no match for eid 0x%x matchData=%s", evtInfo->eid, evtInfo->matchData);
                vosLog_debug("src event dataLen=%d", (*msg)->dataLength);
                if ((*msg)->dataLength > 0)
                {
                    vosLog_debug("src data=%s", (char *)((*msg) + 1));
                }
            }
            else
            {
                if (NULL == targetDInfo->eInfo)
                {
                    vosLog_error("NULL == targetDInfo->eInfo");
                    continue;
                }

                /* OK, we can send this event */
                vosLog_debug("dup and send event msg");
                newMsg = vosMsg_duplicate(*msg);
                if (newMsg == NULL)
                {
                    vosLog_error("could not allocate msg, dropping event");
                }
                else
                {
                    newMsg->src = eventSrc;
                    newMsg->dst = (targetDInfo->eInfo->flags & EIF_MULTIPLE_INSTANCES) ?
                                   targetDInfo->specificEid : targetDInfo->eInfo->eid;

                    smd_sendMessageByState(targetDInfo, &newMsg);
                }
            }
        }
    }

    VOS_MEM_FREE_BUF_AND_NULL_PTR(*msg);

    return;
}

void smd_procGetLockRequest(SMD_DLS_INFO_T * dInfo, VosMsgHeader **msg)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT32 timeout = 3000;
    ret = smd_Lock(&timeout, dInfo); 
    smd_processMsgSimpleFooter(dInfo, ret, msg);
}


void smd_procReleaseLockRequest(SMD_DLS_INFO_T * dInfo, VosMsgHeader **msg)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ret = smd_Unlock(); 
    smd_processMsgSimpleFooter(dInfo, ret, msg);
}


/** Process a message received on the communications link.
 *
 * @param dInfo (IN) The SMD_DLS_INFO_T struct for the sender of the message.
 * @param msg   (IN) The received message.
 * 
 */
static void smd_processMessage(SMD_DLS_INFO_T * dInfo, VosMsgHeader ** msg)
{
    vosLog_notice("Got msg type 0x%x src=0x%x dst=0x%x",
                  (*msg)->type, (*msg)->src, (*msg)->dst);

    /*
     * Before doing normal processing, check for the requeue bit.
     * This allows an app to requeue a message it has received
     * back into its kernel socket queue.  Used as part of vosMsg_putBack
     * and vosMsg_requeuePutBack.
     * Instead of doing a special case here, I could let normal processing
     * occur and this message would end up in smd_routeMessage.  But what if
     * the message I'm requeuing is a TERMINATE_REQUEST msg?  Will it 
     * interfere with special case processing for DLS_TERMINATE_REQUESTED?
     * Revisit this issue again once I get the ACK mechanism worked out.
     */
    if ((*msg)->flags_requeue == 1)
    {
        vosLog_debug("requeueing msg type 0x%x", (*msg)->type);
        (*msg)->flags_requeue = 0;
        oalVosMsg_send(dInfo->commFd, *msg);
        return;
    }


    if ((*msg)->dst == EID_SMD)
    {
        if ((*msg)->flags_event)
        {
            /* this is an event message sent to smd */
            if ((*msg)->type == VOS_MSG_MDM_INITIALIZED)
            {
                UINT32 stage = 2;
                VOS_RET_E ret;

                if ((ret = smd_initDls(stage)) != VOS_RET_SUCCESS)
                {
                    vosLog_error("initDls failed, ret=%d, smd must exit now", ret);
                    g_keepLooping = FALSE;
                }

                SMD_launchOnBoot(stage);
            }
            else
            {
                smd_distributeEventMessage(msg, dInfo);
            }
        }
        else
        {
        
            /* this is a request message sent to smd */
            switch ((*msg)->type)
            {
            case VOS_MSG_GET_LOCK:
                smd_procGetLockRequest(dInfo, msg);
                break;

            case VOS_MSG_RELEASE_LOCK:
                smd_procReleaseLockRequest(dInfo, msg);
                break;
            #if 0
            case VOS_MSG_REGISTER_EVENT_INTEREST:
                smd_processRegisterEventInterest(dInfo, msg, TRUE);
                break;

            case VOS_MSG_UNREGISTER_EVENT_INTEREST:
                smd_processRegisterEventInterest(dInfo, msg, FALSE);
                break;

            case VOS_MSG_REGISTER_DELAYED_MSG:
                smd_processRegisterDelayedMsg(dInfo, msg, TRUE);
                break;

            case VOS_MSG_UNREGISTER_DELAYED_MSG:
                smd_processRegisterDelayedMsg(dInfo, msg, FALSE);
                break;

            case VOS_MSG_REBOOT_SYSTEM:
                smd_processRebootMsg(dInfo, msg);
                break;

                /*
                 * Reboot calls the busybox reboot function, which kills all
                 * all processes and uses an ioctl to reset the MIPS.
                 * Terminate tells smd to kill all processes, exit gracefully,
                 * giving control to the initial shell so that update actions can
                 * occur.
                 */
            case VOS_MSG_TERMINATE:
                smd_processTerminateMsg(dInfo, msg);
                break;

            case VOS_MSG_START_APP:
                smd_processStartAppMsg(dInfo, msg);
                break;

            case VOS_MSG_RESTART_APP:
                smd_processRestartAppMsg(dInfo, msg);
                break;

            case VOS_MSG_STOP_APP:
                smd_processStopAppMsg(dInfo, msg);
                break;

            case VOS_MSG_LOAD_IMAGE_STARTING:
                smd_processLoadImageStarting(dInfo, msg);
                break;

            case VOS_MSG_LOAD_IMAGE_DONE:
                smd_processLoadImageDone(dInfo, msg);
                break;

            case VOS_MSG_SET_LOG_LEVEL:
                vosLog_setLevel((*msg)->wordData);
                break;

            case VOS_MSG_SET_LOG_DESTINATION:
                vosLog_setDestination((*msg)->wordData);
                break;

            case VOS_MSG_IS_APP_RUNNING:
                smd_processIsAppRuning(dInfo, msg);
                break;

            case VOS_MSG_MDM_INIT_STATE:
                smd_proccessMdmInitState(msg);
                break;

            case VOS_MSG_ADD_ENTITY_INFO:
                smd_processAddEntityInfo(dInfo, msg);
                break;

            case VOS_MSG_DEL_ENTITY_INFO:
                smd_processDelEntityInfo(dInfo, msg);
                break;

            case VOS_MSG_GET_EID_FROM_NAME:
                smd_proccessGetEidFromName(dInfo, msg);
                break;

            case VOS_MSG_CLOSE_SERVER_LISTEN:
                smd_processCloseServerListen(dInfo, msg);
                break;

            case VOS_MSG_WATCHDOG_HEARTBEAT:
                dInfo->heartbeat = HEARTBEAT_INIT;
                break;

            case VOS_MSG_WATCHDOG_STOP:
                dInfo->heartbeat = HEARTBEAT_STOP;
                break;    

            case VOS_MSG_CLI_SET_WATCHDOG:
                sg_enableWatchdog = (UBOOL8)(*msg)->wordData;
                SMD_watchdog();
                break;

            case VOS_MSG_SET_WAN_CONN_ID:
                vosLog_debug("smd received VOS_MSG_SET_WAN_CONN_ID msg");
                break;

            case VOS_MSG_SET_SERVER_PORT:
                smd_processSetServerPort(msg);
                break;
            #endif    
            default:
                vosLog_error("cannot handle request type 0x%x", (*msg)->type);
                smd_processMsgSimpleFooter(dInfo, VOS_RET_METHOD_NOT_SUPPORTED, msg);
            }
            
        }
    }
    else
    {
        /* this is a message intended for some other app. */
        smd_routeMessage(msg);
    }

    return;
}


/** Verify that this message is a good APP_LAUNCHED message
 *
 * @return dlsInfo if the message is good.
 */
static SMD_DLS_INFO_T *smd_validateAppLaunchedMsg(const VosMsgHeader * msg)
{
    SMD_DLS_INFO_T *dInfo;
    UBOOL8 isNewDinfo;

    /* this must be the first message */
    if (msg->type != VOS_MSG_APP_LAUNCHED || msg->dst != EID_SMD)
    {
        vosLog_error("wrong message type or dest eid");
        return NULL;
    }

    /*
     * smd_getDlsInfo will create a new dlsInfo entry for this eid if
     * an entry for it does not exist.  This feature is useful
     * if a process is launched by something other than smd,
     * e.g. unittests or command line.
     */
    vosLog_debug("new connection from src=0x%x", msg->src);
    if (NULL == (dInfo = smd_getDlsInfo(msg->src)))
    {
        vosLog_error("Could not get SMD_DLS_INFO_T struct for eid=%d", msg->src);
        return NULL;
    }

    if (NULL == dInfo->eInfo)
    {
        vosLog_error("NULL == dInfo->eInfo");
        return NULL;
    }

    isNewDinfo = (dInfo->pid == UTIL_INVALID_PID);

    /* next block is expected pid check */
    if (dInfo->eInfo->flags & EIF_MULTIPLE_INSTANCES)
    {
        /*
         * I cannot think of a way to verify this case.
         * I do not know if the app was launched by smd or not.  If not
         * launched by smd, then this will be a new dInfo entry with no info
         * for me to check against.  But if it was
         * launched by smd, but with an unexpected pid or specific eid, then
         * it will still look like a new dInfo entry with no info for me to
         * check against.
         */
        dInfo->specificEid = MAKE_SPECIFIC_EID(msg->wordData, msg->src);
    }
    else
    {
        if (isNewDinfo)
        {
            /*
             * This app was not launched by smd, so I don't know what pid
             * to expect.  No checking can be done here.
             */
        }
        else
        {
            /*
             * This app was launched by smd, so we should have an existing
             * dInfo entry and the pid should match.
             */
            if (msg->wordData != dInfo->pid)
            {
                vosLog_notice("expected pid %d but got %d for %s, ignore this one",
                              dInfo->pid, msg->wordData, dInfo->eInfo->name);
                return NULL;
            }
        }
    }

    /* We should not already have a commfd to this app */
    if (dInfo->commFd != UTIL_INVALID_FD)
    {
        vosLog_error("dInfo for %s already has a commFd %d, ignore this one",
                     dInfo->eInfo->name, dInfo->commFd);
        return NULL;
    }

    return dInfo;
}


/** Now that an app has confirmed its launch, send any queued messages we have
 *  waiting for it.
 */
static void smd_processLaunchConfirmation(SMD_DLS_INFO_T * dInfo)
{
    VosMsgHeader *msg;

    if (NULL == dInfo->eInfo)
    {
        vosLog_error("NULL == dInfo->eInfo");
        return;
    }

    dInfo->state = DLS_RUNNING;
    vosLog_debug("%s (eid=%d) transitioning to state=%d",
                 dInfo->eInfo->name, dInfo->eInfo->eid, dInfo->state);

    /* start monitoring this fd */
    FD_SET(dInfo->commFd, &g_readFdsMaster);
    if ((dInfo->eInfo->flags & EIF_SERVER) ||
        (dInfo->eInfo->eid == EID_CONSOLED))
    {
        FD_CLR(dInfo->serverFd, &g_readFdsMaster);
    }
    SMD_UPDATE_MAXFD(dInfo->commFd);

    while ((msg = dInfo->msgQueue) != NULL)
    {
        dInfo->msgQueue = dInfo->msgQueue->next;
        msg->next = NULL;
        vosLog_debug("sending queued msg 0x%x", msg->type);
        oalVosMsg_send(dInfo->commFd, msg);
        VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
    }
}


static void smd_collectApp(SMD_DLS_INFO_T ** dInfo)
{
    CollectProcessInfo collectInfo;
    SpawnedProcessInfo procInfo;
    VosMsgHeader *msg;
    VOS_RET_E ret;


    if (NULL == (*dInfo)->eInfo)
    {
        vosLog_error("NULL == (*dInfo)->eInfo");
        return;
    }

    /*
     * Free any messages that are queued up for this app before
     * we (potentially) free the dInfo struct.
     */
    while ((msg = (*dInfo)->msgQueue) != NULL)
    {
        (*dInfo)->msgQueue = (*dInfo)->msgQueue->next;
        msg->next = NULL;
        VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
    }


    if ((*dInfo)->commFd != UTIL_INVALID_FD)
    {
        FD_CLR((*dInfo)->commFd, &g_readFdsMaster);
        close((*dInfo)->commFd);
        (*dInfo)->commFd = UTIL_INVALID_FD;
    }


    /*
     * Collect the process if smd was the one that launched it.
     * If the process started by itself (e.g. tftp), its pid will be 
     * UTIL_INVALID_PID, so smd should not bother collecting it.
     */
    if ((*dInfo)->pid == UTIL_INVALID_PID)
    {
        vosLog_debug("Do not collect %s, it was not launched by smd", (*dInfo)->eInfo->name);
        if ((*dInfo)->eInfo->flags & EIF_AUTO_RELAUNCH)
        {
            char cmd[128] = {0};
            snprintf(cmd, sizeof(cmd), "%s&", (*dInfo)->eInfo->path);
            system(cmd);

            vosLog_debug("Auto relaunch %s", (*dInfo)->eInfo->name);
            //smd_launchApp(*dInfo);
            return;
        }
    }
    else
    {
        collectInfo.collectMode = COLLECT_PID_TIMEOUT;
        collectInfo.pid = (*dInfo)->pid;
        collectInfo.timeout = 5 * MSECS_IN_SEC; /* wait at most 5 sec */


        if ((ret = prctl_collectProcess(&collectInfo, &procInfo)) != VOS_RET_SUCCESS)
        {
            vosLog_error("Could not collect %s (pid=%d timeout=%dms), ret=%d.  Kill it with SIGKILL.",
                         (*dInfo)->eInfo->name, (*dInfo)->pid, collectInfo.timeout, ret);
            /*
             * Send SIGKILL and collect the process, otherwise,
             * we end up with a zombie process.
             */
            prctl_signalProcess((*dInfo)->pid, SIGKILL);
            if ((ret = prctl_collectProcess(&collectInfo, &procInfo)) != VOS_RET_SUCCESS)
            {
                vosLog_error("Still could not collect %s (pid=%d) after SIGKILL, ret=%d",
                             (*dInfo)->eInfo->name, (*dInfo)->pid, ret);
                /* this process is really stuck.  Not much I can do now.
                 * Just leave it running I guess. */
            }
            else
            {
                vosLog_debug("collected %s (pid %d) after SIGKILL", (*dInfo)->eInfo->name, (*dInfo)->pid);
                /* need create and distribute  the app termicated message to event registered applications */
                //smd_createAppTerminatedMsg((*dInfo)->eInfo->eid, procInfo.signalNumber, procInfo.exitCode); //tongchao todo
            }
        }
        else
        {
            vosLog_debug("collected %s (pid %d) signalNum=%d", (*dInfo)->eInfo->name, (*dInfo)->pid, procInfo.signalNumber);

            /* need create and distribute  the app termicated message to event registered applications */
            //smd_createAppTerminatedMsg((*dInfo)->eInfo->eid, procInfo.signalNumber, procInfo.exitCode); //tongchao to do

            if (procInfo.signalNumber == SIGILL ||
                procInfo.signalNumber == SIGABRT ||
                procInfo.signalNumber == SIGFPE ||
                procInfo.signalNumber == SIGSEGV ||
                procInfo.signalNumber == SIGPIPE ||
                procInfo.signalNumber == SIGINT ||
                procInfo.signalNumber == SIGKILL ||
                procInfo.signalNumber == SIGBUS ||
                procInfo.signalNumber == SIGXCPU ||
                procInfo.signalNumber == SIGXFSZ)
            {
                vosLog_error("%s (pid %d) exited due to uncaught signal number %d", 
                             (*dInfo)->eInfo->name,
                             (*dInfo)->pid,
                             procInfo.signalNumber);

                if ((*dInfo)->eInfo->flags & EIF_AUTO_RELAUNCH)
                {
                    UTIL_DO_SYSTEM_ACTION("echo; echo -------------- Dump Diagnosis Info --------------");
                    UTIL_DO_SYSTEM_ACTION("echo; cat /show_version");
                    UTIL_DO_SYSTEM_ACTION("echo; logctl %s show", (*dInfo)->eInfo->name);
                    UTIL_DO_SYSTEM_ACTION("echo; ps");
                    UTIL_DO_SYSTEM_ACTION("echo; top -b -n 2 -d 2 > /tmp/top.S304; cat /tmp/top.S304; rm -f /tmp/top.S304");
                    UTIL_DO_SYSTEM_ACTION("echo; echo -------------------------------------------------");
                    UTIL_DO_SYSTEM_ACTION("echo");

                    vosLog_debug("Auto relaunch %s", (*dInfo)->eInfo->name);
                    smd_launchApp(*dInfo);
                    return;
                }
            }
        }
    }


    /* start monitoring server fd again (if this app is a server) */
    if (((*dInfo)->eInfo->flags & EIF_SERVER) ||
        ((*dInfo)->eInfo->eid == EID_CONSOLED))
    {
        FD_SET((*dInfo)->serverFd, &g_readFdsMaster);
    }

    /*
     * Free this dInfo if
     * - it does not monitor a server fd, including consoled, and
     * - it is for a multiple instance app or it cannot have multiple instances and it currently has no registration for event interests and delayed messages.
     *
     * Multiple instance apps cannot hold on to event interest or
     * delayed messages once they exit, so we always delete their
     * dInfo structures.
     */
    if ((((*dInfo)->eInfo->flags & EIF_SERVER) == 0) &&
        ((*dInfo)->eInfo->eid != EID_CONSOLED) &&
        (((*dInfo)->eInfo->flags & EIF_MULTIPLE_INSTANCES) ||
         ((((*dInfo)->eInfo->flags & EIF_MULTIPLE_INSTANCES) == 0) &&
          ((*dInfo)->numDelayedMsgRequested == 0) &&
          ((*dInfo)->numEventInterestRequested == 0))))
    {
        VosEntityId eid = ((*dInfo)->eInfo->flags & EIF_MULTIPLE_INSTANCES) ? (*dInfo)->specificEid : (*dInfo)->eInfo->eid;

        vosLog_debug("unlink and free dInfo structure at %p for %s eid=%d (0x%x)", (*dInfo), (*dInfo)->eInfo->name, eid, eid);
        //SMD_interestUnregisterAll(eid);  //tongchao todo
        dlist_unlink((DlistNode *) (*dInfo));
        VOS_MEM_FREE_BUF_AND_NULL_PTR((*dInfo));
    }
    else
    {
        /* we did not free dInfo, clear some key fields */
        (*dInfo)->state = DLS_NOT_RUNNING;
        (*dInfo)->pid = UTIL_INVALID_PID;
        (*dInfo)->specificEid = 0;
    }
#if 0
    if (SF_FEATURE_VOIP_TYPE_ENDPOINT)
    {
        /* Unregister interest in all events when vodsl shuts down */
        if (((*dInfo) != NULL) && ((*dInfo)->eInfo->eid == EID_VODSL))
        {
            SMD_interestUnregisterAll((*dInfo)->eInfo->eid);
        }
    }
#endif
    return;
}



/**   This function contains the main select loop for smd.
 *
 */
VOS_RET_E SMD_processEvents(void)
{
    struct timeval tm;
    fd_set readFds;
    SMD_DLS_INFO_T *dInfo;
    SINT32 i;
    const UINT32 sched_lag_time = 10;
    SINT32 rv;
    UtilTimestamp nowTs;
    int sleepMs = 10;

    /* set our bit masks according to the master */
    readFds = g_readFdsMaster;
    tm.tv_sec = sleepMs / MSECS_IN_SEC;
    tm.tv_usec = (sleepMs % MSECS_IN_SEC) * USECS_IN_MSEC;
    /* pend, waiting for one or more fds to become ready */
    rv = select(g_maxFd + 1, &readFds, NULL, NULL, &tm);
    if (rv < 0)
    {
        if (errno != EINTR)
        {
            vosLog_error("select returned %d errno=%d", rv, errno);
        }
        else
        {
            vosLog_debug("return success anyways");
        }

        /* return success anyways, the main loop will call us again. */
        return VOS_RET_SUCCESS;
    }
    else if (rv == 0)
    {
        /* timed out */
        //vosLog_debug("timeout");
        return VOS_RET_SUCCESS;
    }


    vosLog_debug("select success");

    /* step through fds array and act on fds that are ready */
    for (i = 0; i < g_maxFd + 1; i++)
    {
        if (!FD_ISSET(i, &readFds))
        {
            continue;
        }

        /* there is activity on fd 'i', now I need to figure out who owns fd 'i' */
        vosLog_debug("fd %d is set", i);


        /* check for launching apps based on activity on server socket */
        dInfo = smd_getDlsInfoByServerFd(i);
        vosLog_debug("dInfo = %p for server fd", dInfo);

        if (dInfo != NULL)
        {
            if (dInfo->eInfo)
            {
                if (dInfo->state != DLS_NOT_RUNNING)
                {
                    vosLog_error("got activity on server socket while app %s is in state %d",
                                 dInfo->eInfo->name, dInfo->state);
                }
                else
                {
                    /* launch the application process */
                    vosLog_debug("got activity on server socket %d for %s", dInfo->serverFd, dInfo->eInfo->name);
                    smd_launchApp(dInfo);
                }
            }
            else
            {
                vosLog_error("NULL == dInfo->eInfo");
            }
        }

        /* check for activity on the ipc comm sockets to the individual apps */
        dInfo = smd_getDlsInfoByCommFd(i);
        vosLog_debug("dInfo = %p for comm fd", dInfo);

        if (dInfo != NULL)
        {
            VosMsgHeader *msg;
            VOS_RET_E ret;

            if (dInfo->eInfo)
            {
                vosLog_notice("detected message on fd %d from %s", dInfo->commFd, dInfo->eInfo->name);

                ret = oalVosMsg_receive(dInfo->commFd, &msg, NULL);
                if (ret == VOS_RET_SUCCESS)
                {
                    smd_processMessage(dInfo, &msg);
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
                }
                else if (ret == VOS_RET_DISCONNECTED)
                {
                    vosLog_notice("detected exit of %s (pid=%d) on fd %d", dInfo->eInfo->name, dInfo->pid, dInfo->commFd);
                    smd_collectApp(&dInfo);

#ifdef DESKTOP_LINUX
                    if (EID_CONSOLED == dInfo->eInfo->eid)
                    {
                        return VOS_RET_OP_ABORTED_BY_USER;
                    }
#endif
                }
                else
                {
                    vosLog_error("error on read from fd %d for %s", dInfo->commFd, dInfo->eInfo->name);
                }
            }
            else
            {
                vosLog_error("NULL == dInfo->eInfo");
            }
        }

        /*
         * check for newly launched processes (either launched by smd or on
         * command line) connecting back to smd.
         */
        if (i == sg_ipcListenFd)
        {
            struct sockaddr_un clientAddr;
            UINT32 sockAddrSize;
            SINT32 fd;
            VosMsgHeader *msg = NULL;
            UINT32 timeout = SMD_CONNECT_TO_LAUNCH_MSG_TIMEOUT;
            VOS_RET_E ret;

            vosLog_debug("accept for %d", i);

            sockAddrSize = sizeof(clientAddr);
            if ((fd = accept(sg_ipcListenFd, (struct sockaddr *)&clientAddr, &sockAddrSize)) < 0)
            {
                vosLog_error("accept IPC connection failed. errno=%d", errno);
            }
            else
            {
                vosLog_debug("accepted new connection from app on fd %d", fd);

                /*
                 * When a newly launched app calls vosMsg_init(), that function will send
                 * us an APP_LAUNCHED message to identify themself and to confirm that my
                 * smd_launchApp really worked.  Apps which do not use the VOS messaging system,
                 * and therefore do not call vosMsg_init(), are marked as RUNNING immediately.
                 */
                ret = oalVosMsg_receive(fd, &msg, &timeout);
                if (ret != VOS_RET_SUCCESS)
                {
                    vosLog_error("could not receive launch msg, ret=%d", ret);
                    close(fd);
                }
                else
                {
                    vosLog_notice("Got new connected msg type 0x%x src=0x%x dst=0x%x wordData=%d",
                                             (msg)->type, (msg)->src, (msg)->dst, msg->wordData);
                    dInfo = smd_validateAppLaunchedMsg(msg);
                    if (NULL == dInfo)
                    {
                        close(fd);
                    }
                    else
                    {
                        if (dInfo->eInfo)
                        {
                            vosLog_debug("got APP_LAUNCHED from %s (eid=%d, pid=%d) on fd %d",
                                         dInfo->eInfo->name, dInfo->eInfo->eid, dInfo->pid, fd);
                            dInfo->commFd = fd;
                            smd_processLaunchConfirmation(dInfo);
                            /* mwang_todo: stop launch conf timer */
                        }
                        else
                        {
                            vosLog_error("NULL == dInfo->eInfo");
                        }
                    }

                    VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
                }
            }
        }

        /*
         * Has it been SMD_ZOMBIE_CHECK_INTERVAL ms since we checked for Zombies?
         * Zombies occur when an app launched by smd does not create a
         * VOS Message handle/pipe back to smd.  As a result, smd does not
         * know if it exits.  I don't want to wake up smd just to check for
         * zombies because it will affect performance benchmarks, so only check
         * if smd is already awake doing other stuff.
         */
        #if 0
        utilTms_get(&nowTs);
        if (SMD_ZOMBIE_CHECK_INTERVAL < utilTms_deltaInMilliSeconds(&nowTs, &sg_lastZombieCheckTs))
        {
            utilTms_get(&sg_lastZombieCheckTs);
            smd_collectZombies();
        }
        #endif
#ifdef SUPPORT_DEBUG_TOOLS
        /* check for events on the system monitor fd */
        if (i == g_sysmonFd)
        {
            SMD_SysmonProcessNewConnection();
        }
#endif

    }

    return VOS_RET_SUCCESS;

}   /* End of SMD_processEvents() */


VOS_RET_E smd_lockInit(UBOOL8 attachExisting)
{
    UINT32 flags;

    flags = (attachExisting) ? 0 : IPC_CREAT;

    if ((semid = semget(MDM_LOCK_SEMAPHORE_KEY, NUM_SEMAPHORES, flags | 0666)) == -1)
    {
        vosLog_error("semget failed, errno=%d", errno);
        return VOS_RET_INTERNAL_ERROR;
    }

    smdLckCtx.locked = FALSE;

    if (attachExisting)
    {
        vosLog_notice("attach existing done, semid=%d", semid);
        return VOS_RET_SUCCESS;
    }

    /*
     * We are creating new semaphore, so initialize semaphore to 0.
     */
    if (semctl(semid, semIndex, SETVAL, 0) == -1)
    {
        vosLog_error("setctl setval 0 failed, errno=%d", errno);
        smd_LockCleanup();
        return VOS_RET_INTERNAL_ERROR;
    }

    /* initialize system-wide lock debug tracking */
    smdLckCtx.locked = FALSE;
    smdLckCtx.lockOwner = UTIL_INVALID_PID;
    smdLckCtx.timeAquired.nsec = 0;
    smdLckCtx.timeAquired.sec = 0;
    memset(smdLckCtx.callerFuncName, 0, sizeof(smdLckCtx.callerFuncName));

    return VOS_RET_SUCCESS;
}


void smd_LockCleanup(void)
{
    SINT32 rc;

    if ((smdLckCtx.locked) || (smdLckCtx.lockOwner != UTIL_INVALID_PID))
    {
        vosLog_error("lock is still held by %d, abort delete", smdLckCtx.lockOwner);
        return;
    }


    if ((rc = semctl(semid, 0, IPC_RMID)) < 0)
    {
        vosLog_error("IPC_RMID failed, errno=%d, %s", errno, strerror(errno));
    }
    else
    {
        vosLog_notice("Semid %d deleted.", semid);
        semid = -1;
    }
}


VOS_RET_E smd_Lock(const UINT32 * timeoutMs, SMD_DLS_INFO_T * dInfo)
{
    struct sembuf lockOp[2];
    SINT32 rc = -1;
    UINT32 timeRemainingMs = 0;
    UtilTimestamp startTms, stopTms;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UBOOL8 extraDebug = FALSE;

    lockOp[0].sem_num = semIndex;
    lockOp[0].sem_op = 0;   /* wait for zero: block until write count goes to 0. */
    lockOp[0].sem_flg = 0;

    lockOp[1].sem_num = semIndex;
    lockOp[1].sem_op = 1;   /* incr sem count by 1 */
    lockOp[1].sem_flg = SEM_UNDO;   /* automatically undo this op if process terminates. */

    if (smdLckCtx.locked)
    {
        vosLog_debug("lock currently held by pid=%d func=%s", smdLckCtx.lockOwner, smdLckCtx.callerFuncName);
        extraDebug = TRUE;
    }

    if (timeoutMs != NULL)
    {
        timeRemainingMs = *timeoutMs;
    }


    while (TRUE)
    {
        /*
         * If user specified a timeout, initialize timeout and pass it to semtimedop.
         * If fourth arg to semtimedop is NULL, then it blocks indefinately.
         */
        if (timeoutMs != NULL)
        {
            struct timespec timeout;

            utilTms_get(&startTms);
            timeout.tv_sec = timeRemainingMs / MSECS_IN_SEC;
            timeout.tv_nsec = (timeRemainingMs % MSECS_IN_SEC) * NSECS_IN_MSEC;
            rc = semtimedop(semid, lockOp, sizeof(lockOp) / sizeof(struct sembuf), &timeout);
        }
        else
        {
            rc = semop(semid, lockOp, sizeof(lockOp) / sizeof(struct sembuf));
        }

        /*
         * Our new 2.6.21 MIPS kernel returns the errno in the rc, but my Fedora 7 
         * with 2.6.22 kernel still returns -1 and sets the errno.  So check for both.
         */
        if ((rc == -1 && errno == EINTR) ||
            (rc > 0 && rc == EINTR))
        {
            /*
             * Our semaphore operation was interrupted by a signal or something,
             * go back to the top of while loop and keep trying.
             * But if user has specified a timeout, we have to calculate how long
             * we have waited already, and how much longer we need to wait.
             */
            if (timeoutMs != NULL)
            {
                UINT32 elapsedMs;

                utilTms_get(&stopTms);
                elapsedMs = utilTms_deltaInMilliSeconds(&stopTms, &startTms);

                if (elapsedMs >= timeRemainingMs)
                {
                    /* even though we woke up because of EINTR, we have waited long enough */
                    rc = EAGAIN;
                    break;
                }
                else
                {
                    /* subtract the time we already waited and wait some more */
                    timeRemainingMs -= elapsedMs;
                }
            }
        }
        else
        {
            /* If we get any error other than EINTR, break out of the loop */
            break;
        }
    }

    if (extraDebug)
    {
        vosLog_debug("lock grab result, rc=%d errno=%d", rc, errno);
    }

    if (rc != 0)
    {
        /*
         * most likely cause of error is caught signal, we could also
         * get EIDRM if someone deletes the semphore while we are waiting
         * for it (that indicates programming error.)
         */
        if (errno == EINTR || rc == EINTR)
        {
            vosLog_notice("lock interrupted by signal");
            ret = VOS_RET_OP_INTR;
        }
        else if (errno == EAGAIN || rc == EAGAIN)
        {
            /* the new 2.6.21 kernel seems to return the errno in the rc */
            vosLog_debug("timed out, errno=%d rc=%d", errno, rc);
            return VOS_RET_TIMED_OUT;
        }
        else
        {
            vosLog_error("lock failed, errno=%d rc=%d", errno, rc);
            ret = VOS_RET_INTERNAL_ERROR;
        }
    }
    else
    {
        /* I got the lock! */

        /*
         * Because of the SEM_UNDO feature, when I acquire a lock,
         * if I notice that my mdmShmCtx does not have the same info,
         * then that means the previous owner died suddenly and did not clean up.
         * Update my mdmShmCtx structure to reflect reality.
         */
        if ((smdLckCtx.locked) || (smdLckCtx.lockOwner != UTIL_INVALID_PID))
        {
            vosLog_notice("correcting stale lock data from pid %d", smdLckCtx.lockOwner);
            smdLckCtx.locked = FALSE;
            smdLckCtx.lockOwner = UTIL_INVALID_PID;
        }

        /* update my lock tracking variables */
        smdLckCtx.locked = TRUE;
        smdLckCtx.lockOwner = dInfo->eInfo->eid;
    }

    return ret;
}


VOS_RET_E smd_Unlock()
{
    struct sembuf unlockOp[1];
    SINT32 semval;
    VOS_RET_E ret = VOS_RET_SUCCESS;

    unlockOp[0].sem_num = semIndex;
    unlockOp[0].sem_op = -1;    /* decr sem count by 1 */
    unlockOp[0].sem_flg = SEM_UNDO; /* undo the undo state built up in the kernel during the lockOp */


    /* kernel should have semval of 1 */
    if ((semval = semctl(semid, semIndex, GETVAL, 0)) != 1)
    {
        vosLog_error("kernel has semval=%d", semval);
        utilAst_assert(0);
    }

    /* clear my lock tracking variables before the actual release */
    smdLckCtx.locked = FALSE;
    smdLckCtx.lockOwner = UTIL_INVALID_PID;

    /* now do the actual release */
    if (semop(semid, unlockOp, sizeof(unlockOp) / sizeof(struct sembuf)) == -1)
    {
        vosLog_error("release lock failed, errno=%d", errno);
        ret = VOS_RET_INTERNAL_ERROR;
    }

    return ret;
}


/** This is the main loop of smd.
 */
static int smd_main(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    int exitCode = 0;

    while (g_keepLooping)
    {

        /*
        * wait for events (using OS dependent select), and proccess them.
        */
        if ((ret = SMD_processEvents()) != VOS_RET_SUCCESS)
        {
            /* time to exit, mwang_todo: set exit code based on return value */
            g_keepLooping = FALSE;
        }

    }

    return exitCode;
}


//func_name:daemonize()
//          daemone this process
void daemonize()
{
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);

    if( 0 != fork() )
        exit(0);

    if(-1 == setsid())
    {
        printf("===ERROR! == setsid fail ==\n");
        exit(0);
    }

    signal(SIGHUP,SIG_IGN);

    if(0!=fork())
        exit(0);

    if(0!=chdir("/"))
        exit(0);

}


int main(int argc, char *argv[])
{

    daemonize();
    int c = -1, logLevelNum = 0;
    VosLogLevel logLevel = DEFAULT_LOG_LEVEL;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    SINT32 exitCode = 0;
    UBOOL8 useConfiguredLogLevel = TRUE;

    vosLog_init(EID_SMD); 
    vosEid_init();
#if 0
    /* parse command line args */
    while ((c = getopt(argc, argv, "v:")) != -1)
    {
        switch (c)
        {
        case 'v':
            logLevelNum = atoi(optarg);
            if (logLevelNum == 0)
            {
                logLevel = VOS_LOG_LEVEL_ERR;
            }
            else if (logLevelNum == 1)
            {
                logLevel = VOS_LOG_LEVEL_NOTICE;
            }
            else
            {
                logLevel = VOS_LOG_LEVEL_DEBUG;
            }
            vosLog_setLevel(logLevel);
            useConfiguredLogLevel = FALSE;
            break;

        default:
            smd_usage(argv[0]);
            vosLog_error("bad arguments, exit");
            vosLog_cleanup();
            exit(-1);
        }
    }
#endif
    if (1)
    {
        smd_initLoggingFromConfig();
    }

    if ((ret = smd_init()) != VOS_RET_SUCCESS)
    {
        vosLog_error("initialization failed (%d), exit.", ret);
        vosLog_cleanup();
        return -1;
    }

    exitCode = smd_main();
    vosLog_notice("exiting with code %d", exitCode);

    //smd_cleanup();
    vosLog_cleanup();

    return exitCode;
}

