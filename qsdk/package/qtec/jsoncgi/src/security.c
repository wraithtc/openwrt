#include "basic.h"

int proc_get_smb_pwd(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj;
    int ret ;
    char smbPwd[16] = {0};
    
    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        return ERR_INTERNALLOGIC_WRONG;	
    }

    ret = QtGetSmbPwd(smbPwd, sizeof(smbPwd));
    if (ret != 0)
    {
        printf("Fail to get smb pwd, ret = %d\n", ret);
        return ERR_GET_SMB_PSW_FAIL;
    }
    cJSON_AddItemToObject(dataObj, "username", cJSON_CreateString("network"));
    cJSON_AddItemToObject(dataObj, "password", cJSON_CreateString(smbPwd));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);

    return ret;
}
