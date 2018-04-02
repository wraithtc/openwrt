#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "fwk.h"
#include "vos_log_oal.h"


/** local definitions **/
#define LIGHT_RED_COLOR "\033[1;31m"
#define LIGHT_GREEN_COLOR "\033[1;32m"
#define BLUE_COLOR "\033[0;34m"
#define DEFAULT_COLOR "\033[0m"

/* default settings */

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/
static VosEntityId gEid;

static VOS_LOG_SHARED_MEM_T *logShareMem = NULL;

static UINT32 logHeaderMask; /**< Bitmask of which pieces of info we want
                              *   in the log line header.
                              */ 


  void write_log(const char *fmt, ...)
  {
      int ret = 0;
      char buf[4096] = {0};
      char *cmd = NULL;
      char *allocBuf = NULL;
      va_list paraList;
      FILE *fp;
  
      if (access("/tmp/smdprint", F_OK) != 0)
          return;
      
      va_start(paraList, fmt);
      vsnprintf(buf, sizeof(buf), fmt, paraList);
      va_end(paraList);
  
      fp = fopen("/tmp/smdprint", "a");
      fprintf(fp,"%s", buf);
      fclose(fp);
  }


void vosLog_printf(VosLogLevel logLevel, VosLogDestination logDestination, UBOOL8 newLine, const char *buf)
{
    int logTelnetFd = -1;
    
    if (NULL == buf)
    {
        return;
    }

    if (logDestination == VOS_LOG_DEST_STDERR)
    {
        if (newLine)
        {
            write_log("%s\n", buf);
        }
        else
        {
            write_log("%s", buf);
        }
        
        fflush(stderr);
    }
    else if (logDestination == VOS_LOG_DEST_TELNET )
    {
#ifdef DESKTOP_LINUX
        /* Fedora Desktop Linux */
        logTelnetFd = open("/dev/pts/1", O_RDWR);
#else
        /* CPE use ptyp0 as the first pesudo terminal */
        logTelnetFd = open("/dev/ttyp0", O_RDWR);
#endif
        if(logTelnetFd != -1)
        {
            write(logTelnetFd, buf, strlen(buf));
            close(logTelnetFd);
        }
    }
    else
    {
        oalLog_syslog(logLevel, buf);
    }
}


void vosLog_cache(const char *func, UINT32 lineNum)
{
    UINT32 location = 0;

    if (NULL == logShareMem)
    {
        return;
    }

    location = logShareMem->location;

    if (location < VOS_LOG_MAX_CACHE_NUM)
    {
        /* If use logShareMem->location++ directly,
         * oal_spawnProcess may cause memory write-overflow,
         * because child and parent process use the same logShareMem,
         * and logShareMem->location == VOS_LOG_MAX_CACHE_NUM at a time */

        UTIL_SNPRINTF(logShareMem->cache[location++],
            VOS_LOG_MAX_CACHE_LEN, "<%s:%d>", func, lineNum);
    }

    logShareMem->location = (location % VOS_LOG_MAX_CACHE_NUM);
}


void log_log(VosLogLevel level, const char *func, UINT32 lineNum, const char *fmt, ... )
{
    va_list ap;
    char buf[MAX_LOG_LINE_LENGTH] = {0};
    int len=0, maxLen;
    char *logLevelStr=NULL;
    const VosEntityInfo *einfo=NULL;
    UINT32 headerMask = logHeaderMask;

    if (NULL == logShareMem)
    {
        //printf("%s:%d:[%s]logShareMem is null\n", __FUNCTION__, __LINE__, func);
        return;
    }
   
    vosLog_cache(func, lineNum);

    maxLen = sizeof(buf);

    if (level <= logShareMem->logLevel)
    {
        va_start(ap, fmt);

        switch(level)
        {
            case VOS_LOG_LEVEL_ERR:
                len = snprintf(buf, maxLen, LIGHT_RED_COLOR);
                break;
            case VOS_LOG_LEVEL_NOTICE:
                len = snprintf(buf, maxLen, LIGHT_GREEN_COLOR);
                break;
            case VOS_LOG_LEVEL_DEBUG:
                len = snprintf(buf, maxLen, BLUE_COLOR);
                break;
            case VOS_LOG_LEVEL_PRINT:
                headerMask = 0;
                break;
            default:
                break;
        }

        if (headerMask & VOS_LOG_HDRMASK_APPNAME)
        {
            if ((einfo = vosEid_getEntityInfo(gEid)) != NULL)
            {
                len += snprintf(&(buf[len]), maxLen - len, "%s:", einfo->name);
            }
            else
            {
                len += snprintf(&(buf[len]), maxLen - len, "unknown:");
            }
        }

        if ((headerMask & VOS_LOG_HDRMASK_LEVEL) && (len < maxLen))
        {
         /*
                * Only log the severity level when going to stderr
                * because syslog already logs the severity level for us.
                */
            if (logShareMem->logDestination == VOS_LOG_DEST_STDERR)
            {
                switch(level)
                {
                    case VOS_LOG_LEVEL_ERR:
                        logLevelStr = "error";
                        break;
                    case VOS_LOG_LEVEL_NOTICE:
                        logLevelStr = "notice";
                        break;
                    case VOS_LOG_LEVEL_DEBUG:
                        logLevelStr = "debug";
                        break;
                    default:
                        logLevelStr = "invalid";
                        break;
                }
                len += snprintf(&(buf[len]), maxLen - len, "%s:", logLevelStr);
            }
        }

        /*
            * VosLog timestamp for both stderr and syslog because syslog's
            * timestamp is when the syslogd gets the log, not when it was
            * generated.
            */
        if ((headerMask & VOS_LOG_HDRMASK_TIMESTAMP) && (len < maxLen))
        {
            UtilTimestamp ts;

            utilTms_get(&ts);
            len += snprintf(&(buf[len]), maxLen - len, "%u.%03u:",
                            ts.sec%1000, ts.nsec/NSECS_IN_MSEC);
        }

        if ((headerMask & VOS_LOG_HDRMASK_LOCATION) && (len < maxLen))
        {
            len += snprintf(&(buf[len]), maxLen - len, "%s:%u:", func, lineNum);
        }

        if (len < maxLen)
        {
            len += vsnprintf(&buf[len], maxLen - len, fmt, ap);
        }

        if (VOS_LOG_LEVEL_PRINT == level)
        {
            vosLog_printf(logShareMem->logLevel, logShareMem->logDestination, FALSE, buf);
        }
        else
        {
            len += snprintf(&(buf[len]), maxLen - len, DEFAULT_COLOR);
            vosLog_printf(logShareMem->logLevel, logShareMem->logDestination, TRUE, buf);
        }
        
        va_end(ap);
    }
}  /* End of log_log() */


