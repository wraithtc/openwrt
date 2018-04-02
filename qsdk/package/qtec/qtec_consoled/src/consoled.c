#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "fwk.h"
#include "cli_vty.h"
#include "cli_api.h"


/* global vars */
UBOOL8 g_keepLooping = TRUE;

/* file local vars */
void *g_msgHandle = NULL;


#ifdef DESKTOP_LINUX
static void con_terminalsignalhandler(int signum)
{
    vosLog_notice("caught signal %d, set keepLooping to FALSE", signum);
    g_keepLooping = FALSE;
}
#endif


static void initLoggingFromConfig(void)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    VosLogLevel logLevel = DEFAULT_LOG_LEVEL;
    VosLogDestination logDest = VOS_LOG_DEST_STDERR;

    vosLog_setLevel(logLevel);
    vosLog_setDestination(logDest);

    return;
}


static void con_usage(char *progName)
{
    printf("con_usage: %s [-v num] [-m shmId]\n", progName);
    printf("       v: set verbosity, where num==0 is VOS_LOG_ERROR, 1 is VOS_LOG_NOTICE, all others is VOS_LOG_DEBUG\n");
    printf("       m: shared memory id, -1 if standalone or not using shared mem.\n");
}


SINT32 main(SINT32 argc, char *argv[])
{
    int c = -1, logLevelNum = 0;
    VosLogLevel logLevel = DEFAULT_LOG_LEVEL;
    UBOOL8 useConfiguredLogLevel = TRUE;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    VTY_T *vty = NULL;
    CLI_NODE_ID perm;
    CLI_LEVEL level;

    vosLog_init(EID_CONSOLED); 

    /* update cli lib with the application data */
    cli_setAppData("Consoled", NULL, NULL, 0);

#ifdef DESKTOP_LINUX
    /*
     * On desktop, catch SIGINT and cleanly exit.
     */
    signal(SIGINT, con_terminalsignalhandler);
#else
    /*
     * On the modem, block SIGINT because user might press control-c to stop
     * a ping command or something.
     */
    signal(SIGINT, SIG_IGN);
#endif

    while ((c = getopt(argc, argv, "v:m:")) != -1)
    {
        switch (c)
        {
        case 'm':
            break;

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
            vosLog_error("bad arguments, exit");
            con_usage(argv[0]);
            vosLog_cleanup();
            exit(-1);
        }
    }

    if ((ret = vosMsg_init(EID_CONSOLED, &g_msgHandle)) != VOS_RET_SUCCESS)
    {
        vosLog_error("could not initialize msg, ret=%d", ret);
        vosLog_cleanup();
        exit(-1);
    }

    if (useConfiguredLogLevel)
    {
        initLoggingFromConfig();
    }

    cli_printWelcomeBanner();

    ret = cli_authenticate(NETWORK_ACCESS_CONSOLE, CONSOLED_NO_EXIT_ON_IDLE, &level, &perm);
    if (ret == VOS_RET_SUCCESS)
    {
        vty = vty_create(0);
        if (NULL == vty)
        {
            vosLog_error("could not create vty");
            vosLog_cleanup();
            vosMsg_cleanup(&g_msgHandle);
            exit(-1);
        }

        vty->privilege = level;
        vty->node = perm;
        cli_run(vty, g_msgHandle, CONSOLED_NO_EXIT_ON_IDLE);
    }

    vty_free(vty);
    vosMsg_cleanup(&g_msgHandle);
    vosLog_cleanup();

    return 0;
}
