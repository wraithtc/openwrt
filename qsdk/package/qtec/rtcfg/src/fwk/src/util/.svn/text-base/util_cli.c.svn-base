#include "fwk.h"


VOS_RET_E UTIL_processRemoteCli(void *msgHandle,
                               const VosMsgHeader *msg,
                               UTIL_CLI_REMOTE_FUNC func)
{
    VOS_RET_E ret;
    int i;
    int argc;
    const char *argv[32];
    const char *data;
    char filename[64];
    FILE *file;

    argc = 0;
    memset(argv, 0, sizeof(argv));

    if (msg->dataLength)
    {
        argc = msg->wordData;
        data = (const char *) (msg + 1); 

        for (i = 0; i < argc; i++)
        {
            argv[i] = data;
            data += strlen(data) + 1;
        }
    }
     
    UTIL_SNPRINTF(filename, sizeof(filename), UTIL_CLI_REMOTE_RESULT_FILE);
    remove(filename);

    file = fopen(filename, "w+");
    if (file)
    {
        ret = (*func)(argc, argv, file);
        fclose(file);

        ret = vosMsg_sendReply(msgHandle, msg, ret);
    }
    else
    {
        vosLog_error("open file %s fail", filename);
        ret = VOS_RET_OPEN_FILE_ERROR;
    }

    return ret;
}

