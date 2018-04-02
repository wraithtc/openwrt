#include <termios.h>
#include "fwk.h"
#include "cli_vty.h"
#include "cli_api.h"
#include "cli.h"
 

#ifndef MIN
#define   MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define CLI_BACKSPACE        '\x08'


/* global */
char menuPathBuf[MAX_MENU_PATH_LENGTH] = " > ";
UBOOL8 cli_keepLooping = TRUE;
UINT32 exitOnIdleTimeout;
void *msgHandle = NULL;
char adminUserName[BUFLEN_16] = "admin";
char adminPassword[BUFLEN_24] = "admin";
char sptUserName[BUFLEN_16];
char sptPassword[BUFLEN_24];
char usrUserName[BUFLEN_16];
char usrPassword[BUFLEN_24];

/* authentication information */
char currUser[BUFLEN_64];
UINT8 currPerm = 0;
UINT16 currAppPort = 0;
char currAppName[BUFLEN_16] = { 0 };
char currIpAddr[UTIL_IPADDR_LENGTH];


void cli_printWelcomeBanner(void)
{
    UINT32 chipId;

//   VOS_RET_E ret;

//   if ((ret = devCtl_getChipId(&chipId)) != VOS_RET_SUCCESS)
    {
//        vosLog_error("could not get chipid, fake it");
        chipId = 0x9999;
    }

    printf("Welcome to 3caretec Router\n", chipId);
}


VOS_RET_E vosLog_security(VosLogSecurityLogIDs id, VosLogSecurityLogData * pdata, const char *fmt, ...)
{
    return (VOS_RET_SUCCESS);
}


VOS_RET_E cli_authenticate(NetworkAccessMode accessMode,
                           UINT32 eoiTimeout,
                           CLI_LEVEL *level,
                           CLI_NODE_ID *perm)
{
    char login[BUFLEN_256], pwd[BUFLEN_256];
    char *pc = NULL;
    int done = FALSE, authNum = 0;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    UBOOL8 bAuth = FALSE;
    VosLogSecurityLogData logData = { 0 };

    exitOnIdleTimeout = eoiTimeout;

#ifdef CLI_BYPASS_LOGIN
    /*
     * The ifdef symbol name is a little confusing.  For now, it only applies to
     * logins from console port, although it may be extended to cover telnet and
     * ssh in the future.
     * So if this is a console login, then just set the perm as admin and return.
     */
    if (accessMode == NETWORK_ACCESS_CONSOLE)
    {
        UTIL_STRNCPY(currUser, adminUserName, sizeof(currUser));
        currPerm = PERM_ADMIN;

        VOS_LOG_SEC_SET_APP_NAME(&logData, &currAppName[0]);
        VOS_LOG_SEC_SET_USER(&logData, adminUserName);
        if (currAppPort != 0)
        {
            VOS_LOG_SEC_SET_PORT(&logData, currAppPort);
            VOS_LOG_SEC_SET_SRC_IP(&logData, &currIpAddr[0]);
        }
        vosLog_security(VOS_LOG_SECURITY_AUTH_LOGIN_PASS, &logData, NULL);

        //Tentative: Permissions as same as ROOT
        *level = CLI_LEVEL_DEBUG;
        *perm = ROOT_NODE;

        return VOS_RET_SUCCESS;
    }
#endif


    // read empty line to wait for input, for accessing from the console only
    // Also solve the carriage return problem for windows 2000 telnet client
    if (accessMode == NETWORK_ACCESS_CONSOLE)
    {
        if ((ret = cli_readString(login, sizeof(login))) != VOS_RET_SUCCESS)
        {
            return ret;
        }
    }

    while (done == FALSE)
    {
        login[0] = '\0';
        pwd[0] = '\0';
        printf("VosLogin: ");

        // When the serial port is not configured, telnet sessions need
        // stdout to be flushed here.
        fflush(stdout);

        // Read username string, while checking for idle timeout
        if ((ret = cli_readString(login, sizeof(login))) != VOS_RET_SUCCESS)
        {
            break;
        }

        /* mwang_todo: uh-oh, this will not time out.  Need to explicitly code something
         * to turn off echoing, do a read, and then turn echo back on. */
        pc = getpass("Password: ");
        if (pc != NULL)
        {
            UTIL_STRNCPY(pwd, pc, sizeof(pwd));
            bzero(pc, strlen(pc));
        }

        /*check the accessMode, userName, passWord, return allowed or not allowed*/
        
        vosLog_debug("Enter>, accessMode = %u, login = %s, pwd = %s", accessMode, login, pwd);        
        bAuth = (util_strncmp(login, "root", sizeof(login)) == 0) && (util_strncmp(pwd, "qtec1234", sizeof(pwd)) == 0);
        vosLog_debug("bAuth = %s", bAuth?"TRUE":"FALSE");

        authNum++;

        VOS_LOG_SEC_SET_APP_NAME(&logData, &currAppName[0]);
        VOS_LOG_SEC_SET_USER(&logData, &login[0]);
        if (currAppPort != 0)
        {
            VOS_LOG_SEC_SET_PORT(&logData, currAppPort);
            VOS_LOG_SEC_SET_SRC_IP(&logData, &currIpAddr[0]);
        }

        if (bAuth)
        {
            vosLog_security(VOS_LOG_SECURITY_AUTH_LOGIN_PASS, &logData, NULL);
            done = TRUE;
            
            /* remember who has logged in and set the permission bits */
            UTIL_STRNCPY(currUser, login, sizeof(currUser));
            currPerm = PERM_ADMIN;

        }
        else if (authNum >= CLI_AUTH_NUM_MAX)
        {
            printf("Authorization failed after trying %d times!!!.\n", authNum);
            vosLog_security(VOS_LOG_SECURITY_LOCKOUT_START, &logData, NULL);
            sleep(AUTH_FAIL_SLEEP);
            vosLog_security(VOS_LOG_SECURITY_LOCKOUT_END, &logData, NULL);
            authNum = 0;
        }
        else
        {
            vosLog_security(VOS_LOG_SECURITY_AUTH_LOGIN_FAIL, &logData, NULL);
            printf("VosLogin or Password incorrect. Try again.\n");
        }
    }

    if (ret == VOS_RET_SUCCESS)
    {
        vosLog_debug("current logged in user %s perm=0x%x", currUser, currPerm);
    }

    *level = CLI_LEVEL_DEBUG;
    *perm = ROOT_NODE;

    return ret;
}


