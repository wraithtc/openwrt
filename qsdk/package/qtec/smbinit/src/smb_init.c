/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdio.h>
#include <string.h>
#include "sec_api.h"
#include <fwk.h>

void *g_msgHandle;

static int sendInitDoneMsg()
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    VosMsgHeader msg = EMPTY_MSG_HEADER;
    /*
         * Reuse the event msg to send a VOS_MSG_MDM_INITIALIZED event msg
         * back to smd.  This will trigger stage 2 of the smd SMD_launchOnBoot
         * process.
         */
    msg.type = VOS_MSG_MDM_INITIALIZED;
    msg.src = EID_SMB_INIT;
    msg.dst = EID_SMD;
    msg.flags_event = TRUE;

    if ((ret = vosMsg_send(g_msgHandle, &msg)) != VOS_RET_SUCCESS)
    {
        vosLog_error("MDM init event msg failed. ret=%d", ret);
        return -1;
    }

    return 0;
}


int main(int argc, char *argv[])
{
	int ret = 0;
    int i,j = 0;
    char smbPwdBuf[16];
    char smbPwd[16] = {0};
    char byte[16] = {0};
    char cmd[128] = {0};
    unsigned char xor = 0x0;

    ret=vosMsg_init(EID_SMB_INIT, &g_msgHandle);
    if (ret != VOS_RET_SUCCESS)
    {
        return -1;
    }
    
    while (j < 10)
    {
        if (QtGetSpiLock(g_msgHandle) == 0)
        {
            ret = LoadSmbPwd(smbPwdBuf, sizeof(smbPwdBuf));

            if (ret != 0){
                printf("Fail to load smb passwd, ret = %d\n", ret);
                QtReleaseSpiLock(g_msgHandle);
                return ret;
            }

            if (smbPwdBuf[0] == 0xff && smbPwdBuf[1] == 0xff && smbPwdBuf[2] == 0xff && smbPwdBuf[3] == 0xff
                && smbPwdBuf[4] == 0xff && smbPwdBuf[5] == 0xff && smbPwdBuf[6] == 0xff && smbPwdBuf[7] == 0xff){
                ret = GetRandom(smbPwdBuf, sizeof(smbPwdBuf));
                if (ret != 0){
                    printf("Fail to get random num, ret = %d\n", ret);
                    QtReleaseSpiLock(g_msgHandle);
                    return ret;
                }

                ret = SaveSmbPwd(smbPwdBuf, sizeof(smbPwdBuf));
                if (ret != 0){
                    printf("Fail to save smb passwd, ret = %d\n", ret);
                    //return ret;
                }
            }
            
        	for (i = 0; i < 8; i ++)
            {
                smbPwd[i] = smbPwdBuf[i]%43;
                if (smbPwd[i] > 9 && smbPwd[i] <17)
                {
                    smbPwd[i] -= 10;
                }
                smbPwd[i] += 48;
            }   
            
            printf("smb passwd: %s\n", smbPwd);
            snprintf(cmd, sizeof(cmd), "echo -e \"%s\n%s\" | smbpasswd -a -s network", smbPwd, smbPwd);
            system(cmd);

            ret = LoadData(smbPwdBuf, sizeof(smbPwdBuf));

            if (ret != 0){
                printf("Fail to load smb passwd, ret = %d\n", ret);
                QtReleaseSpiLock(g_msgHandle);
                return ret;
            }

            if (smbPwdBuf[0] == 0xff && smbPwdBuf[1] == 0xff && smbPwdBuf[2] == 0xff && smbPwdBuf[3] == 0xff
                && smbPwdBuf[4] == 0xff && smbPwdBuf[5] == 0xff && smbPwdBuf[6] == 0xff && smbPwdBuf[7] == 0xff){
                ret = GetRandom(smbPwdBuf, sizeof(smbPwdBuf));
                if (ret != 0){
                    printf("Fail to get random num, ret = %d\n", ret);
                    QtReleaseSpiLock(g_msgHandle);
                    return ret;
                }

                ret = SaveData(smbPwdBuf, sizeof(smbPwdBuf));
                if (ret != 0){
                    printf("Fail to save smb passwd, ret = %d\n", ret);
                    //return ret;
                }
            }
            printf("main key:\n");
            for (i = 0; i < 16; i ++)
            {
                printf("0x%.2x ", smbPwdBuf[i]);
            }
            
            printf("\n");
            QtReleaseSpiLock(g_msgHandle);
            break;
        }
        j++;
        sleep(1);
    }

    if ((ret = sendInitDoneMsg()) != 0)
    {
        printf("Fail to send init done msg\n");
        return -1;
    }
	return ret;
}
