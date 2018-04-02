#include <stdio.h>
#include <string.h>
#include "wifi_txpower.h"
#include "rtcfg_uci.h"
#include "librtcfg.h"


/**********************************************
    函数名：QtWifiTxPowerEmptyConfig
    功能：  清空uci配置
    创建人：tongchao
    创建时间：2017/7/20
***********************************************/

void QtWifiTxPowerEmptyConfig()
{
    FILE *fp = fopen(TXPOWER_CONFIG_FILE, "rb");
    FILE *fp1;

    if (fp == NULL)
    {
        fp1 = fopen(TXPOWER_CONFIG_FILE, "ab+");
        if (fp1 != NULL)
        {
            fclose(fp1);
        }
    }
    else
    {
        fclose(fp);
        fp1 = fopen(TXPOWER_CONFIG_FILE, "w");
        if (fp1 != NULL)
        {
            fclose(fp1);
        }
    }
}


/**********************************************
    函数名：QtWifiTxPowerSaveConfig
    功能：  写入到uci配置
    创建人：tongchao
    创建时间：2017/7/20
***********************************************/
bool QtWifiTxPowerSaveConfig(int mode)
{
    bool ret = false;
    int res;
    char value[16] = {0};
    char cmd[128] = {0};
    QtWifiTxPowerEmptyConfig();

    rtcfgUciSet("wifiTxPower.txpower=txpower");
    snprintf(cmd, sizeof(cmd), "wifiTxPower.txpower.mode=%d", mode);
    rtcfgUciSet(cmd);

     
    return ret;
}

/**********************************************
    函数名：QtWifiTxPowerLoadConfig
    功能：  从uci中读取配置
    创建人：tongchao
    创建时间：2017/7/20
***********************************************/
bool QtWifiTxPowerLoadConfig(int *mode)
{
    char value[16] = {0};

    rtcfgUciGet("wifiTxPower.txpower.mode", value);
    if (strlen(value) == 0 || atoi(value) < 0 || atoi(value) > 2)
    {
        *mode = TXPOWER_MODE_HIGH;
    }
    else
    {
        *mode = atoi(value);
    }

    return true;
}


int QtSetWifTxpower(int mode)
{
    int txpower = 0;

    if (mode >2 || mode < 0)
    {
        printf("invalid mode value, mode = %d\n", mode);
        return -1;
    }

    switch(mode)
    {
        case TXPOWER_MODE_LOW:
            system("iwconfig ath0 txpower 10");
            system("iwconfig ath1 txpower 10");
            break;

        case TXPOWER_MODE_MEDIUM:
            system("iwconfig ath0 txpower 20");
            system("iwconfig ath1 txpower 20");
            break;

        case TXPOWER_MODE_HIGH:
            system("iwconfig ath0 txpower 29");
            system("iwconfig ath1 txpower 29");
            break;

        default:
            printf("unkown mode %d\n", mode);
            break;
    }
    QtWifiTxPowerSaveConfig(mode);
    return 0;
}

int QtGetWifiMode(int* mode)
{
    int ret;

    ret = QtWifiTxPowerLoadConfig(mode);

    return (ret == 1)?0:-1;
    
}

