#include <fcntl.h>
#include <sys/stat.h>

#include "cli_util.h"
#include "cli.h"
#include "fwk.h"
#include <math.h>
#include "librtcfg.h"


typedef enum{
    CLI_UPS_GPIO_1 = 37,
    CLI_UPS_GPIO_2,
    CLI_UPS_GPIO_3,
    CLI_UPS_GPIO_4,
}CLI_UPS_GPIO;


VOS_RET_E cli_handleLinuxShell(VTY_T *vty, int argc, char **argv)
{
    vty_exit_term();
    prctl_runCommandInShellBlocking("/bin/ash --login");
    vty_init_term();
    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleGetFactoryMode(VTY_T *vty, int argc, char **argv)
{
    UINT8 isFactoryMode;
    int ret;
    ret = QtGetFactoryMode(&isFactoryMode);
    if (ret != 0)
    {
        printf("Fail\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    printf("%s\n", isFactoryMode?"Factory":"Normal");
    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleCheckDisk(VTY_T *vty, int argc, char **argv)
{
    int ret;
    char *checkResFile = "/tmp/disk_check_res";
    char readBuf[BUFLEN_256] = {0};
    FILE* fd = NULL;
    
    UTIL_DO_SYSTEM_ACTION("qtec_disk_check_and_init.sh 0 > %s", checkResFile);
    fd = fopen(checkResFile, "r");
    if (!fd)
    {
        printf("FAIL\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    ret = fread(readBuf, BUFLEN_256, 1, fd);
    if (util_strstr(readBuf, "can not find disk") != NULL)
    {
        printf("FAIL\n");
    }
    else
    {
        printf("OK\n");
    }

    fclose(fd);
    unlink(checkResFile);
    
    return VOS_RET_SUCCESS;
}




VOS_RET_E cli_handleGponShell(VTY_T *vty, int argc, char **argv)
{
    prctl_runCommandInShellBlocking("telnet 127.0.0.1 8023");
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleLogLevel(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret;
    VosEntityId eid;
    VosLogLevel logLevel;
    char *level;

    if (!util_strcasecmp(argv[0], "smd"))
    {
        eid = EID_SMD;
    }
    else if (!util_strcasecmp(argv[0], "timertask"))
    {
        eid = EID_TIMER_TASK;
    }
    else
    {
        printf("invalid or unsupported app name %s\n", argv[0]);
        return VOS_RET_INTERNAL_ERROR;
    }

    if (1 == argc)
    {
        printf("current log level is %s\n", level);
        return VOS_RET_INTERNAL_ERROR;
    }
    else
    {
        if (util_strcmp(argv[1], "over"))
        {
            UINT8 buf[sizeof(VosMsgHeader) + BUFLEN_32] = {0};
        
            VosMsgHeader *msg = (VosMsgHeader *)buf;
            memcpy((char *)(msg + 1), argv[1], BUFLEN_32);

            msg->type = VOS_MSG_LOG_REDIRECT;
            msg->src = vosMsg_getHandleEid(msgHandle);
            msg->dst = eid;
            msg->flags_request = 1;
            msg->dataLength = strlen(argv[1]) + 1;
            return vosMsg_send(msgHandle, msg);
        }
        else
        {
            VosMsgHeader msg = EMPTY_MSG_HEADER;

            msg.type = VOS_MSG_LOG_REDIRECT_END;
            msg.src = vosMsg_getHandleEid(msgHandle);
            msg.dst = eid;
            msg.flags_request = 1;
            msg.dataLength = 0;
            return vosMsg_send(msgHandle, &msg);
        }

        
    }

    return VOS_RET_SUCCESS;
}

#if 0
VOS_RET_E cli_handleLogDest(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret;
    VosEntityId eid;
    VosLogDestination logDest;
    char *dest;

    if (!util_strcasecmp(argv[0], "smd"))
    {
        eid = EID_SMD;
    }
    else if (!util_strcasecmp(argv[0], "ssk"))
    {
        eid = EID_SSK;
    }
    else if (!util_strcasecmp(argv[0], "cmc"))
    {
        eid = EID_CMC;
    }
    else if (!util_strcasecmp(argv[0], "consoled"))
    {
        eid = EID_CONSOLED;
    }
    else if (!util_strcasecmp(argv[0], "tr69c"))
    {
        eid = EID_TR69C;
    }
    else if (!util_strcasecmp(argv[0], "eponapp"))
    {
        eid = EID_EPON_APP;
    }
    else if (!util_strcasecmp(argv[0], "mcpd"))
    {
        eid = EID_MCPD;
    }
    else
    {
        printf("invalid or unsupported app name %s\n", argv[0]);
        return VOS_RET_INTERNAL_ERROR;
    }

    if (1 == argc)
    {
        ret = CMC_logGetDest(eid, &logDest);
        if (ret != VOS_RET_SUCCESS)
        {
            printf("get log dest failed, ret = %d\n", ret);
            return ret;
        }

        switch (logDest)
        {
            case VOS_LOG_DEST_STDERR:
                dest = "Standard Error";
                break;

            case VOS_LOG_DEST_SYSLOG:
                dest = "Syslog";
                break;

            case VOS_LOG_DEST_TELNET:
                dest = "Telnet";
                break;

            default:
                dest = "Unknown";
                break;
        }

        printf("current log dest is %s\n", dest);
    }
    else
    {
        if (!util_strcasecmp(argv[1], "stderror"))
        {
            logDest = VOS_LOG_DEST_STDERR;
        }
        else if (!util_strcasecmp(argv[1], "syslog"))
        {
            logDest = VOS_LOG_DEST_SYSLOG;
        }
        else if (!util_strcasecmp(argv[1], "telnet"))
        {
            logDest = VOS_LOG_DEST_TELNET;
        }
        else
        {
            printf("Unknown log dest %s\n", argv[1]);
            return VOS_RET_INTERNAL_ERROR;
        }

        ret = CMC_logSetDest(eid, logDest);
        if (ret != VOS_RET_SUCCESS)
        {
            printf("set log dest failed, ret = %d\n", ret);
            return ret;
        }

        printf("new log dest set.\n");
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSaveMdm(VTY_T *vty, int argc, char **argv)
{
    return CMC_sysSaveConfig();
}


VOS_RET_E cli_handleSetMdmObject(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SET_MDM_OBJECT,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleSetMdmObjectRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SET_MDM_OBJECT_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleSetMdmPathRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SET_MDM_PATH_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleAddMdmObject(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_ADD_MDM_OBJECT,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleAddMdmObjectRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_ADD_MDM_OBJECT_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleAddMdmPathRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_ADD_MDM_PATH_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleDelMdmObject(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_DEL_MDM_OBJECT,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleDelMdmObjectRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_DEL_MDM_OBJECT_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleDelMdmPathRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_DEL_MDM_PATH_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}

VOS_RET_E cli_handleShowMdmObject(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SHOW_MDM_OBJECT,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleShowMdmObjectRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SHOW_MDM_OBJECT_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleShowMdmPathRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SHOW_MDM_PATH_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleShowMdmTree(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SHOW_MDM_TREE,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleShowMdmTreeRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SHOW_MDM_TREE_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}

#ifndef DESKTOP_LINUX
VOS_RET_E cli_handleShowObjCmcObjTrace(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                   VOS_MSG_REMOTE_CLI_SHOW_OBJ_TRACE,
                                   argc,
                                   argv);
}
#endif

VOS_RET_E cli_handleDhcpcAddHost(VTY_T *vty, int argc, char **argv)
{
    UINT8 buf[sizeof(VosMsgHeader) + sizeof(SSK_DHCPC_STATE_CHANGE_MSGBODY_T)];
    
    VosMsgHeader *msg = (VosMsgHeader *)buf;
    SSK_DHCPC_STATE_CHANGE_MSGBODY_T *msgBody = (SSK_DHCPC_STATE_CHANGE_MSGBODY_T *)(msg + 1);

    memset(buf, 0, sizeof(buf));
    msg->type = VOS_MSG_DHCPC_STATE_CHANGED;
    msg->src = vosMsg_getHandleEid(msgHandle);
    msg->dst = EID_SSK;
    msg->flags_event = 1;
    msg->dataLength = sizeof(SSK_DHCPC_STATE_CHANGE_MSGBODY_T);

    msgBody->addressAssigned = TRUE;
    UTIL_STRNCPY(msgBody->ip, argv[0], sizeof(msgBody->ip));
    UTIL_STRNCPY(msgBody->mask, argv[1], sizeof(msgBody->mask));
    UTIL_STRNCPY(msgBody->gateway, argv[2], sizeof(msgBody->gateway));
    UTIL_STRNCPY(msgBody->nameserver, argv[3], sizeof(msgBody->nameserver));

    return vosMsg_send(msgHandle, msg);
}

VOS_RET_E cli_handlePppoeAddHost(VTY_T *vty, int argc, char **argv)
{
    UINT8 buf[sizeof(VosMsgHeader) + sizeof(SSK_PPPOE_STATE_CHANGE_MSGBODY_T)];
    VosMsgHeader *msg = (VosMsgHeader *)buf;
    SSK_PPPOE_STATE_CHANGE_MSGBODY_T *msgBody = (SSK_PPPOE_STATE_CHANGE_MSGBODY_T *)(msg + 1);

    memset(buf, 0, sizeof(buf));
    msg->type = VOS_MSG_PPPOE_STATE_CHANGED;
    msg->src = vosMsg_getHandleEid(msgHandle);
    msg->dst = EID_SSK;
    msg->flags_event = 1;
    msg->dataLength = sizeof(SSK_PPPOE_STATE_CHANGE_MSGBODY_T);

    //msgBody->addressAssigned = TRUE;
    msgBody->pppState = BCM_PPPOE_CLIENT_STATE_UP;
    UTIL_STRNCPY(msgBody->ip, argv[0], sizeof(msgBody->ip));
    UTIL_STRNCPY(msgBody->mask, argv[1], sizeof(msgBody->mask));
    UTIL_STRNCPY(msgBody->gateway, argv[2], sizeof(msgBody->gateway));
    UTIL_STRNCPY(msgBody->nameServer, argv[3], sizeof(msgBody->nameServer));

    return vosMsg_send(msgHandle, msg);
}


VOS_RET_E cli_handleShowMdm(VTY_T *vty, int argc, char **argv)
{
    if(SF_FEATURE_LOCATION_JIANGSU)
    {
        return VOS_RET_SUCCESS;
    }
    else
    {
        return CLI_handleRemoteExecute(EID_CMC,
                                       VOS_MSG_REMOTE_CLI_SHOW_MDM,
                                       argc,
                                       argv);
    }

}


VOS_RET_E cli_handleShowMdmRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    if(SF_FEATURE_LOCATION_JIANGSU)
    {
        return VOS_RET_SUCCESS;
    }
    else
    {
        return CLI_handleRemoteRuntime(EID_CMC,
                                       VOS_MSG_REMOTE_CLI_SHOW_MDM_RT,
                                       argc,
                                       argv,
                                       vecCmdDesc);
    }
}


VOS_RET_E cli_handleDoFactoryReset(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_DO_RESET,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleDoReboot(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_DO_REBOOT,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleShowTr69Soap(VTY_T *vty, int argc, char **argv)
{    
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_TR69_SHOW_SOAP,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleClearTr69cLog(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_CLEAR_TR69_SOAP,
                                   argc,
                                   argv);

}

VOS_RET_E cli_handleEnableTr69cLog(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_TR69_ENABLE_SOAP,
                                   argc,
                                   argv);

}

VOS_RET_E cli_handleDoAddObject(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_ADD_OBJ,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleDoDelObject(VTY_T *vty, int argc, char **argv)
{  
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_DEL_OBJ,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleDoGetValue(VTY_T *vty, int argc, char **argv)
{  
    if(SF_FEATURE_LOCATION_JIANGSU)
    {
        return VOS_RET_SUCCESS;
    }
    else
    {
        return CLI_handleRemoteExecute(EID_TR69C,
                                       VOS_MSG_CLI_GET_VALUE,
                                       argc,
                                       argv);
    }
}


VOS_RET_E cli_handleDoSetValue(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_SET_VALUE,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleDoGetName(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_GET_NAME,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleDoGetAttributes(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_GET_ATTRIBUTES,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleDoSetAttributes(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_TR69C,
                                   VOS_MSG_CLI_SET_ATTRIBUTES,
                                   argc,
                                   argv);
}


VOS_RET_E cli_handleShowPppInfo(VTY_T *vty, int argc, char **argv)
{
    if(SF_FEATURE_LOCATION_JIANGSU)
    {
        return VOS_RET_SUCCESS;
    }
    else
    {
        return CLI_handleRemoteExecute(EID_CMC,
                                       VOS_MSG_CLI_SHOW_PPP_INFO,
                                       argc,
                                       argv);
    }
}


VOS_RET_E cli_handleSetPppInfo(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_PPP_INFO,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleShowDhcpPool(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SHOW_DHCP_POOL,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetDhcpPool(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_DHCP_POOL,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleShowLanIP(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SHOW_LAN_IP,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetLanInfo(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_LAN_INFO,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleShowWlanInfo(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SHOW_WLAN_INFO,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetWlanInfo(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_WLAN_INFO,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleShowWlanEncryption(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SHOW_WLAN_ENCRYPTION,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetWlanEncryption(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_WLAN_ENCRYPTION,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetWlanEncryptionOpen(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_WLAN_ENCRYPTION_OPEN,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleShowWlanStatus(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SHOW_WLAN_STATUS,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetWlanStatus(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_WLAN_STATUS,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleShowWlanMode(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SHOW_WLAN_MODE,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetWlanMode(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_WLAN_MODE,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleShowTr069State(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SHOW_TR069_STATE,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetTr069State(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_CMC,
                                 VOS_MSG_CLI_SET_TR069_STATE,
                                 argc,
                                 argv);
}


VOS_RET_E cli_handleSetTr069StateRT(int argc, char **argv, UTIL_VECTOR vecCmdDesc)
{
    return CLI_handleRemoteRuntime(EID_CMC,
                                   VOS_MSG_CLI_SET_TR069_STATE_RT,
                                   argc,
                                   argv,
                                   vecCmdDesc);
}


VOS_RET_E cli_handleMmePrintRxDebug(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_MMED,
                                   VOS_MSG_MME_CLI_PRINT_RX_DEBUG,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleMmePrintTxDebug(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_MMED,
                                   VOS_MSG_MME_CLI_PRINT_TX_DEBUG,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleMmeTestLinkStatus(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_MMED,
                                   VOS_MSG_MME_CLI_TEST_LINK_STATUS,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleMmeSendPack(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_MMED,
                                   VOS_MSG_MME_CLI_SEND_PACKET,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleMmePrintRxNoSpaceDebug(VTY_T *vty, int argc, char **argv)
{
    return CLI_handleRemoteExecute(EID_MMED,
                                   VOS_MSG_MME_CLI_PRINT_RX_NO_SPACE_DEBUG,
                                   argc,
                                   argv);
}

VOS_RET_E cli_handleShowVersion(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    FILE* fs = NULL;
    char buf[BUFLEN_256] = {0};
    if (!util_strcasecmp(argv[0], "version"))
    {

        if ((fs = fopen(UTIL_CLI_VERSION_RESULT_FILE, "r")) == NULL)
        {
            vosLog_error("Can not open version result file!");
            return VOS_RET_INTERNAL_ERROR;
        }

        while(fgets(buf, sizeof(buf), fs))
        {
            printf("%s", buf);
            bzero(buf, sizeof(buf));
        }

        fclose(fs);
    }
    else if (!util_strcasecmp(argv[0], "nat"))
    {
        int ipNatNumber= 0;
        //printf("NAT session supprot 4096\n");
        if ((fs = fopen(UTIL_CLI_NAT_RESULT_FILE, "r")) == NULL)
        {
            vosLog_error("Can not open nat result file!");
            return VOS_RET_INTERNAL_ERROR;
        }

        while(fgets(buf, sizeof(buf), fs))
        {
            ipNatNumber++;//printf("%s", buf);
            bzero(buf, sizeof(buf));
        }
        printf("IP NAT Session Now is %d.\n",ipNatNumber);

        fclose(fs);
    }

    return ret;
}


VOS_RET_E cli_handleShowUrlfilterConfig(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    VosMsgHeader msg = EMPTY_MSG_HEADER;

    ret = UTIL_sendRequestToSmd(msgHandle,
                             VOS_MSG_IS_APP_RUNNING,
                             EID_URLFILTERD,
                             NULL,
                             0);

    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_notice("urlfilterd is not runing.");
        return VOS_RET_SUCCESS;
    }
    
    msg.type = VOS_MSG_URLFILTERD_SHOW_CONFIG;
    msg.src  = vosMsg_getHandleEid(msgHandle);
    msg.dst = EID_URLFILTERD;

    return vosMsg_send(msgHandle, &msg);
}


VOS_RET_E cli_handleShowMcpdRun(VTY_T *vty, int argc, char **argv)
{
    VosMsgHeader msg = EMPTY_MSG_HEADER;
    
    msg.type = VOS_MSG_MCPD_SHOW_RUNNING;
    msg.src  = vosMsg_getHandleEid(msgHandle);
    msg.dst = EID_MCPD;

    if (0 == util_strcmp(argv[0], "config"))
    {
        msg.wordData = 1;
    }
    else if (0 == util_strcmp(argv[0], "objtree"))
    {
        msg.wordData = 2;
    }
    else if (0 == util_strcmp(argv[0], "src_filter"))
    {
        msg.wordData = 3;
    }
    else if (0 == util_strcmp(argv[0], "grp_filter"))
    {
        msg.wordData = 4;
    }

    if (msg.wordData > 0)
    {
        return vosMsg_send(msgHandle, &msg);
    }
    else
    {
        vosLog_error("Unsupport cmd!");
        return VOS_RET_SUCCESS;
    }
}


#ifdef SUPPORT_DEBUG_TOOLS
VOS_RET_E cli_handleShowSharedMem(VTY_T *vty, int argc, char **argv)
{
    VosMsgHeader msg = EMPTY_MSG_HEADER;
    
    msg.type = VOS_MSG_MEM_DUMP_STATS;
    msg.src  = vosMsg_getHandleEid(msgHandle);
    
    if (0 == util_strcmp(argv[0], "cmc"))
    {
        msg.dst = EID_CMC;
    }
    else if (0 == util_strcmp(argv[0], "mmed"))
    {
        msg.dst = EID_MMED;
    }
    else if (0 == util_strcmp(argv[0], "mcpd"))
    {
        msg.dst = EID_MCPD;
    }
    else if (0 == util_strcmp(argv[0], "tr69c"))
    {
        msg.dst = EID_TR69C;
    }
    else
    {
        vosLog_error("Unsupport cmd!");
        return VOS_RET_SUCCESS;
    }

    return vosMsg_send(msgHandle, &msg);

}


VOS_RET_E cli_handleShowMcpdMem(VTY_T *vty, int argc, char **argv)
{
    VosMsgHeader msg = EMPTY_MSG_HEADER;
    
    msg.type = VOS_MSG_MCPD_MEM_TRACE;
    msg.src  = vosMsg_getHandleEid(msgHandle);
    msg.dst = EID_MCPD;
    
    return vosMsg_send(msgHandle, &msg);
}


VOS_RET_E cli_handleShowMcpdTypeMem(VTY_T *vty, int argc, char **argv)
{
    VosMsgHeader msg = EMPTY_MSG_HEADER;
    
    msg.type = VOS_MSG_MCPD_MEM_TRACE;
    msg.src  = vosMsg_getHandleEid(msgHandle);
    msg.dst = EID_MCPD;
    msg.wordData = atoi(argv[0]);
    
    return vosMsg_send(msgHandle, &msg);
}

#endif


#ifdef VOS_MEM_LEAK_TRACING
VOS_RET_E cli_handleShowTracing(VTY_T *vty, int argc, char **argv)
{
    VosMsgHeader msg = EMPTY_MSG_HEADER;

    msg.src = vosMsg_getHandleEid(msgHandle);

    if (0 == util_strcmp(argv[0], "mcpd"))
    {
        msg.dst = EID_MCPD;
    }
    else if (0 == util_strcmp(argv[0], "mme"))
    {
        msg.dst = EID_MMED;
    }
    else if (0 == util_strcmp(argv[0], "cmc"))
    {
        msg.dst = EID_CMC;
    }
    
    if (0 == util_strcmp(argv[1], "all"))
    {
        msg.type = VOS_MSG_MEM_DUMP_TRACEALL;
    }
    else if (0 == util_strcmp(argv[1], "50"))
    {
        msg.type = VOS_MSG_MEM_DUMP_TRACE50;
    }
    else
    {
        msg.type = VOS_MSG_MEM_DUMP_TRACECLONES;
    }

    return vosMsg_send(msgHandle, &msg);
}
#endif

VOS_RET_E cli_handleSetWatchdog(VTY_T *vty, int argc, char **argv)
{
    VosMsgHeader msg = EMPTY_MSG_HEADER;

    msg.src = vosMsg_getHandleEid(msgHandle);
    msg.dst = EID_SMD;
    msg.type = VOS_MSG_CLI_SET_WATCHDOG;

    if (0 == util_strcmp(argv[0], "on"))
    {
        msg.wordData = 1;
    }
    else
    {
        msg.wordData = 0;
    }

    return vosMsg_send(msgHandle, &msg);
}


VOS_RET_E cli_handleIgmpVer(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    CMC_MCAST_IGMP_CONFIG_TYPE_T igmpCfg;

    memset(&igmpCfg, 0, sizeof(igmpCfg));

    if (VOS_RET_SUCCESS != (ret = CMC_mcastGetIgmpCfg(&igmpCfg)))
    {
        vosLog_error("Fail to get igmp config(ret=%d)!", ret);
        return ret;
    }
    
    if (0 == util_strcmp(argv[0], "v1"))
    {
        igmpCfg.igmpVer = 1;
    }
    else if (0 == util_strcmp(argv[0], "v2"))
    {
        igmpCfg.igmpVer = 2;
    }
    else if (0 == util_strcmp(argv[0], "v3"))
    {
        igmpCfg.igmpVer = 3;
    }

    if (VOS_RET_SUCCESS != (ret = CMC_mcastSetIgmpCfg(&igmpCfg)))
    {
        vosLog_error("Fail to set igmp config(ret=%d)!", ret);  
    }

    return ret;
}


VOS_RET_E cli_handleFastLeave(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    CMC_MCAST_IGMP_CONFIG_TYPE_T igmpCfg;

    memset(&igmpCfg, 0, sizeof(igmpCfg));

    if (VOS_RET_SUCCESS != (ret = CMC_mcastGetIgmpCfg(&igmpCfg)))
    {
        vosLog_error("Fail to get igmp config(ret=%d)!", ret);
        return ret;
    }

    if (0 == util_strcmp(argv[0], "on"))
    {
        igmpCfg.igmpFastLeaveEnable = TRUE;
    }
    else if (0 == util_strcmp(argv[0], "off"))
    {
        igmpCfg.igmpFastLeaveEnable = FALSE;
    }
    else
    {
        vosLog_error("Unsupport command:%s.", argv[0]);
        return VOS_RET_INVALID_ARGUMENTS;
    }

    if (VOS_RET_SUCCESS != (ret = CMC_mcastSetIgmpCfg(&igmpCfg)))
    {
        vosLog_error("Fail to set igmp config(ret=%d)!", ret);  
    }

    return ret;
    
}


VOS_RET_E cli_handleLedAll(UBOOL8 ledAction)
{
    HAL_LED_NAME_E ledName;
    
    for (ledName = HAL_LED_NAME_POWER; ledName < HAL_LED_NAME_END; ledName ++)
    {
        if (ledAction)
        {
			HAL_ledSetAction(ledName, HAL_LED_ACTION_ON);
        }
        else
        {
			HAL_ledSetAction(ledName, HAL_LED_ACTION_OFF);
        }
    }
    
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetLedState(VTY_T *vty, int argc, char **argv)
{
    int ledName[10], ledcnt, i;
   
	if (SF_FEATURE_UPLINK_TYPE_EOC)
	{
	      /* bcm5358 solution */
            if (SF_FEATURE_SUPPORT_VOIP)          
            {
                SINT8 LedTestMode = 1;

                HAL_sysGetLedTestMode(&LedTestMode);
                
                /* 0:software ctrl, 1:hardware ctrl */
                if (1 == LedTestMode)
                {
                    if (VOS_RET_SUCCESS != HAL_sysSetLedTestMode(TRUE))
                    {
                        printf("HAL_sysSetLedTestMode error!");
                        return VOS_RET_INTERNAL_ERROR;
                    }
                    
                    LedTestMode = 0;/* set to sw mode */
                }

                if (0 == LedTestMode)
                {
                    if (0 == util_strcmp(argv[0], "on"))
                    {
                        cli_handleLedAll(TRUE);
                    }
                    else  if (0 == util_strcmp(argv[0], "off"))
                    {
                        cli_handleLedAll(FALSE);
                    }
                }
            }
            else
            {
#if 0	
        	    if (SF_FEATURE_CUSTOMER_FIBERHOME)
        	    {
        	        int led[] = {7/*wifi*/, 24/*len3*/, 25/*lan4*/, 26/*sys*/};
        	        memcpy(ledName, led, sizeof(led));
        	        ledcnt = sizeof(led) / sizeof(led[0]);
        	    }
        	    else if (SF_FEATURE_CUSTOMER_LOOTOM)
        	    {
        	        int led[] = {6/*usb*/, 7/*wifi*/, 9/*wps*/, 22/*lan1*/, 23/*lan2*/, 24/*lan3*/, 25/*lan4*/};
        	        memcpy(ledName, led, sizeof(led));
        	        ledcnt = sizeof(led) / sizeof(led[0]);
                }
                else  if (SF_FEATURE_CUSTOMER_DONGYAN)
                {
                    int led[] = {7/*wifi*/, 22/*lan1*/, 23/*lan2*/, 24/*lan3*/, 25/*lan4*/};
                    memcpy(ledName, led, sizeof(led));
                    ledcnt = sizeof(led) / sizeof(led[0]);
                }
                else
#endif        
                {
                    int led[] = {0, 1, 2, 3, 4,
                                 5, 6/*usb*/, 7/*wifi*/,8, 9/*wps*/,
                                 10, 11, 12, 13, 14, 
                                 15, 16, 17, 18, 19, 
                                 20, 21, 22/*lan1*/, 23/*lan2*/, 24/*lan3*/, 
                                 25/*lan4*/, 26, 27};
                    
                    memcpy(ledName, led, sizeof(led));
                    ledcnt = sizeof(led) / sizeof(led[0]);
                }
        
                if (0 == util_strcmp(argv[0], "on"))
                {
                    for (i = 0; i < ledcnt; i++)
                    {
                        UTIL_DO_SYSTEM_ACTION("echo %d 1 > /proc/led", ledName[i]);
                    }
                }
                else  if (0 == util_strcmp(argv[0], "off"))
                {
                    for (i = 0; i < ledcnt; i++)
                    {
                        UTIL_DO_SYSTEM_ACTION("echo %d 0 > /proc/led", ledName[i]);
                    }
                }
                else
                {
                    for (i = 0; i < ledcnt; i++)
                    {
                        UTIL_DO_SYSTEM_ACTION("echo %d 5 > /proc/led", ledName[i]);
                    }
                }
           }
	}
	else
	{
		if (0 == util_strcmp(argv[0], "on"))
	    {
			cli_handleLedAll(TRUE);
		}
		else  if (0 == util_strcmp(argv[0], "off"))
	    {
			cli_handleLedAll(FALSE);
		}
	}
	
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetButtonTestMode(VTY_T *vty, int argc, char **argv)
{
    if (0 == util_strcmp(argv[0], "on"))
    {
        if (SF_FEATURE_UPLINK_TYPE_EOC)
        {
            HAL_sysSetFactoryMode(TRUE);
        }
        else
        {
            HAL_sysSetFactoryTestMode(TRUE);
        }
        printf("in factory mode\r\n");
    }
    else
    {
        if (SF_FEATURE_UPLINK_TYPE_EOC)
        {
            HAL_sysSetFactoryMode(FALSE);
        }
        else
        {
            HAL_sysSetFactoryTestMode(FALSE);
        }
        printf("out factory mode\r\n");
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetCustomerArea(VTY_T *vty, int argc, char **argv)
{
    UINT8 customerArea[32];
    memset(customerArea, 0, sizeof(customerArea));

    if (HAL_sysGetCustomerArea(customerArea) != VOS_RET_SUCCESS)
    {
        printf("fail\n");
        vosLog_error("Get customer area error \n");
        return VOS_RET_INTERNAL_ERROR;
    }

    printf("%s\n", customerArea);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetCustomerArea(VTY_T *vty, int argc, char **argv)
{

    UINT8 customerArea[32];

    if (argc != 1)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }

    memset(customerArea, 0, sizeof(customerArea));

    UTIL_STRNCPY((char*)customerArea, argv[0], sizeof(customerArea));

    if (HAL_sysSetCustomerArea(customerArea) != VOS_RET_SUCCESS)
    {
        printf("fail\n");
        vosLog_error("Set customer area error \n");
        return VOS_RET_INTERNAL_ERROR;
    }

    printf("success\n");
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetCustomerName(VTY_T *vty, int argc, char **argv)
{

    UINT8 customerName[32];
    memset(customerName, 0, sizeof(customerName));

    if (HAL_sysGetCustomerName(customerName) != VOS_RET_SUCCESS)
    {
        vosLog_error("Get customer name error \n");
        return VOS_RET_INTERNAL_ERROR;
    }

    printf("%s\n", customerName);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetCustomerName(VTY_T *vty, int argc, char **argv)
{

    UINT8 customerName[32];

    if (argc != 1)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    
    memset(customerName, 0, sizeof(customerName));

    UTIL_STRNCPY((char*)customerName, argv[0], sizeof(customerName));

    if (HAL_sysSetCustomerName(customerName) != VOS_RET_SUCCESS)
    {
        vosLog_error("Set customer name error \n");
        return VOS_RET_INTERNAL_ERROR;
    }

    printf("success\n");
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleTestUsb(VTY_T *vty, int argc, char **argv)
{
    int fd;
    int fdlink1;
    int fdlink2;

    if (SF_FEATURE_DATA_MODEL_OVERSEA)
    {
        fdlink1 = open("/mnt/usb1_1",O_RDONLY);
        fdlink2 = open("/mnt/usb2_1",O_RDONLY);

        if (fdlink1>0 && fdlink2>0)
        {
            printf("success\n");
        }
        else
        {
            printf("fail\n");
        }
        close(fdlink1);
        close(fdlink2);
    }
    else
    {
        fd = open("/mnt/usb1_1", O_RDONLY);

        if (fd > 0)
        {
            printf("success\n");
        }
        else
        {
            printf("fail\n");
        }
        close(fd);
    }
    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleCheckReset(VTY_T *vty, int argc, char **argv)
{
    UBOOL8 enable;
    HAL_sysGetFactoryMode(&enable);

    if (enable)
    {
        printf("factory mode\n");
    }
    else
    {
        printf("user mode\n");
    }
    
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetSsid(VTY_T *vty, int argc, char **argv)
{
    char ssid[UTIL_WLAN_SSID_MAX_LEN];
    UTIL_STRNCPY(ssid, argv[0], sizeof(ssid));

    if (HAL_sysSetSsid(ssid) != VOS_RET_SUCCESS)
    {
        printf("fail\n");
    }
    else
    {
        printf("success\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetSsid(VTY_T *vty, int argc, char **argv)
{
    char ssid[UTIL_WLAN_SSID_MAX_LEN];
    memset(ssid, 0, sizeof(ssid));
    HAL_sysGetSsid(ssid, UTIL_WLAN_SSID_MAX_LEN);
    printf("%s\n", ssid);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetEocFirmwareVersion(VTY_T *vty, int argc, char **argv)
{
    char fwVersion[128];
    CMC_eocGetFwVersion(fwVersion, sizeof(fwVersion));
    printf("%s\n", fwVersion);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetWlanKey(VTY_T *vty, int argc, char **argv)
{
    char key[UTIL_WLAN_KEY_MAX_LEN];
    UTIL_STRNCPY(key, argv[0], sizeof(key));

    if (HAL_sysSetWlanKey(key) != VOS_RET_SUCCESS)
    {
        printf("fail\n");
    }
    else
    {
        printf("success\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetWlanKey(VTY_T *vty, int argc, char **argv)
{
    char key[UTIL_WLAN_KEY_MAX_LEN];
    memset(key, 0, sizeof(key));
    HAL_sysGetWlanKey(key, UTIL_WLAN_KEY_MAX_LEN);
    printf("%s\n", key);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetUserName(VTY_T *vty, int argc, char **argv)
{
    char name[BUFLEN_32];
    UTIL_STRNCPY(name, argv[0], sizeof(name));

    if (HAL_sysSetUserName(name) != VOS_RET_SUCCESS)
    {
        printf("fail\n");
    }
    else
    {
        printf("success\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetUserName(VTY_T *vty, int argc, char **argv)
{
    char name[BUFLEN_32];
    memset(name, 0, sizeof(name));
    HAL_sysGetUserName(name, BUFLEN_32);
    printf("%s\n", name);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetUserPassWd(VTY_T *vty, int argc, char **argv)
{
    char passWd[BUFLEN_32];
    UTIL_STRNCPY(passWd, argv[0], sizeof(passWd));

    if (HAL_sysSetUserPwd(passWd) != VOS_RET_SUCCESS)
    {
        printf("fail\n");
    }
    else
    {
        printf("success\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetIsolate(VTY_T *vty, int argc, char **argv)
{
    UINT32 isolate = atoi(argv[0]);
    
    if (CMC_eocSetChipIsolate(isolate) == VOS_RET_SUCCESS)
    {
        printf("success\n");
    }
    else
    {
        printf("fail\n");
    }

    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleGetIsolate(VTY_T *vty, int argc, char **argv)
{
    UBOOL8 isolate = FALSE;
    
    CMC_eocGetChipIsolate(&isolate);

    if (isolate)
    {
        printf("enable\n");
    }
    else
    {
        printf("disable\n");
    }
    
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetFastOnLine(VTY_T *vty, int argc, char **argv)
{
    UINT32 fastOnLine = atoi(argv[0]);
    
    if (CMC_eocSetChipFastOnLine(fastOnLine) == VOS_RET_SUCCESS)
    {
        printf("success\n");
    }
    else
    {
        printf("fail\n");
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetFastOnLine(VTY_T *vty, int argc, char **argv)
{
    UBOOL8 fastOnLine = FALSE;
    
    CMC_eocGetChipFastOnLine(&fastOnLine);

    if (fastOnLine)
    {
        printf("enable\n");
    }
    else
    {
        printf("disable\n");
    }
    
    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleGetHfid(VTY_T *vty, int argc, char **argv)
{
    char hfid[BUFLEN_18];
    memset(hfid, 0, sizeof(hfid));
    CMC_eocGetChipHfidUser(hfid, sizeof(hfid));
    printf("%s\n", hfid);
    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handlePortRxEnable(VTY_T *vty, int argc, char **argv)
{
    HAL_ethEnablePortRecive(atoi(argv[0]));

    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handlePortRxDisable(VTY_T *vty, int argc, char **argv)
{
    HAL_ethDisablePortRecive(atoi(argv[0]));

    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleSetHfid(VTY_T *vty, int argc, char **argv)
{
    char hfid[BUFLEN_12];
    UTIL_STRNCPY(hfid, argv[0], sizeof(hfid));

    if (CMC_eocSetChipHfidUser(hfid) == VOS_RET_SUCCESS)
    {
        printf("success\n");
    }
    else
    {
        printf("fail\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetUserPassWd(VTY_T *vty, int argc, char **argv)
{
    char passWd[BUFLEN_32];
    memset(passWd, 0, sizeof(passWd));
    HAL_sysGetUserPwd(passWd, BUFLEN_32);
    printf("%s\n", passWd);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleToFactory(VTY_T *vty, int argc, char **argv)
{
    UBOOL8 enable = TRUE;
    if (HAL_sysSetFactoryMode(enable) == VOS_RET_SUCCESS)
    {
        printf("set to factory mode success\n");
    }
    else
    {
        printf("set to factory mode fail\n");
        return VOS_RET_INTERNAL_ERROR;
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetMacAddr(VTY_T *vty, int argc, char **argv)
{
    UINT8 t, i;
    char *p;
    UINT8 macAddr[6];
    char macStr[18];

    if (argc != 1)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }

    UTIL_STRNCPY((char*)macStr, argv[0], sizeof(macStr));

    for(i = 0; i < 6; i++)
    {
        p = macStr + (i * 3);
        t = *(p + 3);
        *(p + 3) = '\0';
        macAddr[i] = (UINT8)strtoul((char*)p, NULL, 16);
        *(p + 3) = t;
    }
#if 0
    if (macAddr[5] < 0xff)
    {
        macAddr[5]++;
    }
    else
    {
        for(i = 0; i < 6; i++)
        {
            if (macAddr[5 - i] == 0xff)
            {
                macAddr[5 - i] = 0x00;
                if (macAddr[5 - (i + 1)] < 0xff)
                {
                    macAddr[5 - (i + 1)]++;
                }
            }
        }
    }
#endif
    HAL_sysSetMacAddr((UINT8*)macAddr);

    if (CMC_eocSetChipMac(macStr) == VOS_RET_SUCCESS)
    {
        printf("succcess\n");
    }
    else
    {
        printf("fail\n");
        return VOS_RET_INTERNAL_ERROR;
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetEocChipMacAndIsolate(VTY_T *vty, int argc, char **argv)
{
    UBOOL8 isolate;
    UINT8 t, i;
    char *p;
    UINT8 macAddr[6];
    char macStr[18];

    if (argc != 2)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }

    UTIL_STRNCPY((char*)macStr, argv[0], sizeof(macStr));

    for(i = 0; i < 6; i++)
    {
        p = macStr + (i * 3);
        t = *(p + 3);
        *(p + 3) = '\0';
        macAddr[i] = (UINT8)strtoul((char*)p, NULL, 16);
        *(p + 3) = t;
    }
#if 0
    if (macAddr[5] < 0xff)
    {
        macAddr[5]++;
    }
    else
    {
        for(i = 0; i < 6; i++)
        {
            if (macAddr[5 - i] == 0xff)
            {
                macAddr[5 - i] = 0x00;
                if (macAddr[5 - (i + 1)] < 0xff)
                {
                    macAddr[5 - (i + 1)]++;
                }
            }

        }
    }
#endif
    HAL_sysSetMacAddr((UINT8*)macAddr);

    isolate = (UBOOL8)atoi(argv[1]);

    if (CMC_eocSetChipIsolateAndMac(isolate, macStr) == VOS_RET_SUCCESS)
    {
        printf("succcess\n");
    }
    else
    {
        printf("fail\n");
        return VOS_RET_INTERNAL_ERROR;
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetEocChipMacAndIsolateFastOnLine(VTY_T *vty, int argc, char **argv)
{
    UBOOL8 isolate;
    UBOOL8 fastOnLine;
    UINT8 t, i;
    char *p;
    UINT8 macAddr[6];
    char macStr[18];

    if (argc != 3)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }

    UTIL_STRNCPY((char*)macStr, argv[0], sizeof(macStr));

    for(i = 0; i < 6; i++)
    {
        p = macStr + (i * 3);
        t = *(p + 3);
        *(p + 3) = '\0';
        macAddr[i] = (UINT8)strtoul((char*)p, NULL, 16);
        *(p + 3) = t;
    }

    HAL_sysSetMacAddr((UINT8*)macAddr);

    isolate = (UBOOL8)atoi(argv[1]);
    fastOnLine = (UBOOL8)atoi(argv[2]);
    if (CMC_eocSetChipIsolateMacAndFastOnLine(isolate, macStr, fastOnLine) == VOS_RET_SUCCESS)
    {
        printf("succcess\n");
    }
    else
    {
        printf("fail\n");
        return VOS_RET_INTERNAL_ERROR;
    }

    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleRestore(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    ret = CMC_sysResetConfig(CMC_SYS_CONFIG_RESET_LOCAL);
    sleep(2);

    if (ret == VOS_RET_SUCCESS)
    {
        printf("succcess\n");
    }
    else
    {
        printf("fail\n");
        return ret;
    }

    if (!SF_FEATURE_UPLINK_TYPE_EOC)
    {    
        UTIL_DO_SYSTEM_ACTION("reboot");
    }
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleShowtSoftVersion(VTY_T *vty, int argc, char **argv)
{    
    FILE *fp = NULL;
    char strBuff[256];
    char *token, *keyStr = "NO information";

    char showStr[128];

	if (!SF_FEATURE_UPLINK_TYPE_EOC)
	{
		system("cat /show_version");
	}
	else
	{
	    memset(showStr, 0, sizeof(showStr));

	    if (!(fp = fopen("/show_version", "r")))
	    {
	        printf("Fail to open %s \r\n", "/show_version");
	        return VOS_RET_OPEN_FILE_ERROR;
	    }

	    while(fgets(strBuff, sizeof(strBuff), fp))
	    {
	        if (strchr(strBuff, '\n'))
	        {
	            *(strchr(strBuff, '\n')) = '\0';
	        }

	        if ((token = strchr(strBuff, ':')))
	        {
	            *(strchr(strBuff, ':')) = '\0';
	        }

	        if (strstr(strBuff, "Project"))
	        {
	            token++;
	            keyStr = token + strspn(token, " ");
	            UTIL_STRNCAT(showStr, keyStr, sizeof(showStr));
	        }

	        if (strstr(strBuff, "http"))
	        {

	            token++;
	            keyStr = token + strspn(token, " ");

	            if ((token =  strchr(keyStr, ':')))
	            {
	                token++;
	                UTIL_STRNCAT(showStr, "-R1B011D", sizeof(showStr));
	                UTIL_STRNCAT(showStr, token, sizeof(showStr));
	            }
	            break;
	        }
	    }
	    fclose(fp);
	    printf("version info : %s\n", showStr);
	}

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleStrToSn(const char* str, SERIAL_NUMBER *pSn)
{
    UINT32 i;
    char   hexStr[3];

    if (NULL == pSn)
    {
        return VOS_RET_INTERNAL_ERROR;
    }
    
    if (12 != strlen(str))
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    for (i = 0; i < 4; i++)
    {
        pSn->sn[i] = str[i];
    }
    
    memset(hexStr, 0x00, 3);
    for (i = 0; i < 4; i++)
    {
        memcpy(hexStr, &(str[4 + i * 2]), 2);
        pSn->sn[4 + i] = (unsigned char)strtol(hexStr, NULL, 16);
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleOntInfoRead(ONT_INFO_T *pInfo)
{
    char snloid[49];
    char passwd[25];
    memset(snloid, 0, sizeof(snloid));
    memset(passwd, 0, sizeof(passwd));
    
    if (VOS_RET_SUCCESS == HAL_sysGetSnPwd(snloid, (UINT8 *)passwd))
    {
    
        cli_handleStrToSn(snloid, &pInfo->sn);
        memcpy(&pInfo->password, passwd, 10);
    }

    memset(snloid, 0, sizeof(snloid));
    memset(passwd, 0, sizeof(passwd));
        
    if (VOS_RET_SUCCESS == HAL_sysGetLoidPwd(snloid, sizeof(snloid), passwd, sizeof(passwd)))
    {
        memcpy(pInfo->ctcLoid.loid, snloid, 25);
        memcpy(pInfo->ctcLoid.password, passwd, 13);
    }
    
    HAL_sysGetProductCode(&pInfo->productCode.codeClass,
                          &pInfo->productCode.codeHwInfo,
                          &pInfo->productCode.codePortMap,
                          &pInfo->productCode.codeExtend);
    HAL_sysGetMacAddr((UINT8 *)&pInfo->mac1.addr);
    memcpy(&pInfo->mac2.addr, &pInfo->mac1.addr, sizeof(pInfo->mac2.addr));

    HAL_sysGetEqptId(pInfo->eqptId, EQUIPMENTID_LEN);
#ifdef PFM_HGU
    HAL_sysGetOntgVer(pInfo->ontgVer, ONTGVERSION_LEN);
#endif

	return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleEgisShow(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T egis_S;
    (void)argc;
    (void)argv;
    
    ret = cli_handleOntInfoRead(&egis_S);
    if (VOS_RET_SUCCESS != ret)
    {
        return ret;
    }

    printf("  sn          : %c%c%c%c%08ux\r\n",    \
          egis_S.sn.sn[0], egis_S.sn.sn[1], egis_S.sn.sn[2], egis_S.sn.sn[3],    \
          *((UINT32 *)&egis_S.sn.sn[4]));
                       
    printf("  ethaddr0    : %02X:%02X:%02X:%02X:%02X:%02X\r\n",    \
          egis_S.mac1.addr[0], egis_S.mac1.addr[1], egis_S.mac1.addr[2],    \
          egis_S.mac1.addr[3], egis_S.mac1.addr[4], egis_S.mac1.addr[5]);

    printf("  ethaddr1    : %02X:%02X:%02X:%02X:%02X:%02X\r\n",    \
          egis_S.mac2.addr[0], egis_S.mac2.addr[1], egis_S.mac2.addr[2],    \
          egis_S.mac2.addr[3], egis_S.mac2.addr[4], egis_S.mac2.addr[5]);

    printf("  ipAddr      : %d.%d.%d.%d\r\n", (int) ((egis_S.ipaddr>> 24) & 0xff), (int) ((egis_S.ipaddr >> 16) & 0xff),\
                                      (int) ((egis_S.ipaddr >> 8) & 0xff), (int) ((egis_S.ipaddr >> 0) & 0xff));
    
    printf("  netMask     : %d.%d.%d.%d\r\n",(int) ((egis_S.netMask>> 24) & 0xff), (int) ((egis_S.netMask >> 16) & 0xff),\
                                      (int) ((egis_S.netMask >> 8) & 0xff), (int) ((egis_S.netMask >> 0) & 0xff));

    printf("  bootType    : 0x%08x\r\n", egis_S.bootType);
    //printf("  HwVersion   : 0x%08x\r\n",egis_S.HwVersion);
    printf("  codeClass   : 0x%08ux\r\n", egis_S.productCode.codeClass);
    printf("  codeHwInfo  : 0x%08ux\r\n", egis_S.productCode.codeHwInfo);
    printf("  codePortMap : 0x%08ux\r\n", egis_S.productCode.codePortMap);    
    printf("  ctcLoid     : %s\r\n", egis_S.ctcLoid.loid);    
    printf("  ctcPassword : %s\r\n", egis_S.ctcLoid.password);
    printf("  codeExtend  : 0x%08ux\r\n", egis_S.productCode.codeExtend);   
    printf("  Password    : %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
               egis_S.password.pwd[0], egis_S.password.pwd[1], egis_S.password.pwd[2],egis_S.password.pwd[3],egis_S.password.pwd[4],
               egis_S.password.pwd[5], egis_S.password.pwd[6], egis_S.password.pwd[7],egis_S.password.pwd[8],egis_S.password.pwd[9]);

    return ret;
}


void cli_handleStrUpper(char *pStr)
{
    UINT32 i = 0;
    char *ptr = pStr;

    if (NULL == pStr)
    {
        return;
    }
    
    for (i = 0; i < (UINT32)strlen(pStr); i++)
    {
        if ((*ptr >= 'a')&&(*ptr <= 'z'))
        {
            *ptr = *ptr - 32;
        }

        ptr++;
    }

    return ;
}


UINT32 cli_handleStringToIp(char *s)
{
    UINT32 addr;
    char *e;
    UINT32 i;

    if (s == NULL)
    {
        return(0);
    }

    for (addr=0, i=0; i<4; ++i) 
    {
        UINT32 val = s ? strtoul(s, &e, 10) : 0;
        addr <<= 8;
        addr |= (val & 0xFF);
        if (s)
        {
            s = (*e) ? e+1 : e;
        }
    }

    return (htonl(addr));
}


int cli_handleStrNcaseCmp(const char *cs, const char *ct)
{
    register char res;

    for( ; ; )
    {
        if( (res = (char)(tolower(*cs) - tolower(*ct++))) != 0 || !*cs++ )
        {
            break;
        }
    }

    return res;
}



VOS_RET_E cli_handleEgisElementSet(ONT_INFO_T *pEgis, char *elemet, char *value)
{
    UINT8 i = 0;
    char *s, *e;
    char temp[10];
    UINT32 tmpval=0;

    s=value;
    
    cli_handleStrUpper(elemet);
    if(util_strncmp(elemet,"SN",2)==0)
    {
        if(12 != strlen(s))
        {
            printf("setegis error: sn len is  %d\r\n",strlen(s));
            return VOS_RET_INTERNAL_ERROR;
        }
                   
        strncpy(temp,s,4);        
        s += 4;        
        tmpval = strtoul(s, NULL, 16); 
        memcpy(&temp[4],&tmpval,4);    

        memcpy(&pEgis->sn,temp,8);
    }    
    else if(util_strncmp(elemet,"ETHADDR0",8)==0)
    {
        for (i = 0; i < 6; ++i) 
        {
            temp[i] = s ? strtoul (s, &e, 16) : 0;
            if (s)
                s = (*e) ? e + 1 : e;
        }
        memcpy(&pEgis->mac1,temp,6);
    }

    else if(util_strncmp(elemet,"ETHADDR1",8)==0)
    {    
        for (i = 0; i < 6; ++i) 
        {
            temp[i] = s ? strtoul (s, &e, 16) : 0;
            if (s)
                s = (*e) ? e + 1 : e;
        }
        memcpy(&pEgis->mac2,temp,6);
    }
    else if(util_strncmp(elemet,"BOOTTYPE",8)==0)
    {
         pEgis->bootType= strtoul(s, NULL, 16);
    }
    else if(util_strncmp(elemet,"CODECLASS",9)==0)
    {
        pEgis->productCode.codeClass= strtoul(s, NULL, 16);
    }
    else if(util_strncmp(elemet,"CODEHWINFO",10)==0)
    {
        pEgis->productCode.codeHwInfo= strtoul(s, NULL, 16);
    }

    else if(util_strncmp(elemet,"CODEPORTMAP",11)==0)
    {
        pEgis->productCode.codePortMap= strtoul(s, NULL, 16);
    }
    else if(util_strncmp(elemet,"CODEEXTEND",10)==0)
    {
        pEgis->productCode.codeExtend= strtoul(s, NULL, 16);
    }
    else if(util_strncmp(elemet,"IPADDR",6)==0)
    {        
        pEgis->ipaddr = cli_handleStringToIp(s);
    }
    else if(util_strncmp(elemet,"NETMASK",7)==0)
    {        
        pEgis->netMask= cli_handleStringToIp(s);
    }
    else if (0 == cli_handleStrNcaseCmp(elemet, "CTCLOID"))
    {
        strncpy(pEgis->ctcLoid.loid,s,sizeof(pEgis->ctcLoid.loid));
    }   
    else if (0 == cli_handleStrNcaseCmp(elemet, "CTCPASSWORD"))
    {
        strncpy(pEgis->ctcLoid.password,s,sizeof(pEgis->ctcLoid.password));
    }
    else if(util_strncmp(elemet,"GATEWAY",7)==0)
    {        
        pEgis->gateway= cli_handleStringToIp(s);
    }
    else if(util_strncmp(elemet,"PASSWD",6)==0)
    {         
        char temp_buff[2] = {0, 0};
        for (i = 0; i <PASSWORD_LEN; ++i) 
        {
            *temp_buff = *s;
            temp[i] = strtoul (temp_buff, NULL, 16);
            s = *s ? s + 1 : s;
            #if 0
            temp[i] = s ? strtoul (s, &e, 16) : 0;
            if (s)
                s = (*e) ? e + 1 : e;
            #endif
        }
        memcpy(&pEgis->password.pwd[0],temp,PASSWORD_LEN);
    }
    else
    {
        printf("cli_ManuEgisElementSet() : Error,  parameter unkwon!\r\n");
        return VOS_RET_INTERNAL_ERROR;
    }

    return VOS_RET_SUCCESS;
}



VOS_RET_E cli_handleOntInfoWrite(ONT_INFO_T *pInfo)
{
	char sn[13];
    
    memset(sn, 0, sizeof(sn));
    UTIL_SNPRINTF(sn, sizeof(sn), "%c%c%c%c%02X%02X%02X%02X", 
            pInfo->sn.sn[0], pInfo->sn.sn[1], pInfo->sn.sn[2], pInfo->sn.sn[3],
            pInfo->sn.sn[4], pInfo->sn.sn[5], pInfo->sn.sn[6], pInfo->sn.sn[7]);

    HAL_sysSetSnPwd(sn, pInfo->password.pwd);

    if (NULL != pInfo->ctcLoid.loid || NULL != pInfo->ctcLoid.password)
    {
        HAL_sysSetLoidPwd(pInfo->ctcLoid.loid, pInfo->ctcLoid.password);
    }
    
    HAL_sysSetProductCode(pInfo->productCode.codeClass, pInfo->productCode.codeHwInfo, pInfo->productCode.codePortMap, pInfo->productCode.codeExtend);
    HAL_sysSetMacAddr(pInfo->mac1.addr);
    
    HAL_sysSetEqptId(pInfo->eqptId, EQUIPMENTID_LEN);
#ifdef PFM_HGU
    HAL_sysSetOntgVer(pInfo->ontgVer, ONTGVERSION_LEN);
#endif
    
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleEgisSet(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T egis_S;

    if (argc != 2)
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&egis_S);
    if (VOS_RET_SUCCESS != ret)
    {
        printf("cli_handleOntInfoRead err\r\n");
        return ret;
    }

    if (VOS_RET_SUCCESS != cli_handleEgisElementSet(&egis_S, argv[0], argv[1]))
    {
        printf("cli_handleEgisElementSet err\r\n");
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoWrite(&egis_S);
    if (VOS_RET_SUCCESS != ret)
    {
        printf("cli_handleOntInfoWrite err\r\n");
        return ret;
    }

    return ret;
}


VOS_RET_E cli_handleOntInfoSetSn(SERIAL_NUMBER *pSn)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;

    if (NULL == pSn)
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&ontInfo);

    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(&(ontInfo.sn), pSn, sizeof(SERIAL_NUMBER));
        ret = cli_handleOntInfoWrite(&ontInfo);
    }

    return ret;
}


VOS_RET_E cli_handleWriteSn(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    SERIAL_NUMBER sn;

    if ((1 != argc) || (12 != strlen(argv[0])))
    {
        printf("SN formate: 4 vendor ID, 8 serial ID sample: TWSH01020304\r\n");
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleStrToSn(argv[0], &sn);
    if (VOS_RET_SUCCESS == ret)
    {
        ret = cli_handleOntInfoSetSn(&sn);
        printf("set sn success!\r\n");
    }

    if (VOS_RET_SUCCESS != ret)
    {
        printf("Write sn failed!\r\n");
    }
    
    return ret;
}


VOS_RET_E cli_handleOntInfoGetSn(SERIAL_NUMBER *pSn)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;

    if (NULL == pSn)
    {
        return VOS_RET_INTERNAL_ERROR;
    }
    
    ret = cli_handleOntInfoRead(&ontInfo);

    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(pSn, &(ontInfo.sn), sizeof(SERIAL_NUMBER));
    }

    return ret;
}


VOS_RET_E cli_handleReadSn(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    SERIAL_NUMBER sn;

    (void)argc;
    (void)argv;
    
    if (VOS_RET_SUCCESS == cli_handleOntInfoGetSn(&sn))
    {
        printf("ONT sn:%c%c%c%c%02X%02X%02X%02X\r\n",
               sn.sn[0], sn.sn[1], sn.sn[2], sn.sn[3],
               sn.sn[4], sn.sn[5], sn.sn[6], sn.sn[7]);
    }
    else
    {
        printf("Get ONT sn failed\r\n");
    }
    
    return ret;
}


VOS_RET_E cli_handleOntInfoGetMac(MAC_ADDR *pMac1, MAC_ADDR *pMac2)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;
    
    if ((NULL == pMac1) || (NULL == pMac2))
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&ontInfo);

    if (VOS_RET_SUCCESS == ret)
    {
        ontInfo.mac2.addr[5]++;

        memcpy(pMac1, &(ontInfo.mac1), sizeof(MAC_ADDR));
        memcpy(pMac2, &(ontInfo.mac2), sizeof(MAC_ADDR));
    }

    return ret;
}


VOS_RET_E cli_handleGet9331MacAddr(VTY_T *vty, int argc, char **argv)
{
    UINT8 i;
    UINT8 macAddr[6];

    HAL_sysGetMacAddr(macAddr);

    if (macAddr[5] < 0xff)
    {
        macAddr[5]++;
    }
    else
    {
        for(i = 0; i < 6; i++)
        {
            if (macAddr[5 - i] == 0xff)
            {
                macAddr[5 - i] = 0x00;
                if (macAddr[5 - (i + 1)] < 0xff)
                {
                    macAddr[5 - (i + 1)]++;
                }
            }
        }
    }

    printf("%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n", macAddr[0],
                                                    macAddr[1],
                                                    macAddr[2],
                                                    macAddr[3],
                                                    macAddr[4],
                                                    macAddr[5]);
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSet9331MacAddr(VTY_T *vty, int argc, char **argv)
{
        UINT8 t, i;
        char *p;
        UINT8 macAddr[6];
        char macStr[18];

        if (argc != 1)
        {
            return VOS_RET_INVALID_ARGUMENTS;
        }

        UTIL_STRNCPY((char*)macStr, argv[0], sizeof(macStr));

        for(i = 0; i < 6; i++)
        {
            p = macStr + (i * 3);
            t = *(p + 3);
            *(p + 3) = '\0';
            macAddr[i] = (UINT8)strtoul((char*)p, NULL, 16);
            *(p + 3) = t;
        }

        if (macAddr[5] > 0x0)
        {
            macAddr[5]--;
        }
        else
        {
            for(i = 0; i < 6; i++)
            {
                if (macAddr[5 - i] == 0x0)
                {
                    macAddr[5 - i] = 0xff;
                    if (macAddr[5 - (i + 1)] > 0x0)
                    {
                        macAddr[5 - (i + 1)]--;
                    }
                }

            }
        }

        HAL_sysSetMacAddr((UINT8*)macAddr);

        return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetMacAddr(VTY_T *vty, int argc, char **argv)
{
    char macStr[18];

	if (SF_FEATURE_UPLINK_TYPE_EOC)
	{
	    CMC_eocGetChipMac(macStr, sizeof(macStr));

	    printf("%s\n", macStr);
	}
	else
	{
		VOS_RET_E ret = VOS_RET_SUCCESS;
    	MAC_ADDR mac1, mac2;
	
		ret = cli_handleOntInfoGetMac(&mac1, &mac2);
	    if (VOS_RET_SUCCESS != ret)
	    {
	        printf("get mac address failed\r\n");

			return ret;
	    }
	    else
	    {
	        printf("mac addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
	               mac1.addr[0], mac1.addr[1], mac1.addr[2],
	               mac1.addr[3], mac1.addr[4], mac1.addr[5]);
	    }
	}
	
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleStrToMac(const char* str, MAC_ADDR *mac)
{
    char  hexStr[3] = {0};
    UINT8 token = 0;
    UINT32 i;
    
    if ((NULL == mac) || (NULL == str))
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    if(MAC_ADDR_LEN * 2 == strlen(str))
    {
        token = 2;
    }
    else
    if (MAC_ADDR_LEN *3 -1 == strlen(str))
    {
        token = 3;
    }
    else
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    for (i = 0; i < MAC_ADDR_LEN ; i++)
    {
        hexStr[0]    = str[i * token];
        hexStr[1]    = str[i * token + 1];
        hexStr[2]    = '\0';
        mac->addr[i] = (unsigned char)strtol(hexStr, NULL, 16);
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleOntInfoSetMac(MAC_ADDR *pMac1, MAC_ADDR *pMac2)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;
    
    if ((NULL == pMac1) || (NULL == pMac2))
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&ontInfo);

    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(&(ontInfo.mac1), pMac1, sizeof(MAC_ADDR));
        memcpy(&(ontInfo.mac2), pMac2, sizeof(MAC_ADDR));
        ret = cli_handleOntInfoWrite(&ontInfo);
    }

    return ret;
}

VOS_RET_E cli_handleGetOui(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char manufactorOui[3] = {0};
    char oui[7] = {0};

    ret = HAL_sysGetOui(manufactorOui, 3);

    UTIL_SNPRINTF(oui,
                  sizeof(oui),
                  "%02X%02X%02X", 
                  (unsigned char) manufactorOui[0], 
                  (unsigned char) manufactorOui[1], 
                  (unsigned char) manufactorOui[2]);

    if (VOS_RET_SUCCESS != ret)
    {
        printf("get oui failed!\r\n");
        return VOS_RET_INVALID_PARAM_VALUE;
    }
    else
    {
        printf("oui is %s\r\n",oui);
    }

    return VOS_RET_SUCCESS;
}

VOS_RET_E cli_handleSetOui(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    char manuOui[6] = {0};
    char oui[3] = {0};
    char strHex[3] = {0};

    if ((1 != argc) || (6 != strlen(argv[0])))
    {
        printf("OUI formate should 6 characters as A4C7DE\r\n");
        return VOS_RET_INVALID_PARAM_VALUE;
    }

    UTIL_STRNCPY(manuOui, argv[0], sizeof(manuOui) + 1);

    strHex[0] = manuOui[0]; strHex[1] = manuOui[1]; strHex[2] = '\0'; oui[0] = (char)strtoul(strHex, NULL, 16);
    strHex[0] = manuOui[2]; strHex[1] = manuOui[3]; strHex[2] = '\0'; oui[1] = (char)strtoul(strHex, NULL, 16);
    strHex[0] = manuOui[4]; strHex[1] = manuOui[5]; strHex[2] = '\0'; oui[2] = (char)strtoul(strHex, NULL, 16);
    
    ret = HAL_sysSetOui(oui, 3);
    if (VOS_RET_SUCCESS != ret)
    {
        printf("set oui failed!\r\n");
        return VOS_RET_INVALID_PARAM_VALUE;
    }
    else
    {
        printf("set oui success!\r\n");
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleWriteMac(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    MAC_ADDR mac1, mac2;

    if ((2 != argc) || (17 != strlen(argv[1])))
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }

	if (SF_FEATURE_UPLINK_TYPE_EOC && (0 == util_strncmp(argv[0], "3",1)))
    {
    	UINT8 t, i;
    	char *p;
    	UINT8 macAddr[6];
    	char macStr[18];
	
		if (argc != 1)
	    {
	        return VOS_RET_INVALID_ARGUMENTS;
	    }

	    UTIL_STRNCPY((char*)macStr, argv[0], sizeof(macStr));

	    for(i = 0; i < 6; i++)
	    {
	        p = macStr + (i * 3);
	        t = *(p + 3);
	        *(p + 3) = '\0';
	        macAddr[i] = (UINT8)strtoul((char*)p, NULL, 16);
	        *(p + 3) = t;
	    }

	    if (macAddr[5] < 0xff)
	    {
	        macAddr[5]++;
	    }
	    else
	    {
	        for(i = 0; i < 6; i++)
	        {
	            if (macAddr[5 - i] == 0xff)
	            {
	                macAddr[5 - i] = 0x00;
	                if (macAddr[5 - (i + 1)] < 0xff)
	                {
	                    macAddr[5 - (i + 1)]++;
	                }
	            }

	        }
	    }
	    HAL_sysSetMacAddr((UINT8*)macAddr);

	    if (CMC_eocSetChipMac(macStr) == VOS_RET_SUCCESS)
	    {
	        printf("set mac succcess\n");
	    }
	    else
	    {
	        printf("set mac fail\n");
	        return VOS_RET_INTERNAL_ERROR;
	    }
	}
	else
	{
	    ret = cli_handleOntInfoGetMac(&mac1, &mac2);
	    if (VOS_RET_SUCCESS == ret)
	    {
	        if (0 == util_strncmp(argv[0], "1",1))
	        {
	            ret = cli_handleStrToMac(argv[1],&mac1);
	        }
	        else
	        if (0 == util_strncmp(argv[0], "2",1))
	        {
	            ret = cli_handleStrToMac(argv[1],&mac2);
	        }
	        else
	        {
	            ret = VOS_RET_INVALID_ARGUMENTS;
	        }
	    }

	    if (VOS_RET_SUCCESS == ret)
	    {
	        ret = cli_handleOntInfoSetMac(&mac1, &mac2);
	    }

	    if (VOS_RET_SUCCESS != ret)
	    {
	        printf("Set MAC%s Failed\r\n", argv[0]);
	    }
	}
    return ret;
}


VOS_RET_E cli_handleWriteCpuMac(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    MAC_ADDR mac1, mac2;

    if ((1 != argc) || (17 != strlen(argv[0])))
    {
        printf("format:aa:bb:cc:dd:ee:ff\r\n");
        return VOS_RET_INVALID_ARGUMENTS;
    }

    ret = cli_handleOntInfoGetMac(&mac1, &mac2);
    if (VOS_RET_SUCCESS == ret)
    {
        ret = cli_handleStrToMac(argv[0],&mac1);
    }

    if (VOS_RET_SUCCESS == ret)
    {
        ret = cli_handleOntInfoSetMac(&mac1, &mac2);
        printf("set uni mac success!\r\n");
    }

    if (VOS_RET_SUCCESS != ret)
    {
        printf("set uni mac addr:%02x:%02x:%02x:%02x:%02x:%02x invalid\r\n", 
                mac1.addr[0],mac1.addr[1],mac1.addr[2],
                mac1.addr[3],mac1.addr[4],mac1.addr[5]);
    }
    
    return ret;
}


VOS_RET_E cli_handleReadCpuMac(VTY_T *vty, int argc, char **argv)
{
	cli_handleGetMacAddr(vty,argc,argv);

	return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleOntInfoSetDefault(VOID)
{
    ONT_INFO_T ontInfo;

    memset(&ontInfo, 0, sizeof(ontInfo));
    ontInfo.crc = 0;
    ontInfo.sn.sn[0] = 'T';
    ontInfo.sn.sn[1] = 'W';
    ontInfo.sn.sn[2] = 'S';
    ontInfo.sn.sn[3] = 'H';
    ontInfo.sn.sn[4] = 0;
    ontInfo.sn.sn[5] = 0;
    ontInfo.sn.sn[6] = 0;
    ontInfo.sn.sn[7] = 1;
    
    ontInfo.productCode.codeClass   = ONT_PRODUCT_CODE_UNKNOWN; 
    ontInfo.productCode.codeHwInfo  = ONT_PRODUCT_CODE_UNKNOWN; 
    ontInfo.productCode.codePortMap = ONT_PRODUCT_CODE_UNKNOWN; 
    ontInfo.productCode.codeExtend  = ONT_PRODUCT_CODE_UNKNOWN; 
    
    memset(&(ontInfo.mac1), 0, MAC_ADDR_LEN);
    memset(&(ontInfo.mac2), 0, MAC_ADDR_LEN);

    return cli_handleOntInfoWrite(&ontInfo);
}


VOS_RET_E cli_handleWriteProduct(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_PRODUCT_CODE_T productCode;
	ONT_INFO_T ontInfo;

    if (4 != argc)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    productCode.codeClass   = strtoul(argv[0], NULL, 16);
    productCode.codeHwInfo  = strtoul(argv[1], NULL, 16);
    productCode.codePortMap = strtoul(argv[2], NULL, 16);
    productCode.codeExtend  = strtoul(argv[3], NULL, 16);

	ret = cli_handleOntInfoRead(&ontInfo);

    if (VOS_RET_SUCCESS != ret)
    {
        ret = cli_handleOntInfoSetDefault();
    }
    
    if (VOS_RET_SUCCESS == ret)
    {
        ontInfo.productCode.codeClass   = productCode.codeClass;
        ontInfo.productCode.codeHwInfo  = productCode.codeHwInfo;
        ontInfo.productCode.codePortMap = productCode.codePortMap;
        ontInfo.productCode.codeExtend  = productCode.codeExtend; 
        
        ret = cli_handleOntInfoWrite(&ontInfo);
    }
    
    return ret;
}


VOS_RET_E cli_handleOntInfoSetPassword(PON_PASSWORD *pPassword)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;
    
    if (NULL == pPassword)
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&ontInfo);

    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(&(ontInfo.password), pPassword, sizeof(PON_PASSWORD));
        ret = cli_handleOntInfoWrite(&ontInfo);
    }

    return ret;
}


VOS_RET_E cli_handleWritePassword(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    PON_PASSWORD password;
    UINT8 len = strlen(argv[0]), i = 0;

    (void) argc;

    if (len > 10)
    {
        printf("Password formate: 10 characters; sample: ABCDEFGHIJ\r\n");
        return VOS_RET_INVALID_ARGUMENTS;
    }

    /*if (0 == access(PON_LINK_UP_FLAG, 0))
    {
        printf("Password modification not allowed when pon link is up\r\n");
        return SYS_ERR_STATE;
    }*/

    memset(&password, 0, PASSWORD_LEN);
    memcpy(&password, argv[0], len);

    for (i = 0; i < len; i++)
    {
        if ((argv[0][i] >= '0' && argv[0][i] <= '9') || (argv[0][i] >= 'a' && argv[0][i] <= 'z')
             || (argv[0][i] >= 'A' && argv[0][i] <= 'Z'))
        {
            continue;
        }
        else
        {
            printf("Invalid Character(s).\r\n");
            return VOS_RET_INVALID_ARGUMENTS;
        }
    }

    ret = cli_handleOntInfoSetPassword(&password);
    if (VOS_RET_SUCCESS != ret)
    {
        printf("Write password failed\r\n");
    }
    else
    {
        printf("Write password success!\r\n");
    }
    
    return ret;
}


VOS_RET_E cli_handleOntInfoGetPassword(PON_PASSWORD *pPassword)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;
    
    if (NULL == pPassword)
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&ontInfo);

    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(pPassword, &(ontInfo.password), sizeof(PON_PASSWORD));
    }

    return ret;

}


VOS_RET_E cli_handleReadPassword(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    PON_PASSWORD password;

    (void)argc;
    (void)argv;
    
    if (VOS_RET_SUCCESS == cli_handleOntInfoGetPassword(&password))
    {
        UINT8 i = 0;
        printf("ONT password: ");
        for (i = 0; i < PASSWORD_LEN; i++)
        {
            if (0 == password.pwd[i])
            {
                printf(" ");
            }
            else
            {
                printf("%c", password.pwd[i]);
            }
        }
        printf("\r\n");
    }
    else
    {
        printf("Get ONT password failed\r\n");
    }
    
    return ret;
}


VOS_RET_E cli_handleStrToPassword(const char* str, PON_PASSWORD *pPassword)
{
    UINT8  i;
    char   hexStr[3];
    char   tmpStr[PASSWORD_LEN * 2];
    
    if ((NULL == pPassword)||(NULL == str))
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    memset(tmpStr, '0', sizeof(tmpStr));
    if (PASSWORD_LEN * 2 < strlen(str))
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    memcpy(tmpStr, str, strlen(str));

    for (i = 0; i < 2*PASSWORD_LEN; i++)
    {
        if ((tmpStr[i] >= '0' && tmpStr[i] <= '9') || (tmpStr[i] >= 'A' && tmpStr[i] <= 'F') || (tmpStr[i] >= 'a' && tmpStr[i] <= 'f'))
        {
            continue;
        }
        else
        {
            return VOS_RET_INTERNAL_ERROR;
        }
    }

    memset(hexStr, 0x00, 3);
    for (i = 0; i < PASSWORD_LEN; i++)
    {
        memcpy(hexStr, tmpStr+(2 * i), 2);
        pPassword->pwd[i] = (unsigned char)strtoul(hexStr, NULL, 16);
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleWritePasswordHex(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    PON_PASSWORD password;

    if ((1 != argc) || (2*PASSWORD_LEN < strlen(argv[0])))
    {
        printf("Password formate: 20 hex value; sample: 00112233445566778899\r\n");
        return VOS_RET_INTERNAL_ERROR;
    }

    /*if (0 == access(PON_LINK_UP_FLAG, 0))
    {
        printf("Password modification not allowed when pon link is up\r\n");
        return SYS_ERR_STATE;
    }*/

    ret = cli_handleStrToPassword(argv[0], &password);
    if (VOS_RET_SUCCESS == ret)
    {
        ret = cli_handleOntInfoSetPassword(&password);
        printf("Write password success!\r\n");
    }

    if (VOS_RET_SUCCESS != ret)
    {
        printf("Write password failed. Please check the format. Sample: 001133557799aaccffee\r\n");
    }
    
    return ret;
}


VOS_RET_E cli_handleReadPasswordHex(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    PON_PASSWORD password;

    (void)argc;
    (void)argv;
    
    if (VOS_RET_SUCCESS == cli_handleOntInfoGetPassword(&password))
    {
        printf("ONT password:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
               password.pwd[0], password.pwd[1], password.pwd[2], password.pwd[3],password.pwd[4],
               password.pwd[5], password.pwd[6], password.pwd[7], password.pwd[8],password.pwd[9]);
    }
    else
    {
        printf("Get ONT password failed\r\n");
    }
    
    return ret;
}


VOS_RET_E cli_handleOntInfoGetCtc(ONT_CTC_T *pCtc)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;

    if (NULL == pCtc)
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&ontInfo);
    if (VOS_RET_SUCCESS == ret)
    {        
        memcpy(pCtc, &ontInfo.ctcLoid, sizeof(ONT_CTC_T));
    }

    return ret;
}


VOS_RET_E cli_handleOntInfoSetCtc(ONT_CTC_T *pCtc)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;
    
    if (NULL == pCtc)
    {
        return VOS_RET_INTERNAL_ERROR;
    }    

    ret = cli_handleOntInfoRead(&ontInfo);
    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(&ontInfo.ctcLoid, pCtc, sizeof(ONT_CTC_T));
        ret = cli_handleOntInfoWrite(&ontInfo);
    }

    return ret;
}


VOS_RET_E cli_handleSetCtcInfoByStr(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_CTC_T ctc;
    (void)argv;
    
    memset(&ctc, 0, sizeof(ctc));
    
    ret = cli_handleOntInfoGetCtc(&ctc);
    if (VOS_RET_SUCCESS == ret)
    {
        if (0 == cli_handleStrNcaseCmp("loid", argv[0]))
        {
            strncpy(ctc.loid, argv[1], sizeof(ctc.loid) - 1);
        }
        else
        {
            strncpy(ctc.password, argv[1], sizeof(ctc.password) - 1);
        }
        
        ret = cli_handleOntInfoSetCtc(&ctc);
        if (VOS_RET_SUCCESS != ret)
        {
            printf("Set CTC %s Failed", argv[0]);
        }
    }
    else
    {
        printf("Set ONT CTC %s failed\r\n", argv[0]);
    }
    
    return ret;
}

VOS_RET_E cli_handleShowCtcInfoStr(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_CTC_T ctc;
    (void)argc;
    (void)argv;
    
    memset(&ctc, 0, sizeof(ctc));
    
    if (VOS_RET_SUCCESS == cli_handleOntInfoGetCtc(&ctc))
    {
        printf("ONT CTC Loid    : %s\r\n", ctc.loid);
        printf("ONT CTC Password: %s\r\n", ctc.password);
    }
    else
    {
        printf("Get ONT CTC info failed\r\n");
    }
    
    return ret;
}


VOS_RET_E cli_handleStrToHex(const char *str, UINT8 *pHexBuffer, UINT32 size)
{
    UINT32 idx    = 0;
    UINT32 strLen = 0;
    
    if ((NULL == pHexBuffer)||(NULL == str))
    {
        return VOS_RET_INTERNAL_ERROR;
    }
    
    strLen  = strlen(str);
    strLen -= strLen & 1;
    if (size * 2 < strLen)
    {
        strLen = size * 2;
    }

    for (idx = 0; idx < strLen; idx++)
    {
        if ((str[idx] >= '0' && str[idx] <= '9') 
         || (str[idx] >= 'A' && str[idx] <= 'F') 
         || (str[idx] >= 'a' && str[idx] <= 'f'))
        {
            continue;
        }
        else
        {
            return VOS_RET_INTERNAL_ERROR;
        }
    }
    
    for (idx = 0; idx < strLen/2; idx++)
    {
        char hexStr[3];
        
        hexStr[0] = str[idx * 2 + 0];
        hexStr[1] = str[idx * 2 + 1];
        hexStr[2] = 0;
        
        pHexBuffer[idx] = (unsigned char)strtoul(hexStr, NULL, 16);
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleSetCtcInfoByHex(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_CTC_T ctc;
    
    memset(&ctc, 0, sizeof(ctc));
    
    ret = cli_handleOntInfoGetCtc(&ctc);
    if (VOS_RET_SUCCESS == ret)
    {
        if (0 == cli_handleStrNcaseCmp("loid", argv[0]))
        {
            memset(ctc.loid, 0, sizeof(ctc.loid));
            cli_handleStrToHex(argv[1], (UINT8 *)ctc.loid, sizeof(ctc.loid) - 1);
        }
        else
        {
            memset(ctc.password, 0, sizeof(ctc.password));
            cli_handleStrToHex(argv[1], (UINT8 *)ctc.password, sizeof(ctc.password) - 1);
        }
        
        ret = cli_handleOntInfoSetCtc(&ctc);
        if (VOS_RET_SUCCESS != ret)
        {
            printf("Set CTC %s Failed", argv[0]);
        }
    }
    else
    {
        printf("Set ONT CTC %s failed\r\n", argv[0]);
    }
    
    return ret;
}


void cli_handleSvcPrintHex(void *buf, UINT32 len)
{
    UINT32 idx = 0;
    char *p = (char *)buf;

    if (NULL == buf || len == 0)
    {
        return;
    }
    
    while (idx < len)
    {
        UINT8 c = p[idx];
        printf("%02X", c);
        idx++;
    }
    printf("\r\n");
}


VOS_RET_E cli_handleShowCtcInfoHex(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_CTC_T ctc;
    (void)argc;
    (void)argv;
    
    memset(&ctc, 0, sizeof(ctc));
    if (VOS_RET_SUCCESS == cli_handleOntInfoGetCtc(&ctc))
    {
        printf("ONT CTC Loid    : ");
        cli_handleSvcPrintHex(ctc.loid, sizeof(ctc.loid) - 1);
        printf("ONT CTC Password: ");
        cli_handleSvcPrintHex(ctc.password, sizeof(ctc.password) - 1);
    }
    else
    {
        printf("Get ONT CTC info failed\r\n");
    }
    
    return ret;
}


#ifdef PFM_HGU
VOS_RET_E cli_handleLaserCtrl(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (0 == util_strcmp(argv[0], "on"))
    {
        ret = HAL_gponSetTxTest(TRUE);
    }
    else  if (0 == util_strcmp(argv[0], "off"))
    {
        ret = HAL_gponSetTxTest(FALSE);
    }

	return ret;
}

VOS_RET_E cli_handleLaserPrbsCtrl(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;

	if (0 == util_strcmp(argv[0], "on"))
    {
        ret = HAL_gponSetTxPrbsMode(1);
    }
    else  if (0 == util_strcmp(argv[0], "off"))
    {
        ret = HAL_gponSetTxPrbsMode(0);
    }

	return ret;
}

VOS_RET_E cli_handleLaserOndefaultMode(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    FILE *fp = NULL;
    char temp[128];

    fp = fopen("/var/config/laseron", "w+");

    if(fp == NULL)
    {
        printf("create /var/config/laseron   fail \r\n");
    }
    
    if (0 == util_strcmp(argv[0], "on"))
    {
        UTIL_SNPRINTF(temp, sizeof(temp), "laser defalut : enable\r\n");
        ret = HAL_gponSetTxTest(TRUE);
    }
    else  if (0 == util_strcmp(argv[0], "off"))
    {
        UTIL_SNPRINTF(temp, sizeof(temp), "laser defalut : disable\r\n");
        ret = HAL_gponSetTxTest(FALSE);
    }
    
    fputs(temp, fp);
    fclose(fp);
    return ret;
}

VOS_RET_E cli_handlePerfTestMode(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    FILE *fp = NULL;
    char temp[128]={0};

    fp = fopen("/var/config/perfTest", "w+");

    if(fp == NULL)
    {
        printf("create /var/config/perfTest   fail \r\n");
    }
    
    if (0 == util_strcmp(argv[0], "on"))
    {
        UTIL_SNPRINTF(temp, sizeof(temp), "Test");
    }
    else  if (0 == util_strcmp(argv[0], "off"))
    {
        UTIL_SNPRINTF(temp, sizeof(temp), "Normal");
    }
    
    fputs(temp, fp);
    fclose(fp);
    return ret;
}


VOS_RET_E cli_BobReadreg(VTY_T *vty, int argc, char **argv)
{
#if 1
	VOS_RET_E ret = VOS_RET_SUCCESS;  
    UINT32 i2caddr;
    UINT32 i2coffset;	
    UINT8 data[256];	

    if(argc != 2)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    i2caddr= strtoul(argv[0], NULL, 0);
    i2coffset= strtoul(argv[1], NULL, 0);
    ret = HAL_sysI2CReadReg(i2caddr,i2coffset,1,data);
    if(ret == VOS_RET_SUCCESS)
    {
        printf("bobtest read_reg %d %d %d\n", i2caddr, i2coffset, data[0]); 
    }
    return ret;
#else
//printf("===%s(...): argc is [%d] ===\n", __FUNCTION__, argc);
	//char cmdbuf[256];
    if(argc == 2)
    {
		//printf("===%s(...): argv[0] is %s, argv[1] is %s ===\n", __FUNCTION__, argv[0], argv[1]);
		UTIL_DO_SYSTEM_ACTION("echo a %s %s 1 > /proc/i2c_gpon/gponPhyTest | grep ""="" ", argv[0], argv[1]);
		//sprintf(cmdbuf, "echo a %s %s 1 > /proc/i2c_gpon/gponPhyTest | grep ""="" ", argv[0], argv[1]);
		//if(!system(cmdbuf))
		{
			return VOS_RET_SUCCESS;
		}
		//return VOS_RET_INVALID_ARGUMENTS;
		
    }
	else
	{
		printf("Error: Need just 2 arguments \n");
		return VOS_RET_INVALID_ARGUMENTS;
    }
#endif
}

VOS_RET_E cli_BobReadregs(VTY_T *vty, int argc, char **argv)
{
#if 1
	VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT32 i;
    int len;	
    UINT32 i2caddr, i2coffset;	
    UINT8 data[256];	

    if(argc != 3)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    i2caddr= strtoul(argv[0], NULL, 0);
    i2coffset= strtoul(argv[1], NULL, 0);
    len  = strtoul(argv[2], NULL, 0);


    ret = HAL_sysI2CReadReg(i2caddr,i2coffset,len,data);
    if(ret == VOS_RET_SUCCESS)
    {
        printf("bobtest read_regs %d %d ", i2caddr, i2coffset);
        for(i=0; i< len; i++)
        {

            printf("0x%02x ",data[i]);
        }
        printf("\n");    
    }
    return ret;
#else
	//printf("===%s(...): argc is [%d] ===\n", __FUNCTION__, argc);
	//char cmdbuf[256];
    if(argc == 3)
    {
		//printf("===%s(...): argv[0] is %s, argv[1] is %s, argv[2] is %s ===\n",  __FUNCTION__, argv[0], argv[1], argv[2]);
		UTIL_DO_SYSTEM_ACTION("echo a %s %s %s > /proc/i2c_gpon/gponPhyTest | grep ""="" ", argv[0], argv[1], argv[2]);
		//sprintf(cmdbuf, "echo a %s %s 1 > /proc/i2c_gpon/gponPhyTest | grep ""="" ", argv[0], argv[1]);
		//if(!system(cmdbuf))
		{
			return VOS_RET_SUCCESS;
		}
		//return VOS_RET_INVALID_ARGUMENTS;
	}
	else
    {	
		printf("Error: Need just 3 arguments \n");
		return VOS_RET_INVALID_ARGUMENTS;
    }

#endif
}

VOS_RET_E cli_BobWritereg(VTY_T *vty, int argc, char **argv)
{
#if 1
	UINT32 i2caddr;
    UINT32 i2coffset;	
    UINT8  data[256];	
    UINT8  rdata[256];
    
    if(argc != 3)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    i2caddr= strtoul(argv[0], NULL, 0);
    i2coffset= strtoul(argv[1], NULL, 0);
    data[0] = strtoul(argv[2], NULL, 0);
    HAL_sysI2CWriteReg(i2caddr,i2coffset,1,data); 
    usleep(2000);
    HAL_sysI2CReadReg(i2caddr,i2coffset,1,rdata);
    if(data[0] != rdata[0])
    {
        printf("bob write reg fail ,addr 0x%02x offset 0x%02x dataw 0x%02x datar 0x%02x \r\n",(SINT32)i2caddr,(SINT32)i2coffset,(SINT8)data[0],(SINT8)rdata[0]);
    }
    return VOS_RET_SUCCESS;
    
#else
//printf("===%s(...): argc is [%d] ===\n", __FUNCTION__, argc);
    if(argc == 3)
    {
		//printf("===%s(...): argv[0] is %s, argv[1] is %s, argv[2] is %s ===\n",  __FUNCTION__, argv[0], argv[1], argv[2]);
		UTIL_DO_SYSTEM_ACTION("echo a %s %s 1 %s > /proc/i2c_gpon/gponPhyTest", argv[0], argv[1], argv[2]);
		 return VOS_RET_SUCCESS;
	}
	else
    {
		printf("Error: Need just 3 arguments \n");
		return VOS_RET_INVALID_ARGUMENTS;
    }

#endif
}

VOS_RET_E cli_BobWriteregs(VTY_T *vty, int argc, char **argv)
{
#if 1
	UINT32 i ;
    UINT32 len;		
    UINT32 i2caddr;
    UINT32 i2coffset;	
    UINT8  data[256];	
    UINT8  rdata[256];
    UINT8  strdata[10]={0};
    UINT8  *datastring = NULL;
    

    if(argc != 4)
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    i2caddr= strtoul(argv[0], NULL, 0);
    i2coffset= strtoul(argv[1], NULL, 0);
    len = strtoul(argv[2], NULL, 0);
    
    if ( len*4 != strlen(argv[3]))
    {
        printf("len = %d, but val is not enough \n", len);
        return VOS_RET_INVALID_ARGUMENTS;
    }
    datastring = (UINT8 *)argv[3];
    for (i = 0; i < len; i++)
    {   
        UTIL_STRNCPY((char *)strdata,(char *)datastring,5);        
        data[i] = strtoul((char*)strdata, NULL, 0);
        datastring += 4;
    }
    HAL_sysI2CWriteReg(i2caddr,i2coffset,len,data); 
    usleep(2000);
    HAL_sysI2CReadReg(i2caddr,i2coffset,len,rdata);
    for (i=0 ;i < len ; i++ )
    {
        if(data[i] != rdata[i])
        {
            printf("bob write reg fail ,addr 0x%02x offset 0x%02x dataw 0x%02x datar 0x%02x \r\n",(SINT32)i2caddr,(SINT32)(i2coffset+i),(SINT8)data[i],(SINT8)rdata[i]);
        }
    }
    return VOS_RET_SUCCESS;
#else
//printf("===%s(...): argc is [%d] ===\n", __FUNCTION__, argc);
    if(argc == 4)
    {
		//printf("===%s(...): argv[0] is %s, argv[1] is %s, argv[2] is %s, argv[3] is %s ===\n",  __FUNCTION__, argv[0], argv[1], argv[2], argv[3]);
		UTIL_DO_SYSTEM_ACTION("echo a %s %s %s %s > /proc/i2c_gpon/gponPhyTest", argv[0], argv[1], argv[2], argv[3]);
		 return VOS_RET_SUCCESS;
	}
	else
    {
		printf("Error: Need just 4 arguments \n");
		return VOS_RET_INVALID_ARGUMENTS;
    }

#endif
}


VOS_RET_E cli_BobShowDataParam(VTY_T *vty, int argc, char **argv)
{
    HAL_sysShowPonDataParam();
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_bobTransceiverState(VTY_T *vty, int argc, char **argv)
{
    float fParamValue           = 0.0;    
    UINT16 paramRegValue = 0;
    
    printf("\r\n");
    HAL_sysI2CReadReg(0x51,96,2,(UINT8*)&paramRegValue);
    fParamValue = (float)(((float)(paramRegValue & 0xFF))/256);
    if (paramRegValue & 0x8000)
    {   
        fParamValue = fParamValue*(-1) - (float)(((~paramRegValue) >>8) & 0xff);   
    }
    else
    {
        fParamValue += (float)((paramRegValue>>8)&0xff);
    }    
    printf("Temperature : %.2f Degrees\r\n", fParamValue);

    HAL_sysI2CReadReg(0x51,98,2,(UINT8*)&paramRegValue);
    fParamValue = ((float)paramRegValue) /10000;
    printf("VCC         : %.2f Volts\r\n",   fParamValue);

    HAL_sysI2CReadReg(0x51,100,2,(UINT8*)&paramRegValue);
    fParamValue = (float)(0.002 * paramRegValue);
    printf("TX Bias     : %.2f mA\r\n",      fParamValue);

    HAL_sysI2CReadReg(0x51,102,2,(UINT8*)&paramRegValue);
    if (paramRegValue)
    {
        fParamValue = (float)(10 * log10(((float)paramRegValue) * 0.0001));
    }
    printf("TX Power    : %.2f dBm\r\n",     fParamValue);

    HAL_sysI2CReadReg(0x51,104,2,(UINT8*)&paramRegValue);
    if (paramRegValue)
    {
        fParamValue = (float)(10 * log10(((float)paramRegValue) * 0.0001));
    }
    printf("RX Power    : %.2f dBm\r\n",     fParamValue);
    printf("\r\n");
    return VOS_RET_SUCCESS;
}

#endif


VOS_RET_E cli_handleSetProductClass(VTY_T *vty, int argc, char **argv)
{
	return HAL_sysSetProductClass(argv[0]);
}


VOS_RET_E cli_handleSetDeviceId(VTY_T *vty, int argc, char **argv)
{
	return HAL_sysSetDeviceId(argv[0]);
}


VOS_RET_E cli_handleRemoteResetDefault(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    
    vosLog_debug("Enter>, argc = %d", argc);

    if ((1 == argc) && util_strstr(argv[0], "Y"))
    {
        ret = CMC_sysResetConfig(CMC_SYS_CONFIG_RESET_REMOTE);

        if (VOS_RET_SUCCESS != ret)
        {
            vosLog_error("CMC_sysResetConfig failed, ret = %d", ret);
            return ret;
        }

        if (SF_FEATURE_LOCATION_FUJIAN)
        {
            HAL_sysSetLoidPwd(NULL, NULL);
        }
        
        HAL_Reboot_System();
    }

    return ret;
}


VOS_RET_E cli_handleDownloadImage(VTY_T *vty, int argc, char **argv)
{
    char cmd[BUFLEN_512] = {0};
    char desFilePath[BUFLEN_128] = "/tmp/image";
    char *ptr = NULL;
    struct stat fileStat;
    VOS_RET_E ret = VOS_RET_SUCCESS;
    VOS_IAMGE_FORMAT format = VOS_FLASH_PARTITION_NONE;
        
    vosLog_debug("Enter>, argc = %d, argv = %p", argc, argv);

    if (argc < 3)
    {
        vosLog_error("need more arguments input");
        return ret;
    }
    
    if (!util_strcmp(argv[0], "ftp"))
    {
        UTIL_SNPRINTF(cmd, sizeof(cmd), "wget ftp://%s:21/%s -O %s", argv[1], argv[2], desFilePath);
    }
    else
    {
        UTIL_SNPRINTF(cmd, sizeof(cmd), "tftp -g -r %s -l %s %s", argv[2], desFilePath, argv[1]);
    }

    vosLog_debug("cmd = %s", cmd);
    UTIL_DO_SYSTEM_ACTION(cmd);
    
    if (0 > stat(desFilePath, &fileStat))
    {
        vosLog_error("%s: get download file size error! errorCode", argv[0]);
        return VOS_RET_INVALID_IMAGE;
    }
    else
    {
        vosLog_debug("%s: get download file size %d", argv[0], (UINT32)fileStat.st_size);
        ptr = (char *)vomMem_mapFile2Mem(desFilePath, (UINT32)fileStat.st_size);
        if (NULL != ptr)
        {
            ret = HAL_flashValidaImage(ptr, (UINT32)fileStat.st_size, &format);
            if (VOS_RET_SUCCESS != ret)
            {
                vosLog_error("invalid image, ret = %d", ret);
                return ret;
            }
            
            ret = HAL_flashWriteImage(ptr, (UINT32)fileStat.st_size);
            if (VOS_RET_SUCCESS != ret)
            {
                vosLog_error("%s: write to flash error, ret = %d", argv[0], ret);
                return ret;
            }
            else
            {
                HAL_Reboot_System();
            }
        }
        else
        {
            vosLog_error("%s: mapping file to Memory error!", argv[0]);
            return VOS_RET_INTERNAL_ERROR;
        }
    }
  
    return ret;
}


VOS_RET_E cli_handleSetHwVersion(VTY_T *vty, int argc, char **argv)
{
    UINT8 hwVersion[32];

	if (!SF_FEATURE_UPLINK_TYPE_EOC)
	{
		VOS_RET_E ret = VOS_RET_SUCCESS;
	    ONT_INFO_T ontInfo;
		char  ontgVer[ONTGVERSION_LEN];

	    if ((1 != argc) || (strlen(argv[0]) > 14))
	    {
	        printf("set Hardware version failed!\r\n");
	        return VOS_RET_INVALID_ARGUMENTS;
	    }

	    memset(ontgVer, 0, sizeof(ontgVer));
	    memcpy(ontgVer, argv[0], strlen(argv[0]));

	    if (NULL == ontgVer)
	    {
	        return VOS_RET_INTERNAL_ERROR;
	    }

	    ret = cli_handleOntInfoRead(&ontInfo);
	    if (VOS_RET_SUCCESS == ret)
	    {
	        memcpy(ontInfo.ontgVer, ontgVer, sizeof(ontInfo.ontgVer));
	        ret = cli_handleOntInfoWrite(&ontInfo);
	    }
	}
	else
	{
	    if (argc != 1)
	    {
	        return VOS_RET_INVALID_ARGUMENTS;
	    }
	    
	    memset(hwVersion, 0, sizeof(hwVersion));

	    UTIL_STRNCPY((char*)hwVersion, argv[0], sizeof(hwVersion));

	    if (HAL_sysSetHwVersion(hwVersion) != VOS_RET_SUCCESS)
	    {
                printf("fail\n");
	        vosLog_error("Set hardware version error \n");
	        return VOS_RET_INTERNAL_ERROR;
	    }
	}
    printf("success\n");
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleGetHwVersion(VTY_T *vty, int argc, char **argv)
{

    UINT8 hwVersion[32];

	if (!SF_FEATURE_UPLINK_TYPE_EOC)
	{
		VOS_RET_E ret = VOS_RET_SUCCESS;
		ONT_INFO_T ontInfo;

		ret = cli_handleOntInfoRead(&ontInfo);
	    if (VOS_RET_SUCCESS == ret)
	    {
			printf("ONT Hardware version: %s\r\n", ontInfo.ontgVer);
	    }
		else
	    {
	        printf("Get ONT Hardware version failed\r\n");
			return ret;
	    }
	}
	else
	{
	    memset(hwVersion, 0, sizeof(hwVersion));

	    if (HAL_sysGetHwVersion(hwVersion) != VOS_RET_SUCCESS)
	    {
	        vosLog_error("Get hardware version error \n");
	        return VOS_RET_INTERNAL_ERROR;
	    }

	    printf("%s\n", hwVersion);
	}
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleOntInfoSetEqpt(char *pEqpt)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    ONT_INFO_T ontInfo;

    if (NULL == pEqpt)
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = cli_handleOntInfoRead(&ontInfo);
    if (VOS_RET_SUCCESS == ret)
    {
        memcpy(ontInfo.eqptId, pEqpt, sizeof(ontInfo.eqptId));
        ret = cli_handleOntInfoWrite(&ontInfo);
    }

    return ret;
}


VOS_RET_E cli_handleWriteEquipmentId(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
    UINT8 equipmentId[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if ((1 != argc) || (20 < strlen(argv[0])))
    {
        printf("set equipment id failed!\r\n");
        return VOS_RET_INVALID_ARGUMENTS;
    }
    
    memcpy(equipmentId, argv[0], strlen(argv[0]));
    ret = cli_handleOntInfoSetEqpt((char *)equipmentId);
    if (VOS_RET_SUCCESS != ret)
    {
        printf("set equipment id failed!\r\n");
    }
    else
    {
        printf("set equipment id success!\r\n");
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleReadEquipmentId(VTY_T *vty, int argc, char **argv)
{
	VOS_RET_E ret = VOS_RET_SUCCESS;
	ONT_INFO_T ontInfo;
    
    ret = cli_handleOntInfoRead(&ontInfo);
    if (VOS_RET_SUCCESS == ret)
    {
        printf("ONT Equipment Id: %s\r\n", ontInfo.eqptId);
    }
    else
    {
        printf("Get ONT Equipment Id failed\r\n");
    }
    
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleShowUpsState(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    int istate1 = 0;
    int istate2 = 0;
    int istate3 = 0;
    int istate4 = 0;

    if (SF_DATA_MODEL_OVERSEA)
    {
        HAL_GetGpioState(CLI_UPS_GPIO_1, &istate1);
        HAL_GetGpioState(CLI_UPS_GPIO_2, &istate2);
        HAL_GetGpioState(CLI_UPS_GPIO_3, &istate3);
        HAL_GetGpioState(CLI_UPS_GPIO_4, &istate4);

        printf("GPIO%d %s GPIO%d %s GPIO%d %s GPIO%d %s\n", CLI_UPS_GPIO_1, istate1?"HIGH":"LOW", CLI_UPS_GPIO_2, istate2?"HIGH":"LOW", CLI_UPS_GPIO_3, istate3?"HIGH":"LOW", CLI_UPS_GPIO_4, istate4?"HIGH":"LOW");
    }

    return ret;
}


VOS_RET_E cli_handlePonSwitch(VTY_T *vty, int argc, char **argv)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;

    if (util_strcmp(argv[0], "on"))
    {   
        vosLog_notice("set gpio 18 high");
        HAL_SetGpioState(18, 1);
    }
    else
    {   
        vosLog_notice("set gpio 18 low");
        HAL_SetGpioState(18, 0);
    }

    return ret;
}


VOS_RET_E cli_handleNetSniffer(VTY_T *vty, int argc, char **argv)
{
    VosMsgHeader msg;

    memset(&msg, 0, sizeof(msg));

    msg.type = VOS_MSG_NET_SNIFFER_CONSOLE;
    msg.src = vosMsg_getHandleEid(msgHandle);
    msg.dst = EID_NET_SNIFFER;
    msg.flags_bounceIfNotRunning = 1;

    if (0 == util_strcasecmp("macList", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_MACLIST;
    }
    else if (0 == util_strcasecmp("pcInfo", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_PCINFO;
    }
    else if (0 == util_strcasecmp("uploadInfo", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_UPLOADINFO;
    }
    else if (0 == util_strcasecmp("httpInfo", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_HTTPINFO;
    }
    else if (0 == util_strcasecmp("tcpInfo", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_TCPINFO;
    }
    else if (0 == util_strcasecmp("dnsInfo", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_DNSINFO;
    }
    else if (0 == util_strcasecmp("threadInfo", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_THREADINFO;
    }
    else if (0 == util_strcasecmp("classfyRules", argv[0]))
    {
        msg.wordData = CONSOLE_CONTROL_TYPE_SHOW_CLASSFYRULES;
    }

    msg.dataLength = 0;

    return vosMsg_send(msgHandle, &msg);
}

#endif
static CLI_CMD_T sg_cmdTable[] = 
{
    {
         "linuxshell",
         "Enter linux shell\n",
         (CLI_FUNC)cli_handleLinuxShell,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
        "getfactory",
        "get factory mode\n",
        (CLI_FUNC)cli_handleGetFactoryMode,
        (CLI_RUNTIME_FUNC)NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "checkdisk",
        "check sata disk is ready or not\n",
        (CLI_FUNC)cli_handleCheckDisk,
        (CLI_RUNTIME_FUNC)NULL,
        CLI_LEVEL_DEBUG
    },

    {
         "logdest set (timertask|consoled|hal|httpd|mcpd|pppd|smd|sntp|ssk|telnetd|tr69c|upnp|urlfilterd|vodsl|eponapp|mmed|net_sniffer) <address>",
         "set app log dest\n"
         "set app log dest\n"
         "app name\n"
         "dest ip\n",
         (CLI_FUNC)cli_handleLogLevel,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
#if 0

    {
         "loglevel set (cmc|consoled|hal|httpd|mcpd|pppd|smd|sntp|ssk|telnetd|tr69c|upnp|urlfilterd|vodsl|eponapp|mmed|net_sniffer) (error|notice|debug)",
         "App log level\n"
         "Set app log level\n"
         "App name\n"
         "Log level\n",
         (CLI_FUNC)cli_handleLogLevel,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "logdest get (smd|ssk|cmc|consoled|tr69c|mcpd)",
         "App log destnation\n"
         "Get app log destnation\n"
         "App name\n",
         (CLI_FUNC)cli_handleLogDest,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "logdest set (smd|ssk|cmc|consoled|tr69c|mcpd) (stderror|syslog|telnet)",
         "App log destnation\n"
         "Set app log destnation\n"
         "App name\n"
         "Log level\n",
         (CLI_FUNC)cli_handleLogDest,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "mdm set $object_id $instance_id $param_name $param_value",
         "mdm\n"
         "mdm set\n"
         "object id\n"
         "instance id\n"
         "param name\n"
         "param value\n",
         (CLI_FUNC)cli_handleSetMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleSetMdmObjectRT,
         CLI_LEVEL_DEBUG
    },

    {
         "mdm set $object_path $param_name $param_value",
         "mdm\n"
         "mdm set\n"
         "object path\n"
         "param name\n"
         "param value\n",
         (CLI_FUNC)cli_handleSetMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleSetMdmPathRT,
         CLI_LEVEL_DEBUG
    },
    
    {
         "mdm save",
         "mdm\n"
         "mdm save\n",
         (CLI_FUNC)cli_handleSaveMdm,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show mdm $object_id $instance_id",
         "Show\n"
         "Show mdm\n"
         "object id\n"
         "instance id\n",
         (CLI_FUNC)cli_handleShowMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleShowMdmObjectRT,
         CLI_LEVEL_DEBUG
    },

    {
         "show mdm $object_path",
         "Show\n"
         "Show mdm\n"
         "object path\n",
         (CLI_FUNC)cli_handleShowMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleShowMdmPathRT,
         CLI_LEVEL_DEBUG
    },

    {
         "mdm add $object_id $instance_id",
         "mdm\n"
         "mdm add\n"
         "object id\n"
         "instance id\n",
         (CLI_FUNC)cli_handleAddMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleAddMdmObjectRT,
         CLI_LEVEL_DEBUG
    },

    {
         "mdm add $object_path",
         "mdm\n"
         "mdm add\n"
         "object path\n",
         (CLI_FUNC)cli_handleAddMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleAddMdmPathRT,
         CLI_LEVEL_DEBUG
    },

    {
         "mdm del $object_id $instance_id",
         "mdm\n"
         "mdm del\n"
         "object id\n"
         "instance id\n",
         (CLI_FUNC)cli_handleDelMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleDelMdmObjectRT,
         CLI_LEVEL_DEBUG
    },

    {
         "mdm del $object_path",
         "mdm\n"
         "mdm del\n"
         "object path\n",
         (CLI_FUNC)cli_handleDelMdmObject,
         (CLI_RUNTIME_FUNC)cli_handleDelMdmPathRT,
         CLI_LEVEL_DEBUG
    },

    {
         "dhcpc add host A.B.C.D A.B.C.D/M A.B.C.D A.B.C.D",
         "Dhcp client config\n"
         "dhcp client add\n"
         "dhcp client add host\n"
         "ip\n"
         "netmask\n"
         "gateway\n"
         "nameserver\n"
         "instance id\n",
         (CLI_FUNC)cli_handleDhcpcAddHost,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "pppoe add host A.B.C.D A.B.C.D/M A.B.C.D A.B.C.D",
         "Pppoe client config\n"
         "pppoe client add\n"
         "pppoe client add host\n"
         "ip\n"
         "netmask\n"
         "gateway\n"
         "nameserver\n"
         "instance id\n",
         (CLI_FUNC)cli_handlePppoeAddHost,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
    
    {
         "show mdm (all|config)",
         "show\n"
         "show mdm\n"
         "all\n"
         "config file\n",
         (CLI_FUNC)cli_handleShowMdm,
         (CLI_RUNTIME_FUNC)cli_handleShowMdmRT,
         CLI_LEVEL_DEBUG
    },

    {
         "show ppp info",
         "Show PPP information\n"
         "Show PPP information\n"
         "Show PPP information\n",
         (CLI_FUNC)cli_handleShowPppInfo,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set ppp wanid <0-100> username <username> password <password>",
         "Set PPP config\n"
         "Set PPP information\n"
         "Set PPPOE wanconnection ID\n"
         "Wan connection instace ID\n"
         "PPPOE username \n"
         "PPPOE username\n"
         "PPPOE password \n"
         "PPPOE password \n",
         (CLI_FUNC)cli_handleSetPppInfo,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show dhcp pool",
         "Show dhcp pool information\n"
         "Show DHCP information\n"
         "Show LAN side dhcp pool\n",
         (CLI_FUNC)cli_handleShowDhcpPool,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set dhcp pool (Computer|Camera|Stb|Phone) <address> to <address>",
         "Set dhcp pool config\n"
         "Set LAN side dhcp pool\n"
         "Set LAN side dhcp pool\n"
         "Set LAN side Computer dhcp pool\n"
         "Set LAN side Camera dhcp pool\n"
         "Set LAN side STB dhcp pool\n"
         "Set LAN side PHONE dhcp pool\n"
         "Start ip address\n"
         "to\n"
         "End ip address\n",
         (CLI_FUNC)cli_handleSetDhcpPool,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show lan ip",
         "Show Lan IP information\n"
         "Show LAN information\n"
         "Show LAN ip address\n",
         (CLI_FUNC)cli_handleShowLanIP,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set lan ipaddr <address> mask <address> dhcppool <address> to <address>",
         "Set LAN ip config\n"
         "Set LAN information\n"
         "Set LAN ip address\n"
         "LAN ip address\n"
         "LAN ip mask\n"
         "LAN ip mask\n"
         "LAN PC DHCP pool\n"
         "Start ip address\n"
         "to\n"
         "End ip address\n",
         (CLI_FUNC)cli_handleSetLanInfo,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show wlan info",
         "Show Wlan information\n"
         "Show Wireless LAN information\n"
         "Show Wireless LAN information\n",
         (CLI_FUNC)cli_handleShowWlanInfo,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set wlan ssid <name> channel <number>",
         "Set WLAN SSID and Channel\n"
         "Set WLAN information\n"
         "Set WLAN ssid information\n"
         "SSID name\n"
         "SSID channel\n"
         "SSID channel number\n",
         (CLI_FUNC)cli_handleSetWlanInfo,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show wlan encryption type",
         "Show WLAN encryption type\n"
         "Show WLAN information\n"
         "Show WLAN encryption\n"
         "Show WLAN encryption type\n",
         (CLI_FUNC)cli_handleShowWlanEncryption,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set wlan encryption open",
         "Set Wlan encryption type Open\n"
         "Set WLAN information\n"
         "Set WLAN encryption authentication method\n"
         "Set WLAN encryption authentication open\n",
         (CLI_FUNC)cli_handleSetWlanEncryptionOpen,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set wlan encryption (open|shared) keyindex <index> key <key>",
         "Set WLAN encryption and keyindex\n"
         "Set WLAN information\n"
         "Set WLAN encryption authentication method\n"
         "Set WLAN encryption authentication open\n"
         "Set WLAN encryption authentication shared\n"
         "Set WLAN Keyindex\n"
         "Index number is <1-4>\n"
         "Set WLAN Key number\n"
         "WLAN key number is 5 ASCII characters or 10 hexadecimal digits\n",
         (CLI_FUNC)cli_handleSetWlanEncryption,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set wlan encryption (WPA|11i|WPAand11i) algorithm (TKIPEncryption|AESEncryption|TKIPandAESEncryption) key <key>",
         "Set WLAN encryption mode\n"
         "Set WLAN information\n"
         "Set WLAN encryption authentication method\n"
         "Set WLAN encryption authentication WPA\n"
         "Set WLAN encryption authentication WPA2\n"
         "Set WLAN encryption authentication Mixed WPA/WPA2\n"
         "Set WLAN encryption algorithm\n"
         "TKIP\n"
         "AES\n"
         "TKIP and AES\n"
         "Set WLAN Key number\n"
         "WLAN key number\n",
         (CLI_FUNC)cli_handleSetWlanEncryption,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show wlan status",
         "Show WLAN status information\n"
         "Show WLAN information\n"
         "Show WLAN status\n",
         (CLI_FUNC)cli_handleShowWlanStatus,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set wlan status (on|off)",
         "Set WLAN status\n"
         "Set WLAN information\n"
         "Set WLAN status\n"
         "Enable\n"
         "Disable\n",
         (CLI_FUNC)cli_handleSetWlanStatus,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show wlan 802.11 mode",
         "Show WLAN 802.11 mode\n"
         "Show WLAN information\n"
         "Show WLAN 802.11 mode\n"
         "Show WLAN 802.11 mode\n",
         (CLI_FUNC)cli_handleShowWlanMode,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set wlan 802.11 mode (b|bg|bgn)",
         "Set WLAN 802.11 mode\n"
         "Set WLAN information\n"
         "Set WLAN 802.11\n"
         "Set WLAN 802.11 mode\n"
         "Support b mode\n"
         "Support b and g mode\n"
         "Support b g and n mode\n",
         (CLI_FUNC)cli_handleSetWlanMode,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show tr069 state",
         "Show tr069 state information\n"
         "Show TR069 information\n"
         "Show TR069 state configuration\n",
         (CLI_FUNC)cli_handleShowTr069State,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show urlfilter config",
         "Show URLfilter\n"
         "Show URLfilter information\n",
         (CLI_FUNC)cli_handleShowUrlfilterConfig,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "set tr069 CPE Inform (Status|Interval) $param_value",
         "Set tr069 CPE Information\n"
         "Set Tr069 information\n"
         "Set CPE command group\n"
         "Set TR069 CPE inform\n"
         "Set status mode\n"
         "Set interval time\n",
         (CLI_FUNC)cli_handleSetTr069State,
         (CLI_RUNTIME_FUNC)cli_handleSetTr069StateRT,
         CLI_LEVEL_DEBUG
    },

    {
         "set tr069 ACS (URL|Username|Password) $param_value",
         "Set tr069 ACS\n"
         "Set Tr069 information\n"
         "Set ACS command group\n"
         "Set TR069 ACS URL\n"
         "Set TR069 ACS username\n"
         "Set TR069 ACS password\n",
         (CLI_FUNC)cli_handleSetTr069State,
         (CLI_RUNTIME_FUNC)cli_handleSetTr069StateRT,
         CLI_LEVEL_DEBUG
    },

    {
         "set tr069 Connection (Request_Authentication|Request_User|Request_Password|Request_Path|Request_Port) $param_value",
         "Set tr069 Connetion\n"
         "Set Tr069 information\n"
         "Set Connection comand group\n"
         "Set TR069 ACS connection request authentication\n"
         "Set TR069 ACS connection request user\n"
         "Set TR069 ACS connection request password\n"
         "Set TR069 ACS connection request path\n"
         "Set TR069 ACS connection request port\n",
         (CLI_FUNC)cli_handleSetTr069State,
         (CLI_RUNTIME_FUNC)cli_handleSetTr069StateRT,
         CLI_LEVEL_DEBUG
    },

    {
         "show tr69c log",
         "Show\n"
         "Show TR69c\n"
         "Show TR69c log\n",
         (CLI_FUNC)cli_handleShowTr69Soap,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
    
    {
         "tr69c log (enable|disable)",
         "TR69c\n"
         "TR69c log\n"
         "enable\n"
         "disable\n",
         (CLI_FUNC)cli_handleEnableTr69cLog,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
    
    {
         "tr69c log clear",
         "TR69c\n"
         "TR69c log\n"
         "TR69c log clear\n",
         (CLI_FUNC)cli_handleClearTr69cLog,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG

    },
      
    {  
         "tr69c addObj $object_path",
         "TR69c\n"
         "TR69c addObj\n"
         "object path\n",
         (CLI_FUNC)cli_handleDoAddObject,
         (CLI_RUNTIME_FUNC)cli_handleAddMdmPathRT,
         CLI_LEVEL_DEBUG
    },

    {  
         "tr69c delObj $object_path",
         "TR69c\n"
         "TR69c delObj\n"
         "object path\n",
         (CLI_FUNC)cli_handleDoDelObject,
         (CLI_RUNTIME_FUNC)cli_handleDelMdmPathRT,
         CLI_LEVEL_DEBUG
    },

    {
        "tr69c getParamval $object_path $param_name",
        "TR69c\n"
        "TR69c getParamval\n"
        "object path\n",
        (CLI_FUNC)cli_handleDoGetValue,
        (CLI_RUNTIME_FUNC)cli_handleSetMdmPathRT,
        CLI_LEVEL_DEBUG
    },
    
    {
         "tr69c setParamval $object_path $param_name $param_value",
         "TR69c\n"
         "TR69c setParamval\n"
         "object path\n"
         "param name\n"
         "param value\n",
         (CLI_FUNC)cli_handleDoSetValue,
         (CLI_RUNTIME_FUNC)cli_handleSetMdmPathRT,
         CLI_LEVEL_DEBUG
    },
    
    {
         "tr69c reboot",
         "TR69c\n"
         "TR69c reboot\n" ,       
         (CLI_FUNC)cli_handleDoReboot,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
    
    {
         "tr69c factoryreset",
         "TR69c\n"
         "TR69c factoryreset\n" ,       
         (CLI_FUNC)cli_handleDoFactoryReset,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
    {
        "tr69c getParamNames $object_path",
        "TR69c\n"
        "TR69c getParamNames\n"
        "object path\n",
        (CLI_FUNC)cli_handleDoGetName,
        (CLI_RUNTIME_FUNC)cli_handleShowMdmPathRT,
        CLI_LEVEL_DEBUG
    }, 

    {
        "tr69c getParamattr $object_path $param_name",
        "TR69c\n"
        "TR69c getParamattr\n"
        "object path\n"
        "param_name\n",
        (CLI_FUNC)cli_handleDoGetAttributes,
        (CLI_RUNTIME_FUNC)cli_handleSetMdmPathRT,
        CLI_LEVEL_DEBUG
    },
    
    {
        "tr69c setParamattr $object_path $param_name (0|1|2)",
        "TR69c\n"
        "TR69c setParamattr\n"
        "object path\n"
        "param_name\n",
        (CLI_FUNC)cli_handleDoSetAttributes,
        (CLI_RUNTIME_FUNC)cli_handleSetMdmPathRT,
        CLI_LEVEL_DEBUG
    },

    {
        "show (version|nat)",
        "Show system information\n"
        "Show Version information\n"
        "Show system NAT session information\n"
        "version\n",
        (CLI_FUNC)cli_handleShowVersion,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show mcpd (config|objtree|src_filter|grp_filter)",
        "Show\n"
        "Show mcpd information\n",
        (CLI_FUNC)cli_handleShowMcpdRun,
        NULL,
        CLI_LEVEL_DEBUG
    },
    
#ifdef SUPPORT_DEBUG_TOOLS
    {
        "show memory (cmc|mcpd|mmed|tr69c)",
        "Show\n",
        (CLI_FUNC)cli_handleShowSharedMem,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show mcpd memory allType",
        "Show\n"
        "Show mcpd\n"
        "Show mcpd allType",
        (CLI_FUNC)cli_handleShowMcpdMem,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show mcpd memory typeId <0-20>",
        "Show\n"
        "Show mcpd\n"
        "Show mcpd typeId",
        (CLI_FUNC)cli_handleShowMcpdTypeMem,
        NULL,
        CLI_LEVEL_DEBUG
    },
#endif

#ifdef VOS_MEM_LEAK_TRACING
        {
            "show (mcpd|mmed|cmc) trace (all|50|clones)",
            "Show\n"
            "Show mcpd\n"
            "Show mcpd trace\n",
            (CLI_FUNC)cli_handleShowTracing,
            NULL,
            CLI_LEVEL_DEBUG
        },
#endif

    {
         "show mdm tree $object_id",
         "Show\n",
         (CLI_FUNC)cli_handleShowMdmTree,
         (CLI_RUNTIME_FUNC)cli_handleShowMdmTreeRT,
         CLI_LEVEL_DEBUG
    },

#ifndef DESKTOP_LINUX
    {
         "show cmc obj trace",
         "Show\n",
         (CLI_FUNC)cli_handleShowObjCmcObjTrace,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
#endif
    {
         "watchdog (on|off)",
         "watchdog\n"
         "on\n"
         "off",
         (CLI_FUNC)cli_handleSetWatchdog,
         NULL,
         CLI_LEVEL_DEBUG
    },

    /* Support mcast command START */
    {
        "igmp version (v1|v2|v3)",
        "igmp\n"
        "igmp version\n",
        (CLI_FUNC)cli_handleIgmpVer,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "igmp fast-leave (on|off)",
        "igmp\n"
        "igmp fast-leave\n",
        (CLI_FUNC)cli_handleFastLeave,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "pon (on|off)",
        "pon\n"
        "on\n"
        "off",
        (CLI_FUNC)cli_handlePonSwitch,
        NULL,
        CLI_LEVEL_DEBUG
    },
        
    /* Support mcast command END */
    {
        "init <Y/N>", 
        "init \n",
        (CLI_FUNC)cli_handleRemoteResetDefault,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "load pack by <ftp/tftp> svr ip <x.x.x.x> remotefile <xxx.w>", 
        "load \n"
        "load pack \n"
        "load pack by ftp \n"
        "load pack by ftp svr \n"
        "load pack by ftp svr ip \n",
        (CLI_FUNC)cli_handleDownloadImage, 
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
         "netSniffer show (macList|pcInfo|uploadInfo|httpInfo|tcpInfo|dnsInfo|threadInfo|classfyRules)",
         "net_sniffer command\n"
         "show app info\n"
         "options\n",
         (CLI_FUNC)cli_handleNetSniffer,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },   
    #endif
};
#if 0
static CLI_CMD_T sg_gponCmdTable[] = 
{
    {
         "gponshell",
         "Enter gpon shell\n",
         (CLI_FUNC)cli_handleGponShell,
         (CLI_RUNTIME_FUNC)NULL,
         CLI_LEVEL_DEBUG
    },
};

static CLI_CMD_T sg_eocCmdTable[] = 
{
    {
        "mme receive packets (enable|disable)",
        "mme receive\n"
        "mme receive packets\n"
        "enable\n"
        "disable\n",
        (CLI_FUNC)cli_handleMmePrintRxDebug,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "mme transmit packets (enable|disable)",
        "mme transmit\n"
        "mme transmit packets\n"
        "enable\n"
        "disable\n",
        (CLI_FUNC)cli_handleMmePrintTxDebug,
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "mme test link (up|down)",
        "mme test\n"
        "mme test link\n"
        "up\n"
        "down\n",
        (CLI_FUNC)cli_handleMmeTestLinkStatus,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "mme send packets <contents>",
        "mme send\n"
        "mme send packets\n",
        (CLI_FUNC)cli_handleMmeSendPack,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "mme receive packets without blank (enable|disable)",
        "mme receive\n"
        "mme receive packets\n"
        "mme receive packets without blank\n"
        "enable\n"
        "disable\n",
        (CLI_FUNC)cli_handleMmePrintRxNoSpaceDebug,
        NULL,
        CLI_LEVEL_DEBUG
    },    
};

static CLI_CMD_T sg_eocAteTable[] = 
{
    {
        "show customername",
        "show customername\n",
        (CLI_FUNC)cli_handleGetCustomerName,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "show customerarea ",
        "show customerarea\n",
        (CLI_FUNC)cli_handleGetCustomerArea,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "show firmware",
        "show firmware\n",
        (CLI_FUNC)cli_handleGetEocFirmwareVersion,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "show hfid ",
        "show hfid\n",
        (CLI_FUNC)cli_handleGetHfid,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "show fastOnLine",
        "show fastOnLine\n",
        (CLI_FUNC)cli_handleGetFastOnLine,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "show isolate ",
        "show isolate\n",
        (CLI_FUNC)cli_handleGetIsolate,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write mac  <xx:xx:xx:xx:xx:xx>",
        "write hostmac  xx:xx:xx:xx:xx:xx\n",
        (CLI_FUNC)cli_handleSetMacAddr,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write 9331mac  <xx:xx:xx:xx:xx:xx>",
        "write 9331mac  xx:xx:xx:xx:xx:xx\n",
        (CLI_FUNC)cli_handleSet9331MacAddr,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write macisolate  <xx:xx:xx:xx:xx:xx> (0|1)",
        "write macisolate  xx:xx:xx:xx:xx:xx 0|1\n",
        (CLI_FUNC)cli_handleSetEocChipMacAndIsolate,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "show 9331mac",
        "show 9331mac\n",
        (CLI_FUNC)cli_handleGet9331MacAddr,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "show loginname",
        "show loginname\n",
        (CLI_FUNC)cli_handleGetUserName,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write customername  <customerName>",
        "write customername\n",
        (CLI_FUNC)cli_handleSetCustomerName,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write customerarea <area>",
        "write customerarea\n",
        (CLI_FUNC)cli_handleSetCustomerArea,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write loginname <name>",
        "write loginname\n",
        (CLI_FUNC)cli_handleSetUserName,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write isolate <0-1>",
        "write isolate\n",
        (CLI_FUNC)cli_handleSetIsolate,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write hfid <hfid>",
        "write hfid\n",
        (CLI_FUNC)cli_handleSetHfid,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "port forward enable <portNum>",
        "port forward enable\n",
        (CLI_FUNC)cli_handlePortRxEnable,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "port forward diable <portNum>",
        "port forward diable\n",
        (CLI_FUNC)cli_handlePortRxDisable,
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "write macisolateFastOnLine  <xx:xx:xx:xx:xx:xx> (0|1) (0|1)",
        "write macisolateFastOnLine  xx:xx:xx:xx:xx:xx 0|1  0|1\n",
        (CLI_FUNC)cli_handleSetEocChipMacAndIsolateFastOnLine,
        NULL,
        CLI_LEVEL_DEBUG
    },
};


static CLI_CMD_T sg_shareCmdTable[] = 
{
    {
        "led (on|off)",
        "led \n"
        "on\n"
        "off\n",
        (CLI_FUNC)cli_handleSetLedState,
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "write mac <1-3> <A.B.C.D.E.F>",
        "write ont parameters\n"
        "write ont mac address\n"
        "1:CPU mac 2:VOIP mac 3:EOC mac\n"
        "Formate:aa:bb:cc:dd:ee:ff",
        (CLI_FUNC)cli_handleWriteMac, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show mac",
        "show mac parameters\n"
        "show mac address\n",
        (CLI_FUNC)cli_handleGetMacAddr,
        NULL,
        CLI_LEVEL_DEBUG
    },
            
    {
         "write oui <string>",
         "write manufacturer oui\n",
         (CLI_FUNC)cli_handleSetOui,
         NULL,
         CLI_LEVEL_DEBUG
    },

    {
         "show oui",
         "show manufacturer oui\n",
         (CLI_FUNC)cli_handleGetOui,
         NULL,
         CLI_LEVEL_DEBUG
    },

    {
        "restore",
        "restore\n"
        "reset default configuration\n",
        (CLI_FUNC)cli_handleRestore, 
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "tofactory",
        "tofactory\n"
        "reset manufactory configuration\n",
        (CLI_FUNC)cli_handleToFactory, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show isfactorymode",
        "show isfactorymode\n"
        "in factory mode or user mode\n",
        (CLI_FUNC)cli_handleCheckReset,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "button test (on|off)",
        "button switch\n"
        "button test"
        "on:in factory mode\n"
        "off:out factory mode\n",
        (CLI_FUNC)cli_handleSetButtonTestMode,
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "usbtest",
        "test usb interface\n"
        "get message if there are some usb devices\n",
        (CLI_FUNC)cli_handleTestUsb,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write ssid  <ssid>",
        "write wireless ssid name\n",
        (CLI_FUNC)cli_handleSetSsid,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show ssid",
        "show ssid\n",
        (CLI_FUNC)cli_handleGetSsid,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write wlankey  <ssid>",
        "write wlankey\n",
        (CLI_FUNC)cli_handleSetWlanKey,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show wlankey ",
        "show wlankey\n",
        (CLI_FUNC)cli_handleGetWlanKey,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write loginpasswd <passwd>",
        "write loginpasswd\n"
        "Set user loginpassword\n",
        (CLI_FUNC)cli_handleSetUserPassWd,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show loginpasswd",
        "show loginpasswd\n",
        (CLI_FUNC)cli_handleGetUserPassWd,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show softversions",
        "show softversions\n",
        (CLI_FUNC)cli_handleShowtSoftVersion,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write hwversion <version>",
        "write hwversion\n",
        (CLI_FUNC)cli_handleSetHwVersion,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show hwversion ",
        "show hwversion\n",
        (CLI_FUNC)cli_handleGetHwVersion,
        NULL,
        CLI_LEVEL_DEBUG
    },
    
};


static CLI_CMD_T sg_ponCmdTable[] = 
{
    {
         "show egis",
         "show ont parameters\n"
         "show egis\n",
         (CLI_FUNC)cli_handleEgisShow,
         NULL,
         CLI_LEVEL_DEBUG
    },
    
    {
         "write egis (sn|ethaddr0|ethaddr1|BOOTTYPE|CODECLASS|CODEHWINFO|CODEPORTMAP|CODEEXTEND|IPADDR|NETMASK|CTCLOID|CTCPASSWORD|GATEWAY|PASSWD) <value>",
         "write\n"
         "write egis\n"
         "value\n",
         (CLI_FUNC)cli_handleEgisSet,
         NULL,
         CLI_LEVEL_DEBUG
    },
        
    {
        "write sn <string>",
        "write ont parameters\n"
        "write ont serial number\n"
        "format:CCCCXXXXXXXX",
        (CLI_FUNC)cli_handleWriteSn, 
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "show sn",
        "show ont parameters\n"
        "show ont serial number\n",
        (CLI_FUNC)cli_handleReadSn, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write product <string> <string> <string> <string>",
        "write product code\n"
        "product\n"
        "product <Class-Word> <Hwinfo-Word> <PortMappong-Word> <Extend-Word>\n",
        (CLI_FUNC)cli_handleWriteProduct,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write oltpassword <string>",
        "Write olt register's parameters\n"
        "Write olt register's password\n"
        "Format:XXXXXXXXXXXXXXXXXXXX\n",
        (CLI_FUNC)cli_handleWritePassword, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show oltpassword",
        "show olt register's parameters\n"
        "show olt register's password\n",
        (CLI_FUNC)cli_handleReadPassword,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write oltpassword hex <string>",
        "Write olt register's parameters\n"
        "Write olt register's password\n"
        "Hex mode\n"
        "Format:XXXXXXXXXXXXXXXXXXXX\n",
        (CLI_FUNC)cli_handleWritePasswordHex,
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "show oltpassword hex",
        "show olt register's parameters\n"
        "show olt register's password\n"
        "Hex mode\n",
        (CLI_FUNC)cli_handleReadPasswordHex,
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write ctc (loid|password) <string>",
        "write\n"
        "ctc\n"
        "loid\n"
        "password\n"
        "string\n",
        (CLI_FUNC)cli_handleSetCtcInfoByStr, 
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "write ctc (loid|password) hex <value>",
        "write\n"
        "ctc\n"
        "loid\n"
        "password\n"
        "hex mode\n",
        (CLI_FUNC)cli_handleSetCtcInfoByHex, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show ctc info",
        "show\n"
        "ctc info\n"
        "ctc loid & password info\n",
        (CLI_FUNC)cli_handleShowCtcInfoStr, 
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "show ctc info hex",
        "show\n"
        "ctc info\n"
        "ctc loid & password info\n"
        "hex mode\n",
        (CLI_FUNC)cli_handleShowCtcInfoHex, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write uni mac <string>",
        "write ont parameters\n"
        "write uni parameters\n"
        "write uni mac address\n"
        "format:aa:bb:cc:dd:ee:ff\n",
        (CLI_FUNC)cli_handleWriteCpuMac,
        NULL,
        CLI_LEVEL_DEBUG
    },
    
    {
        "show uni mac",
        "show ont parameters\n"
        "show uni parameters\n"
        "show uni mac address\n",
        (CLI_FUNC)cli_handleReadCpuMac,
        NULL,
        CLI_LEVEL_DEBUG
    },

#ifdef PFM_HGU
    {
        "laser (on|off)",
        "laser switch\n"
        "laser force on or off\n",
        (CLI_FUNC)cli_handleLaserPrbsCtrl, 
        NULL,
        CLI_LEVEL_DEBUG
    },
    {
        "laser burst (on|off)",
        "laser \n"
        "laser burst switch"
        "laser force on or off\n",
        (CLI_FUNC)cli_handleLaserCtrl, 
        NULL,
        CLI_LEVEL_DEBUG    
    },
    {
        "laser default (on|off)",
        "laser \n"
        "laser default switch"
        "laser default on or off\n",
        (CLI_FUNC)cli_handleLaserOndefaultMode, 
        NULL,
        CLI_LEVEL_DEBUG    
    },
    {
        "perf test (on|off)",
        "perf \n"
        "perf test switch"
        "perf test on or off\n",
        (CLI_FUNC)cli_handlePerfTestMode, 
        NULL,
        CLI_LEVEL_DEBUG        
    },    
    {
         "bobtest read <string> <string>",
         "bobtest read_reg \n"
         "bobtest read_reg slave_addr reg_addr\n"
         "format:addr offset",
         (CLI_FUNC)cli_BobReadreg,
         NULL,
         CLI_LEVEL_DEBUG
    },
    {
         "bobtest reads <string> <string> <string>",
         "bobtest read_regs \n"
         "bobtest read_regs slave_addr begin_addr count\n"
         "format:addr offset len",
         (CLI_FUNC)cli_BobReadregs,
         NULL,
         CLI_LEVEL_DEBUG
    },
    {
         "bobtest write <string> <string> <string>",
         "bobtest write_reg \n"
         "bobtest write_reg slave_addr reg_addr value\n"
         "format:addr offset 0xaa",
         (CLI_FUNC)cli_BobWritereg,
         NULL,
         CLI_LEVEL_DEBUG
    },
    {
         "bobtest writes <string> <string> <string> <string>",
         "bobtest write_regs \n"
         "bobtest write_regs slave_addr reg_addr value\n"
         "format: addr offset len 0xaa0xbb0xcc....",
         (CLI_FUNC)cli_BobWriteregs,
         NULL,
         CLI_LEVEL_DEBUG
    },  
    {
         "bobtest show dataparam",
         "bobtest show \n"
         "bobtest show dataparam",
         (CLI_FUNC)cli_BobShowDataParam,
         NULL,
         CLI_LEVEL_DEBUG
    }, 
    {
         "transceiver state",
         "transceiver\n"
         "transceiver state",
         (CLI_FUNC)cli_bobTransceiverState,
         NULL,
         CLI_LEVEL_DEBUG
    },    
#endif
    
    {
        "write ProductClass <string>",
        "write factory mode parameter\n"
        "write the Product Class\n"
        "format xxxxxxxx\n",
        (CLI_FUNC)cli_handleSetProductClass, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write DeviceId <string>",
        "Set factory mode parameter\n"
        "write Device Identifier\n"
        "format xxxxxxxx\n",
        (CLI_FUNC)cli_handleSetDeviceId, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "write equipment id <string>", 
        "write\n"
        "write equipment\n"
        "write equipment id\n"
        "equipment id\n",
        (CLI_FUNC)cli_handleWriteEquipmentId, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show equipment id", 
        "show\n"
        "show equipment\n"
        "show equipment id\n",
        (CLI_FUNC)cli_handleReadEquipmentId, 
        NULL,
        CLI_LEVEL_DEBUG
    },

    {
        "show ups state", 
        "show\n"
        "show ups\n"
        "show ups state\n",
        (CLI_FUNC)cli_handleShowUpsState, 
        NULL,
        CLI_LEVEL_DEBUG
    },
};
#endif


VOS_RET_E CLI_init(void)
{
    VOS_RET_E ret;
    UINT32 i;

    CLI_addNode(ROOT_NODE, ROOT_NODE, "%s", NULL);
    CLI_addNode(ROOT_NODE, MANUFACTORY_NODE, "MANUFACTORY", NULL);

    for (i = 0; i < sizeof(sg_cmdTable) / sizeof(sg_cmdTable[0]); i++)
    {
        ret = CLI_addCommand(ROOT_NODE,
                             sg_cmdTable[i].cmd,
                             sg_cmdTable[i].help,
                             NULL,
                             sg_cmdTable[i].func,
                             sg_cmdTable[i].runtime_func,
                             sg_cmdTable[i].level,
                             FALSE);

        if (VOS_RET_SUCCESS != ret)
        {
            printf("create cmd[%s] failed\r\n", sg_cmdTable[i].cmd);
        }
    }
#if 0
    for (i = 0; i < sizeof(sg_shareCmdTable) / sizeof(sg_shareCmdTable[0]); i++)
    {
       ret = CLI_addCommand(MANUFACTORY_NODE,
                             sg_shareCmdTable[i].cmd,
                             sg_shareCmdTable[i].help,
                             NULL,
                             sg_shareCmdTable[i].func,
                             sg_shareCmdTable[i].runtime_func,
                             sg_shareCmdTable[i].level,
                             FALSE);

        if (VOS_RET_SUCCESS != ret)
        {
            printf("create cmd[%s] failed\r\n", sg_shareCmdTable[i].cmd);
        }
    }

    if (SF_FEATURE_UPLINK_TYPE_GPON || SF_FEATURE_UPLINK_TYPE_EPON)
    {
        for (i = 0; i < sizeof(sg_gponCmdTable) / sizeof(sg_gponCmdTable[0]); i++)
        {
            ret = CLI_addCommand(ROOT_NODE,
                                 sg_gponCmdTable[i].cmd,
                                 sg_gponCmdTable[i].help,
                                 NULL,
                                 sg_gponCmdTable[i].func,
                                 sg_gponCmdTable[i].runtime_func,
                                 sg_gponCmdTable[i].level,
                                 FALSE);

            if (VOS_RET_SUCCESS != ret)
            {
                printf("create cmd[%s] failed\r\n", sg_gponCmdTable[i].cmd);
            }
        }


        for (i = 0; i < sizeof(sg_ponCmdTable) / sizeof(sg_ponCmdTable[0]); i++)
        {
            ret = CLI_addCommand(MANUFACTORY_NODE,
                                 sg_ponCmdTable[i].cmd,
                                 sg_ponCmdTable[i].help,
                                 NULL,
                                 sg_ponCmdTable[i].func,
                                 sg_ponCmdTable[i].runtime_func,
                                 sg_ponCmdTable[i].level,
                                 FALSE);

            if (VOS_RET_SUCCESS != ret)
            {
                printf("create cmd[%s] failed\r\n", sg_ponCmdTable[i].cmd);
            }
        }
    }

    if (SF_FEATURE_UPLINK_TYPE_EOC)
    {
        for (i = 0; i < sizeof(sg_eocCmdTable) / sizeof(sg_eocCmdTable[0]); i++)
        {
            ret = CLI_addCommand(ROOT_NODE,
                                 sg_eocCmdTable[i].cmd,
                                 sg_eocCmdTable[i].help,
                                 NULL,
                                 sg_eocCmdTable[i].func,
                                 sg_eocCmdTable[i].runtime_func,
                                 sg_eocCmdTable[i].level,
                                 FALSE);

            if (VOS_RET_SUCCESS != ret)
            {
                printf("create cmd[%s] failed\r\n", sg_eocCmdTable[i].cmd);
            }
        }

        for (i = 0; i < sizeof(sg_eocAteTable) / sizeof(sg_eocAteTable[0]); i++)
        {
           ret = CLI_addCommand(MANUFACTORY_NODE,
                                 sg_eocAteTable[i].cmd,
                                 sg_eocAteTable[i].help,
                                 NULL,
                                 sg_eocAteTable[i].func,
                                 sg_eocAteTable[i].runtime_func,
                                 sg_eocAteTable[i].level,
                                 FALSE);

            if (VOS_RET_SUCCESS != ret)
            {
                printf("create cmd[%s] failed\r\n", sg_eocAteTable[i].cmd);
            }
        }
    }

    ret = CLI_initProduct();
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("Init product cli blsend error!\n");
    }

    VOS_RET_E CLI_initSrpcCMC(CLI_NODE_ID nodeId);
    //CLI_initSrpcCMC(ROOT_NODE);
#endif

    return VOS_RET_SUCCESS;
}