void cli_setAppData(char *appName, char *ipAddr, char *curUser, UINT16 appPort)
{
    if (NULL != appName)
    {
        memset(&currAppName[0], 0, sizeof(currAppName));
        strncpy(&currAppName[0], appName, MIN(sizeof(currAppName), strlen(appName)));
    }

    if (NULL != ipAddr)
    {
        memset(&currIpAddr[0], 0, sizeof(currIpAddr));
        strncpy(&currIpAddr[0], ipAddr, MIN(sizeof(currIpAddr), strlen(ipAddr)));
    }

    if (NULL != curUser)
    {
        memset(&currUser[0], 0, sizeof(currUser));
        strncpy(&currUser[0], curUser, MIN(sizeof(currUser), strlen(curUser)));
    }

    currAppPort = appPort;

}


/** Reads an input line from user and processes it.
 *
 */
void processInput(VTY_T *vty)
{
    char cmdLine[CLI_MAX_BUF_SZ];
    UBOOL8 foundHandler;
    int supportedModes = 0;


#ifdef SUPPORT_CLI_MENU
    supportedModes++;
#endif

#ifdef SUPPORT_CLI_CMD
    supportedModes++;
#endif

    /* if neither menu or cmd cli is enabled, just spawn a shell */
    if (supportedModes == 0)
    {
        prctl_runCommandInShellBlocking("/bin/ash --login");
        cli_keepLooping = FALSE;
        return;
    }


    /*
     * Read an input line from the user and process it using
     * menu and/or cmd code.
     */
    bzero(cmdLine, CLI_MAX_BUF_SZ);
    if (read(STDIN_FILENO, cmdLine, CLI_MAX_BUF_SZ) <= 0)
    {
        cli_keepLooping = FALSE;
        return;
    } 

    vosLog_debug("read =>%s<=", cmdLine);

    foundHandler = FALSE;

#ifdef SUPPORT_CLI_MENU
    if ((foundHandler = cli_processMenuItem(cmdLine)) == TRUE)
    {
        /* input is menu item */
        /* no need to do anything in here */
    }
#endif /* SUPPORT_CLI_MENU */

#ifdef SUPPORT_CLI_CMD
    if (!foundHandler)
    {
#if 1
        vty_read(vty, cmdLine);
#else
        if (cli_processCliCmd(cmdLine) == TRUE)
        {
            /* input is command line command */
            /* no need to do anything in here */
        }
        else if (cli_processHiddenCmd(cmdLine) == TRUE)
        {
            /* input is hidden command */
            /* no need to do anything in here */
        }
        else
        {
            vosLog_error("unrecognized command %s", cmdLine);
        }
#endif
        if (cli_keepLooping == TRUE)
        {
            cli_waitForContinue();
        }
    }
#endif /* SUPPORT_CLI_CMD */

    return;
}


