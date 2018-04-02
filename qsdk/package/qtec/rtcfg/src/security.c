#include <stdio.h>
#include <string.h>
#include "sec_api.h"

int QtGetSmbPwd(char *smbPwd, int len)
{
    int ret = 0;
    int i;
    char smbPwdBuf[16];
    char byte[16] = {0};
    char cmd[128] = {0};
    unsigned char xor = 0x0;

    if (len < 8){
        printf("smbpwd buff too shor\n");    
        return -3;
    }
    ret = LoadSmbPwd(smbPwdBuf, sizeof(smbPwdBuf));

    if (ret != 0){
        printf("Fail to load smb passwd, ret = %d\n", ret);
        return ret;
    }

    if (smbPwdBuf[0] == 0xff && smbPwdBuf[1] == 0xff && smbPwdBuf[2] == 0xff && smbPwdBuf[3] == 0xff
        && smbPwdBuf[4] == 0xff && smbPwdBuf[5] == 0xff && smbPwdBuf[6] == 0xff && smbPwdBuf[7] == 0xff){
        ret = GetRandom(smbPwdBuf, sizeof(smbPwdBuf));
        if (ret != 0){
            printf("Fail to get random num, ret = %d\n", ret);
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
    
	return ret;
}

