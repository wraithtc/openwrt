#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "rtcfg_uci.h"
#include "wireless_set.h"

int main()
{
#if 0
    WirelessCfg stWirelessCfg = {0};
    strcpy(stWirelessCfg.WifiDevice, "mt7603e");
    strcpy(stWirelessCfg.Key, "87654321");
    strcpy(stWirelessCfg.Wds, "1");

    printf("enter wireless test \r\n");
    (void)WirelessUciSet(&stWirelessCfg);

#endif
#if 1
    QT_WDS_WIFI_INFO wds[64] = {0};
    int i = 0;
    int len = 64;
    QtWdsScanWifi(wds, &len);

    for (i = 0; i < len; i++)
    {
        printf("ssid:%s\n", wds[i].ssid);
        printf("mac:%s\n", wds[i].mac);
        printf("channel:%d\n", wds[i].channel);
        printf("power:%d\n", wds[i].power);
        printf("encryption:%s\n", wds[i].encryption);
        printf("\n");
    }
#endif
#if 0
    QT_WDS_WIFI_INFO wdsInfo = {"360WiFi-111222", 0, 0, 0, "psk2", "", "11111111"};

    QtWdsSetUp(&wdsInfo);
#endif
#if 0
    QT_WDS_BASIC_CFG wdscfg;
    wdscfg.enable = 0;

    QtWdsSetBasicCfg(&wdscfg);
#endif
#if 0
    QT_WDS_WIFI_NET_INFO wdsNetInfo = {0};
    QtWdsGetStatus(&wdsNetInfo);
    printf("ip:%s, netmask:%s, dns:%s, gateway:%s\n", wdsNetInfo.ipaddr, wdsNetInfo.netmask, wdsNetInfo.dns, wdsNetInfo.gateway);
#endif
    return 0;
}