VOS_RET_E cli_readString(char *buf, int size)
{
    SINT32 nchars = 0;
    int ch = 0;
    VOS_RET_E ret;

    memset(buf, 0, size);

    if ((ret = cli_waitForInputAvailable()) != VOS_RET_SUCCESS)
    {
        return ret;
    }

    /* read individual characters until we get a newline or until
     * we exceed given buffer size.
     */
    for (ch = fgetc(stdin);
         ch != '\r' && ch != '\n' && ch != EOF && nchars < (size - 1);
         ch = fgetc(stdin))
    {
        if (ch == CLI_BACKSPACE)
        {
            if (nchars > 0)
                nchars--;
        }
        else
        {
            buf[nchars++] = ch;
        }
    }

    if (ch == EOF)
    {
        printf("EOF detected, terminate login session.\n");
        exit(0);
    }

    buf[nchars] = '\0';

    return ret;
}


/** This is only needed when menu driven CLI is enabled
 *  because everytime we print out the menu, we blank out the output
 *  from the previous command.
 */
void cli_waitForContinue(void)
{
#ifdef SUPPORT_CLI_MENU
    int ch;
    VOS_RET_E ret;

    printf("\nHit <enter> to continue\n");
    fflush(stdout);

    if ((ret = cli_waitForInputAvailable()) != VOS_RET_SUCCESS)
    {
        return;
    }


    for (ch = 0; ch != '\r' && ch != '\n'; ch = fgetc(stdin))
    {
        /* read until we get \r or \n */
    }
#endif
}


VOS_RET_E cli_waitForInputAvailable()
{
    struct timeval timeout;
    struct timeval *timeoutPtr = NULL;
    fd_set readfds;
    SINT32 msgfd = 0;
    ssize_t n;

    if (exitOnIdleTimeout > 0)
    {
        timeout.tv_sec = exitOnIdleTimeout;
        timeout.tv_usec = 0;
        timeoutPtr = &timeout;
    }
    else
    {
        /*
         * If user has set exitOnIdleTimeout to 0, that means no timeout.  Wait indefinately.
         */
        timeoutPtr = NULL;
    }

    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    if (msgHandle != NULL)
    {
        vosMsg_getEventHandle(msgHandle, &msgfd);
        FD_SET(msgfd, &readfds);
    }

    n = select(msgfd + 1, &readfds, NULL, NULL, timeoutPtr);
    if (n == 0)
    {
        printf("session terminated due to idle timeout (%d seconds)\n", exitOnIdleTimeout);
        return VOS_RET_TIMED_OUT;
    }
    else if (n < 0)
    {
        vosLog_notice("select interrupted");
        return VOS_RET_OP_INTR;
    }
    else if ((msgHandle != NULL) && (FD_ISSET(msgfd, &readfds)))
    {
        VosMsgHeader *msg;
        VOS_RET_E ret;

        /* we got a message on the comm fd, read it */
        ret = vosMsg_receive(msgHandle, &msg);
        if (ret == VOS_RET_SUCCESS)
        {
            vosLog_error("unsupported msg type 0x%x", msg->type);
            VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
        }
        else if (ret == VOS_RET_DISCONNECTED)
        {
            vosLog_error("lost connection to smd, exit now.");
            cli_keepLooping = 0;
            return VOS_RET_OP_INTR;
        }
        else
        {
            vosLog_error("error during receive, ret=%d", ret);
        }
    }

    return VOS_RET_SUCCESS;
}


void cli_run(VTY_T *vty, void *mh, UINT32 eoiTimeout)
{
    VosLogSecurityLogData logData = { 0 };

    vosLog_debug("CLI library entered");
    exitOnIdleTimeout = eoiTimeout;

    msgHandle = mh;

    //vty_read(vty, "\n");

    while (cli_keepLooping)
    {
#ifdef SUPPORT_CLI_MENU
        cli_displayMenu();
#endif
        processInput(vty);
    }

    VOS_LOG_SEC_SET_APP_NAME(&logData, &currAppName[0]);
    VOS_LOG_SEC_SET_USER(&logData, &currUser[0]);
    if (currAppPort != 0)
    {
        VOS_LOG_SEC_SET_PORT(&logData, currAppPort);
        VOS_LOG_SEC_SET_SRC_IP(&logData, &currIpAddr[0]);
    }

    vosLog_security(VOS_LOG_SECURITY_AUTH_LOGOUT, &logData, NULL);

    printf("\nBye bye. Have a nice day!!!\n");

    memset(currUser, 0, sizeof(currUser));
    currPerm = 0;
}
