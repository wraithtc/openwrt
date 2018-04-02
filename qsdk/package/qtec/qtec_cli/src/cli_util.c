#include "cli_api.h"
#include "cli_vty.h"
#include "cli_util.h"
#include "cli.h"


VOS_RET_E cli_handleSelf(CMD_ELEMENT_T *element, VTY_T *vty, int argc, char **argv)
{
    (void)argc;
    (void)argv;

    vty->node = element->node;
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleCd(CMD_ELEMENT_T *element, VTY_T *vty, int argc, char **argv)
{
    CLI_NODE_ID nodeId = INVALID_NODE;

    (void)element;

    if (argc != 1)
    {
        return VOS_RET_SUCCESS;
    }

    nodeId = cmd_find_node(ROOT_NODE, argv[0]);
    if (INVALID_NODE != nodeId)
    {
        vty->node = nodeId;
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleBack(CMD_ELEMENT_T *element, VTY_T *vty, int argc, char **argv)
{
    (void)argc;
    (void)argv;

    vty->node = element->node;
    return VOS_RET_SUCCESS;
}


VOS_RET_E cli_handleQuit(CMD_ELEMENT_T *element, VTY_T *vty, int argc, char **argv)
{
    (void)element;
    (void)vty;
    (void)argc;
    (void)argv;

    cli_keepLooping = FALSE;
    return VOS_RET_SUCCESS;
}


VOS_RET_E CLI_addCommand(CLI_NODE_ID nodeId,
                         const char *cmd,
                         const char *help,
                         CLI_POWER_FUNC powerFunc,
                         CLI_FUNC func,
                         CLI_RUNTIME_FUNC runtimeFunc,
                         CLI_LEVEL level,
                         UBOOL8 hide)
{
    CMD_ELEMENT_T *element = NULL;

    element = VOS_MALLOC(sizeof(CMD_ELEMENT_T));
    if (NULL == element)
    {
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    memset(element, 0, sizeof(CMD_ELEMENT_T));
    element->node = nodeId;
    element->string = VOS_STRDUP(cmd);
    element->doc = VOS_STRDUP(help);
    element->power_func = powerFunc;
    element->func = func;
    element->runtime_func = runtimeFunc;
    element->level = level;
    element->hide = hide;
    cmd_install_element(nodeId, element);

    return VOS_RET_SUCCESS;
}


VOS_RET_E CLI_addNode(CLI_NODE_ID parentNodeId, CLI_NODE_ID nodeId, const char *prompt, CLI_FUNC func)
{
    CMD_NODE_T *node = NULL;
    CMD_ELEMENT_T *element = NULL;

    node = VOS_MALLOC(sizeof(CMD_NODE_T));
    if (NULL == node)
    {
        vosLog_error("VOS_MALLOC failed");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    memset(node, 0, sizeof(CMD_NODE_T));
    node->node = nodeId;
    node->prompt = VOS_STRDUP(prompt);
    cmd_install_node(node, func);

    if (nodeId != ROOT_NODE)
    {
        element = VOS_MALLOC(sizeof(CMD_ELEMENT_T));
        if (NULL == element)
        {
            vosLog_error("VOS_MALLOC failed");
            return VOS_RET_RESOURCE_EXCEEDED;
        }

        memset(element, 0, sizeof(CMD_ELEMENT_T));
        element->node = nodeId;
        element->string = VOS_STRDUP(prompt);
        element->doc = NULL;
        element->power_func = cli_handleSelf;
        element->func = NULL;
        element->runtime_func = NULL;
        element->level = CLI_LEVEL_PUBLIC;
        element->hide = FALSE;
        cmd_install_element(parentNodeId, element);
    }


    element = VOS_MALLOC(sizeof(CMD_ELEMENT_T));
    if (NULL == element)
    {
        vosLog_error("VOS_MALLOC failed");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    memset(element, 0, sizeof(CMD_ELEMENT_T));
    element->node = nodeId;
    element->string = VOS_STRDUP("cd <string>");
    element->doc = VOS_STRDUP("Go to certain Node\n Node name");
    element->power_func = cli_handleCd;
    element->func = NULL;
    element->runtime_func = NULL;
    element->level = CLI_LEVEL_PUBLIC;
    element->hide = TRUE;
    cmd_install_element(nodeId, element);


    element = VOS_MALLOC(sizeof(CMD_ELEMENT_T));
    if (NULL == element)
    {
        vosLog_error("VOS_MALLOC failed");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    memset(element, 0, sizeof(CMD_ELEMENT_T));
    element->node = parentNodeId;
    element->string = VOS_STRDUP("exit");
    element->doc = VOS_STRDUP("Go to up level Node");
    element->power_func = cli_handleBack;
    element->func = NULL;
    element->runtime_func = NULL;
    element->level = CLI_LEVEL_PUBLIC;
    element->hide = TRUE;
    cmd_install_element(nodeId, element);


    element = VOS_MALLOC(sizeof(CMD_ELEMENT_T));
    if (NULL == element)
    {
        vosLog_error("VOS_MALLOC failed");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    memset(element, 0, sizeof(CMD_ELEMENT_T));
    element->node = nodeId;
    element->string = VOS_STRDUP("quit");
    element->doc = VOS_STRDUP("quit CLI");
    element->power_func = cli_handleQuit;
    element->func = NULL;
    element->runtime_func = NULL;
    element->level = CLI_LEVEL_PUBLIC;
    element->hide = TRUE;
    cmd_install_element(nodeId, element);

    return VOS_RET_SUCCESS;
}


static VOS_RET_E cli_handleRemote(VosEntityId eid,
                              VosMsgType msgType,
                              int argc,
                              char **argv)
{
    int i, j;
    UINT32 msgDataLen;
    VosMsgHeader *msg;
    void *msgBuf;
    char *data;
    VOS_RET_E ret;

    msgDataLen = 0;
    if (argv)
    {
        for (i = 0; i < argc; i++)
        {
            if (argv[i])
            {
                msgDataLen += strlen(argv[i]);
            }
            msgDataLen++;
        }
    }

    if (msgDataLen > 0)
    {
        msgBuf = VOS_MALLOC_FLAGS(sizeof(VosMsgHeader) + msgDataLen, ALLOC_ZEROIZE);
    } 
    else
    {
        msgBuf = VOS_MALLOC_FLAGS(sizeof(VosMsgHeader), ALLOC_ZEROIZE);
    }

    msg = (VosMsgHeader *)msgBuf;
    if (NULL == msg)
    {
        vosLog_error("VOS_MALLOC_FLAGS fail");
        return VOS_RET_RESOURCE_EXCEEDED;
    }

    msg->type = msgType;
    msg->src = vosMsg_getHandleEid(msgHandle);
    msg->dst = eid;
    msg->wordData = argc;

    if (msgDataLen > 0)
    {
        msg->dataLength = msgDataLen;

        data = (char *) (msg + 1); 
        for (i = 0; i < argc; i++)
        {
            if (argv[i])
            {
                for (j = 0; (*data++ = argv[i][j]); j++) ;
            }
            else
            {
                *data++ = '\0';
            }
        }
    }      

    ret = vosMsg_sendAndGetReply(msgHandle, msg);
    if (ret != VOS_RET_SUCCESS)
    {
        vosLog_error("Send msg 0x%x to 0x%x fail, ret=%d", msgType, eid, ret);
    }
    else
    {
        vosLog_debug("Send msg 0x%x to 0x%x succeeded", msgType, eid);
    }

    VOS_FREE(msgBuf);

    return ret;
}


VOS_RET_E CLI_handleRemoteExecute(VosEntityId eid,
                                  VosMsgType msgType,
                                  int argc,
                                  char **argv)
{
    VOS_RET_E ret;
    char filename[64];
    char line[256];
    FILE *file;

    ret = cli_handleRemote(eid, msgType, argc, argv);
    if (ret != VOS_RET_SUCCESS)
    {
        return ret;
    }

    snprintf(filename, sizeof(filename), UTIL_CLI_REMOTE_RESULT_FILE);
    
    file = fopen(filename, "r");
    if (NULL == file)
    {
        vosLog_error("open file %s fail", filename);
        return VOS_RET_OPEN_FILE_ERROR;
    }

    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }

    fclose(file);
    remove(filename);

    return VOS_RET_SUCCESS;
}


VOS_RET_E CLI_handleRemoteRuntime(VosEntityId eid,
                                  VosMsgType msgType,
                                  int argc,
                                  char **argv,
                                  UTIL_VECTOR vecCmdDesc)
{
    VOS_RET_E ret;
    char filename[64];
    char line[256];
    FILE *file;
    char *desc;

    ret = cli_handleRemote(eid, msgType, argc, argv);
    if (ret != VOS_RET_SUCCESS)
    {
        return ret;
    }

    snprintf(filename, sizeof(filename), UTIL_CLI_REMOTE_RESULT_FILE);
    file = fopen(filename, "r");
    if (NULL == file)
    {
        vosLog_error("open file %s fail", filename);
        return VOS_RET_OPEN_FILE_ERROR;
    }

    while (fgets(line, sizeof(line), file))
    {
        util_strTrim(line);
        desc = line;

        if ('\"' == *desc)
        {
            desc++;
            while (*desc && '\"' != *desc)
            {
                desc++;
            }

            if ('\"' == *desc)
            {
                desc++;
            }
        }
        else
        {
            while (*desc && ! isblank(*desc))
            {
                desc++;
            }
        }

        if (*desc)
        {
            *desc++ = '\0';
        }

        if (strlen(line) > 0)
        {
            cmd_install_cmddesc(line, desc, vecCmdDesc);
        }
    }

    fclose(file);
    remove(filename);

    return VOS_RET_SUCCESS;
}