void vosLog_init(VosEntityId eid)
{
    SINT32 shmId = 0;
    int shmFlg = 0;
    char keyfile[BUFLEN_32] = {0};
    key_t key;
    FILE *fp = NULL;
    const VosEntityInfo *einfo = NULL;

    einfo = vosEid_getEntityInfo(eid);
    if (NULL == einfo)
    {
        printf("%s:%d:einfo(%d) is null\n", __FUNCTION__, __LINE__, eid);
        return;
    }

    UTIL_SNPRINTF(keyfile, sizeof(keyfile), VOS_LOG_KEY_FILE_PREFIX"%s", einfo->name);
    fp = fopen(keyfile, "a");
    if (fp != NULL)
    {
        fclose(fp);
    }
    else
    {
        printf("%s:%d:fopen %s error\n", __FUNCTION__, __LINE__, keyfile);
        return;
    }
    
    key = ftok(keyfile, 'a');

    shmId = shmget(key, 0, 0);
    if (-1 == shmId)
    {
        shmFlg = 0666;
        shmFlg |= (IPC_CREAT | IPC_EXCL);
        
        shmId = shmget(key, sizeof(VOS_LOG_SHARED_MEM_T), shmFlg);
        if (-1 == shmId)
        {
            printf("%s:%d:eid=%s, shmget fail\n", __FUNCTION__, __LINE__, einfo->name);
            return;
        }
    }

    logShareMem = (VOS_LOG_SHARED_MEM_T *)shmat(shmId, NULL, 0);
    if ((VOS_LOG_SHARED_MEM_T *)(-1) == logShareMem)
    {
        printf("%s:%d:eid=%s, shmat fail\n", __FUNCTION__, __LINE__, einfo->name);
        logShareMem = NULL;
        return;
    }

    logShareMem->logLevel = DEFAULT_LOG_LEVEL;
    logShareMem->logDestination = DEFAULT_LOG_DESTINATION;

    logHeaderMask  = DEFAULT_LOG_HEADER_MASK;
    gEid = (VosEntityId)eid;

    oalLog_init();

    return;
}  /* End of vosLog_init() */

  
void vosLog_cleanup(void)
{
   oalLog_cleanup();
   return;

}  /* End of vosLog_cleanup() */
  

void vosLog_setLevel(VosLogLevel level)
{
    if (logShareMem != NULL)
    {
        logShareMem->logLevel = level;
    }
}


VosLogLevel vosLog_getLevel(void)
{
    if (logShareMem != NULL)
    {
        return logShareMem->logLevel;
    }

    return DEFAULT_LOG_LEVEL;
}


void vosLog_setDestination(VosLogDestination dest)
{
    if (logShareMem != NULL)
    {
        logShareMem->logDestination = dest;
    }
}


VosLogDestination vosLog_getDestination(void)
{
    if (logShareMem != NULL)
    {
        return logShareMem->logDestination;
    }

    return DEFAULT_LOG_DESTINATION;
}


void vosLog_setHeaderMask(UINT32 headerMask)
{
   logHeaderMask = headerMask;
   return;
}


UINT32 vosLog_getHeaderMask(void)
{
   return logHeaderMask;
}


VOS_RET_E vosLog_security(VosLogSecurityLogIDs id, VosLogSecurityLogData * pdata, const char *fmt, ...)
{
    return (VOS_RET_SUCCESS);
}


VOS_RET_E vosSyslog_info(const char *fmt, ...)
{
    int len = 0;
    va_list ap;
    char buf[MAX_LOG_LINE_LENGTH] = {0};

    va_start(ap, fmt);

    len = snprintf(buf, MAX_LOG_LINE_LENGTH, "000000 ");
    len += vsnprintf(&buf[len], MAX_LOG_LINE_LENGTH - len, fmt, ap);
    va_end(ap);

    syslog(LOG_INFO, buf);
    return (VOS_RET_SUCCESS);
}

