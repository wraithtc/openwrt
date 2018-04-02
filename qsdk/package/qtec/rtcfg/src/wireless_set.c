#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "rtcfg_uci.h"
#include "wireless_set.h" 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fwk.h"

int WifiUciAdd(WifiDevice *WifiDeviceInData, WifiIface *WifiIfaceInData)
{	
    int  ret = 0;
	char cmd[256] = {0}; 
	
	//入参判空
	if((NULL == WifiDeviceInData) ||(NULL ==WifiIfaceInData))
	{
		return 1;
	}

    if(0 != WifiDeviceInData->WifiDevice[0])
	{
		memset(cmd,0,256);
		snprintf(cmd,256,"wireless.%s=wifi-device",WifiDeviceInData->WifiDevice);
		ret += rtcfgUciSet(cmd);
		
		if(0 != WifiDeviceInData->Type[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.type=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Type);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Country[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.country=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Country);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Channel[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.channel=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Channel);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Band[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.band=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Band);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Disabled[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.disabled=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Disabled);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Vendor[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.vendor=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Vendor);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Autoch[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.autoch=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Autoch);
			ret += rtcfgUciSet(cmd);
		}	
		if(0 != WifiDeviceInData->bandwidth[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.bw=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->bandwidth);
			ret += rtcfgUciSet(cmd);
		}
	}

	//wifi-iface
	rtcfgUciAdd("wireless","wifi-iface");
	if(0 != WifiIfaceInData->Device[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].device=%s",WifiIfaceInData->Device);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Network[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].network=%s",WifiIfaceInData->Network);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Ssid[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].ssid=%s",WifiIfaceInData->Ssid);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Mode[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].mode=%s",WifiIfaceInData->Mode);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Encryption[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].encryption=%s",WifiIfaceInData->Encryption);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Key[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].key=%s",WifiIfaceInData->Key);
        ret += rtcfgUciSet(cmd);

		rtcfgUciCommit("system");
		AuthMessageSet(WifiIfaceInData->Key);
	}

    if(0 != WifiIfaceInData->Wds[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].wds=%s",WifiIfaceInData->Wds);
        ret += rtcfgUciSet(cmd);
	}
    
    if(0 != WifiIfaceInData->Ifname[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].ifname=%s",WifiIfaceInData->Ifname);
        ret += rtcfgUciSet(cmd);
	}
    
    if(0 != WifiIfaceInData->Hidden[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[-1].hidden=%s",WifiIfaceInData->Hidden);
        ret += rtcfgUciSet(cmd);
	}

	if(0 == ret)
    {
        rtcfgUciCommit("wireless");
        system("/etc/init.d/network reload");
    }

	return ret;
}

int WifiUciDel(char *WifiDevice, int index)
{
	if(NULL == WifiDevice)
	{
		return 1;
	}

	char cmd[256] = {0};
	if(0 != WifiDevice[0])
	{
		snprintf(cmd,256,"wireless.%s",WifiDevice);
		rtcfgUciDel(cmd);
	}

	if(0xffff != index)
	{
		snprintf(cmd,256,"wireless.@wifi-iface[%d]",index);
		rtcfgUciDel(cmd);
	}

	rtcfgUciCommit("wireless");
    system("/etc/init.d/network reload");
    return 0;
}

int WifiUciEdit(WifiDevice *WifiDeviceInData, WifiIface *WifiIfaceInData, int index)
{
	int  ret = 0;
	char cmd[256] = {0}; 
	
	//入参判空
	if((NULL == WifiDeviceInData) ||(NULL ==WifiIfaceInData))
	{
		return 1;
	}

	//wifi device
    if(0 != WifiDeviceInData->WifiDevice[0])
	{
		memset(cmd,0,256);
		snprintf(cmd,256,"wireless.%s=wifi-device",WifiDeviceInData->WifiDevice);
		ret += rtcfgUciSet(cmd);
		
		if(0 != WifiDeviceInData->Type[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.type=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Type);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Country[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.country=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Country);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Channel[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.channel=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Channel);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Band[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.band=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Band);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Disabled[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.disabled=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Disabled);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Vendor[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.vendor=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Vendor);
			ret += rtcfgUciSet(cmd);
		}

		if(0 != WifiDeviceInData->Autoch[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.autoch=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->Autoch);
			ret += rtcfgUciSet(cmd);
		}
		if(0 != WifiDeviceInData->bandwidth[0])
		{
			memset(cmd,0,256);
			snprintf(cmd,256,"wireless.%s.bw=%s",WifiDeviceInData->WifiDevice, WifiDeviceInData->bandwidth);
			ret += rtcfgUciSet(cmd);
		}	
	}

	//wifi-iface
	if(0 != WifiIfaceInData->Device[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].device=%s",index, WifiIfaceInData->Device);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Network[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].network=%s",index, WifiIfaceInData->Network);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Ssid[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].ssid=%s",index, WifiIfaceInData->Ssid);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Mode[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].mode=%s",index, WifiIfaceInData->Mode);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Encryption[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].encryption=%s",index, WifiIfaceInData->Encryption);
        ret += rtcfgUciSet(cmd);
	}

    if(0 != WifiIfaceInData->Key[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].key=%s",index, WifiIfaceInData->Key);
        ret += rtcfgUciSet(cmd);

		rtcfgUciCommit("system");
		AuthMessageSet(WifiIfaceInData->Key);
	}

    if(0 != WifiIfaceInData->Wds[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].wds=%s",index, WifiIfaceInData->Wds);
        ret += rtcfgUciSet(cmd);
	}
    
    if(0 != WifiIfaceInData->Ifname[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].ifname=%s",index, WifiIfaceInData->Ifname);
        ret += rtcfgUciSet(cmd);
	}
    
    if(0 != WifiIfaceInData->Hidden[0])
	{
        memset(cmd,0,256);
		snprintf(cmd,256,"wireless.@wifi-iface[%d].hidden=%s",index, WifiIfaceInData->Hidden);
        ret += rtcfgUciSet(cmd);
	}

	if(0 == ret)
    {
        rtcfgUciCommit("wireless");
        system("/etc/init.d/network reload");
    }

	return ret;
}

WifiIface *WifiInfoGet(int *tablenum)
{
	WifiIface *output = NULL;
	int ret = 0;
	int num = 0;
	int index = -1;
	char cmd[256] = {0};
	char tmp_store[16] ={0};

	while(ret == 0)
    {
        index++;
        memset(cmd,0,256);
        snprintf(cmd,256,"wireless.@wifi-iface[%d]",index);
        memset(tmp_store,0,16);
        ret = rtcfgUciGet(cmd,tmp_store);
    }

	num = index;

    if(num == 0)
    {
        output=NULL;
        *tablenum=0;
        return output;
    }

    output=malloc(num*sizeof(WifiIface));

    for(index=0;index<num;index++)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"wireless.@wifi-iface[%d].mode",index);
        rtcfgUciGet(cmd,output[index].Mode);

        memset(cmd,0,256);
        snprintf(cmd,256,"wireless.@wifi-iface[%d].ssid",index);
        rtcfgUciGet(cmd,output[index].Ssid);

        memset(cmd,0,256);
        snprintf(cmd,256,"wireless.@wifi-iface[%d].encryption",index);
        rtcfgUciGet(cmd,output[index].Encryption);

        memset(cmd,0,256);
        snprintf(cmd,256,"wireless.@wifi-iface[%d].key",index);
        rtcfgUciGet(cmd,output[index].Key);
    }

    *tablenum =num;
    
    return output;
}

int setWifiConfig(WifiConfig *cfg)
{
	int ret = 0;
	char cmd[256] = {0};
    char value[64] = {0};
    int i = 0;

	printf("ssid1:%s,ssid2:%s,key1:%s,key2:%s.\r\n", cfg->Ssid1, cfg->Ssid2, cfg->Key1, cfg->Key2);
    while(0 == ret)
    {
        memset(cmd, 0, sizeof(cmd));
        memset(value, 0, sizeof(value));
        snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].network", i);
        ret = rtcfgUciGet(cmd, value);
        if (0 != ret)
            break;
        if (strncmp(value, "lan", 3))
        {
            i++;
            continue;
        }
        memset(cmd, 0, sizeof(cmd));
        memset(value, 0, sizeof(value));
        snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].device", i);
        ret = rtcfgUciGet(cmd, value);
        if (!strncmp(value, "wifi0", 5))
        {
           if(strlen(cfg->Disabled1) != 0)
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[0].disabled=%s", cfg->Disabled1);
                ret = rtcfgUciSet(cmd);
            }

            if(strlen(cfg->Ssid1) != 0)
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].ssid=%s", i, cfg->Ssid1);
                ret = rtcfgUciSet(cmd);
            }

            if(strlen(cfg->Key1) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].key=%s", i, cfg->Key1);
                ret = rtcfgUciSet(cmd);
            }

            if(strlen(cfg->Hidden1) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].hidden=%s", i, cfg->Hidden1);
                ret = rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Encryption1) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].encryption=%s", i, cfg->Encryption1);
                ret = rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Bandwith1) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[0].bw=%s", cfg->Bandwith1);
                ret = rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Wifimode1) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[0].wifimode=%s", cfg->Wifimode1);
                ret = rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Channel1) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[0].channel=%s", cfg->Channel1);
                ret = rtcfgUciSet(cmd);
            } 
        }
        else if (!strncmp(value, "wifi1", 5))
        {
            if(strlen(cfg->Disabled2) != 0)
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[1].disabled=%s", cfg->Disabled2);
                ret = rtcfgUciSet(cmd);
            }

            if(strlen(cfg->Ssid2) != 0)
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].ssid=%s", i, cfg->Ssid2);
                ret = rtcfgUciSet(cmd);
            }

            if(strlen(cfg->Key2) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].key=%s", i, cfg->Key2);
                ret = rtcfgUciSet(cmd);
            }

            if(strlen(cfg->Hidden2) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].hidden=%s", i, cfg->Hidden2);
                ret = rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Encryption2) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-iface[%d].encryption=%s", i, cfg->Encryption2);
                ret+= rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Bandwith2) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[1].bw=%s", cfg->Bandwith2);
                ret = rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Wifimode2) != 0 )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[1].wifimode=%s", cfg->Wifimode2);
                ret = rtcfgUciSet(cmd);
            }

        	if(strlen(cfg->Channel2) != 0 )
            {
                if (util_strncmp(cfg->Channel2, "auto", sizeof(cfg->Channel2)) == 0)
                {
                    memset(cmd,0,256);
                    snprintf(cmd,256,"wireless.@wifi-device[1].channel=149");
                    ret = rtcfgUciSet(cmd);
                }
                else
                {
                    memset(cmd,0,256);
                    snprintf(cmd,256,"wireless.@wifi-device[1].channel=%s", cfg->Channel2);
                    ret = rtcfgUciSet(cmd);
                }
                memset(cmd,0,256);
                snprintf(cmd,256,"wireless.@wifi-device[1].real_channel=%s", cfg->Channel2);
                ret = rtcfgUciSet(cmd);
            }
        }
        i++;
    }
    

	rtcfgUciCommit("wireless");
	printf("wifi set is committed.\n");
	//system("wifi up");
    
    return 0;
}

int getWifiConfig(WifiConfig *cfg)
{
	int ret = 0;
	char cmd[256] = {0};
	
	memset(cfg, 0, sizeof(WifiConfig));
	
	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[0].disabled");
    rtcfgUciGet(cmd,cfg->Disabled1);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[0].device");
    rtcfgUciGet(cmd,cfg->Device1);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[0].network");
    rtcfgUciGet(cmd,cfg->Network1);
    
    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[0].ssid");
    rtcfgUciGet(cmd,cfg->Ssid1);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[0].key");
    rtcfgUciGet(cmd,cfg->Key1);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[0].hidden");
    rtcfgUciGet(cmd,cfg->Hidden1);

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[0].encryption");
    rtcfgUciGet(cmd,cfg->Encryption1);

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[0].bw");
    rtcfgUciGet(cmd,cfg->Bandwith1);

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[0].wifimode");
    rtcfgUciGet(cmd,cfg->Wifimode1);

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[0].channel");
    rtcfgUciGet(cmd,cfg->Channel1);
	
	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[1].disabled");
    rtcfgUciGet(cmd,cfg->Disabled2);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[1].device");
    rtcfgUciGet(cmd,cfg->Device2);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[1].network");
    rtcfgUciGet(cmd,cfg->Network2);
    
    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[1].ssid");
    rtcfgUciGet(cmd,cfg->Ssid2);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[1].key");
    rtcfgUciGet(cmd,cfg->Key2);

    memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[1].hidden");
    rtcfgUciGet(cmd,cfg->Hidden2); 

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-iface[1].encryption");
    rtcfgUciGet(cmd,cfg->Encryption2);

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[1].bw");
    rtcfgUciGet(cmd,cfg->Bandwith2);

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[1].wifimode");
    rtcfgUciGet(cmd,cfg->Wifimode2);

	memset(cmd,0,256);
    snprintf(cmd,256,"wireless.@wifi-device[1].real_channel");
    rtcfgUciGet(cmd,cfg->Channel2);
    
    return 0;
}

int QtSetGuestWifi(QT_GUEST_WIFI_CFG *guestWifiCfg)
{
    char cmd[128] = {0};
    char iptable_cmd[128] = {0};
    char dev[32] = {0};
    char lanIp[32] = {0};
    char network[32] = {0};
    char lanNetmask[32] = {0};
    char netIp[32];
    char name[32]={0};
    char value[32] = {0};
    char disabled[4] = {0};
    char ifName[16] = {0};
    unsigned long ip, netmask;
    struct in_addr addr;
    int i = 0, j = 0, athNum = 0;
    int ret = 0;
    
    while (0 == ret)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].device", i);
        memset(dev, 0, sizeof(dev));
        ret = rtcfgUciGet(cmd, dev);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].network", i);
        memset(network, 0, sizeof(network));
        rtcfgUciGet(cmd, network);
        if (!util_strncmp(dev, "wifi0", sizeof(dev)) && util_strncmp(network, "guest", sizeof(network)))
        {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].disabled", i);
            rtcfgUciGet(cmd, disabled);
            if (!(util_strlen(disabled)&&atoi(disabled)))
                athNum++;
        }
        i++;
    }

    if (athNum > 0)
    {
        UTIL_SNPRINTF(ifName, sizeof(ifName), "ath0%d", athNum);
    }
    else
    {
        UTIL_STRNCPY(ifName, "ath0", sizeof(ifName));
    }
    rtcfgUciGet("network.lan.ipaddr",lanIp);
    rtcfgUciGet("network.lan.netmask",lanNetmask);
    ip = inet_addr(lanIp);
    netmask = inet_addr(lanNetmask);
    ip &= netmask;printf("ip:%d\n", ip);
    memcpy(&addr, &ip, 4); 
    strncpy(netIp, inet_ntoa(addr), sizeof(netIp));
    i = 0;
    while (netmask)
    {
        netmask = netmask >> 1;
        i++;
    }
    printf("netIp:%s/%d\n", netIp,i);
    if (guestWifiCfg->enable)
    {
        rtcfgUciGet("wireless.guest.network", network);
        if (strncmp(network, "guest", sizeof(network)))
        {
            rtcfgUciSet("wireless.guest=wifi-iface");
        }

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"wireless.@wifi-iface[0].device");
        rtcfgUciGet(cmd,dev);

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"wireless.guest.device=%s", dev);
        rtcfgUciSet(cmd);

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"wireless.guest.ssid=%s", guestWifiCfg->name);
        rtcfgUciSet(cmd);

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"wireless.guest.hidden=%d", guestWifiCfg->isHide);
        rtcfgUciSet(cmd);
        
        rtcfgUciSet("wireless.guest.mode=ap");
        rtcfgUciSet("wireless.guest.encryption=none");
        rtcfgUciSet("wireless.guest.network=guest");
        rtcfgUciSet("wireless.guest.disabled=0");
        rtcfgUciCommit("wireless");

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "network.guest=interface");
        rtcfgUciSet(cmd);

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "network.guest._orig_ifname=%s", ifName);
        rtcfgUciSet(cmd);
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "network.guest.ifname=%s", ifName);
        rtcfgUciSet(cmd);
        rtcfgUciSet("network.guest._orig_bridge=false");
        
        rtcfgUciSet("network.guest.proto=static");
        rtcfgUciSet("network.guest.netmask=255.255.255.0");

        if (!strncmp(lanIp, GUEST_WIFI_IP, sizeof(lanIp)))
        {
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.ipaddr=%s", GUEST_WIFI_IP_EX); 
            rtcfgUciSet(cmd);
#if 0
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.gateway=%s", GUEST_WIFI_IP_EX); 
            rtcfgUciSet(cmd);
#endif            
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.dns=%s", GUEST_WIFI_IP_EX); 
            rtcfgUciSet(cmd);

            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.broadcast=%s", GUEST_WIFI_BRAODCAST_IP_EX); 
            rtcfgUciSet(cmd);
        }
        else
        {
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.ipaddr=%s", GUEST_WIFI_IP); 
            rtcfgUciSet(cmd);
#if 0
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.gateway=%s", GUEST_WIFI_IP); 
            rtcfgUciSet(cmd);
#endif             
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.dns=%s", GUEST_WIFI_IP); 
            rtcfgUciSet(cmd);

            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.guest.broadcast=%s", GUEST_WIFI_BRAODCAST_IP); 
            rtcfgUciSet(cmd);
        }
        rtcfgUciCommit("network");

        rtcfgUciSet("dhcp.guest=dhcp");
        rtcfgUciSet("dhcp.guest.start=100");
        rtcfgUciSet("dhcp.guest.leasetime=12h");
        rtcfgUciSet("dhcp.guest.limit=150");
        rtcfgUciSet("dhcp.guest.interface=guest");

        rtcfgUciCommit("dhcp");

        rtcfgUciSet("firewall.guest_wifi_rule=rule");
        rtcfgUciSet("firewall.guest_wifi_rule.name='guest wifi rule'");
        rtcfgUciSet("firewall.guest_wifi_rule.src=lan");
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"firewall.guest_wifi_rule.device=%s", ifName); 
        rtcfgUciSet(cmd);
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"firewall.guest_wifi_rule.dest_ip=%s/%d", netIp, i); 
        rtcfgUciSet(cmd);
        rtcfgUciSet("firewall.guest_wifi_rule.target=DROP");
        
        memset(cmd,0,sizeof(cmd));
        snprintf(iptable_cmd, sizeof(iptable_cmd), "iptables -I FORWARD -i %s -d %s/%d -j DROP", ifName, netIp, i);
        snprintf(cmd, sizeof(cmd), "echo '%s' >> /etc/firewall.user", iptable_cmd);
        system(cmd);

        for (j = 0; j < 2; j++)
        {
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"firewall.@zone[%d].name", j); 
            rtcfgUciGet(cmd, name);

            if (!strncmp(name, "lan", sizeof(name)))
            {
                break;
            }
        }
        if ( j == 2)
        {
            printf("can not find firewall lan zone!\n");
            return -1;
        }

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"firewall.@zone[%d].network=lan\ guest", j); 
        rtcfgUciSet(cmd);
        rtcfgUciCommit("firewall");

    }
    else
    {
        rtcfgUciSet("wireless.guest.disabled=1");
        rtcfgUciCommit("wireless");
    
        rtcfgUciDel("network.guest");
        rtcfgUciCommit("network");
        rtcfgUciDel("dhcp.guest");
        rtcfgUciCommit("dhcp");

        rtcfgUciDel("firewall.guest_wifi_rule");
        for (j = 0; j < 2; j++)
        {
            memset(cmd,0,sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"firewall.@zone[%d].name", j); 
            rtcfgUciGet(cmd, name);

            if (!strncmp(name, "lan", sizeof(name)))
            {
                break;
            }
        }
        if ( j == 2)
        {
            printf("can not find firewall lan zone!\n");
            return -1;
        }

        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"firewall.@zone[%d].network=lan", j); 
        rtcfgUciSet(cmd);
        rtcfgUciCommit("firewall");
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"sed -i '/iptables -I FORWARD -i %s -d/d' /etc/firewall.user", ifName); 
        system(cmd);
    }

    return 0;
    
}


int QtGetGuestWifi(QT_GUEST_WIFI_CFG *guestWifiCfg)
{
    char network[32] = {0};
    char isHide[8] = {0};
    int ret = 0;
    FILE *fp;
    char cmd[128] = {0};
    int i = 0;
    char ifName[32] = {0};
    char disabled[4] = {0};
    
    rtcfgUciGet("wireless.guest.network", network);
    if (!strncmp(network, "guest", sizeof(network)))
    {
        rtcfgUciGet("wireless.guest.disabled", disabled);
        guestWifiCfg->enable = util_strlen(disabled)?(atoi(disabled)?0:1):1;
        rtcfgUciGet("wireless.guest.ssid", guestWifiCfg->name);
        rtcfgUciGet("wireless.guest.hidden", isHide);
        guestWifiCfg->isHide = atoi(isHide);
        rtcfgUciGet("network.guest.ifname", ifName);
        snprintf(cmd, sizeof(cmd), "iwinfo %s assoclist | grep SNR > %s", ifName, GUEST_USER_INFO_FILE);
        system(cmd);
        fp = fopen(GUEST_USER_INFO_FILE, "r");
        if (fp == NULL)
        {
            printf("can't finde guest user info file \n");
            return -1;
        }
        while(fgets(cmd, sizeof(cmd), fp))
        {
            if (strstr(cmd, "00:00:00:00:00:00"))
                continue;
            i++;
        }
        guestWifiCfg->guestUserNum = i;
        fclose(fp);
    }
    else
    {
        guestWifiCfg->enable = 0;
    }

    return ret;
}


int QtWdsGetBasicCfg(QT_WDS_BASIC_CFG *wdsBasicCfg)
{
    char enable[8] = {0};
    char auto_switch[8] = {0};
    char ssid[32] = {0};
    char status[8] = {0};
    char mac[32] = {0};
    FILE *fp;
    char lanIp[32] = {0};
    char lanNetmask[32] = {0};
    unsigned long ip, netmask, wdsIp, wdsNetmask, lanSugIp, lanSugNetmask;
    QT_WDS_WIFI_NET_INFO wdsInfo;

    rtcfgUciGet("wireless.wdscfg.enable", enable);

    if (strlen(enable) == 0 || atoi(enable) == 0)
    {
        wdsBasicCfg->enable = 0;
        return 0;
    }

    wdsBasicCfg->enable = atoi(enable);
    rtcfgUciGet("wireless.wdscfg.auto_switch", auto_switch);
    if (strlen(auto_switch) == 0)
    {
        wdsBasicCfg->auto_switch = 0;
    }
    else
    {
        wdsBasicCfg->auto_switch = atoi(auto_switch);
    }

    rtcfgUciGet("wireless.wds.ssid", ssid);
    strncpy(wdsBasicCfg->ssid, ssid, sizeof(wdsBasicCfg->ssid));

    rtcfgUciGet("wireless.wds.mac_addr", mac);
    strncpy(wdsBasicCfg->mac, mac, sizeof(wdsBasicCfg->mac));

    system("ubus call network.interface.wwan status | grep '\"up\"' | sed -e 's/^.*: \\(.*\\),/\\1/g' > /tmp/wdsup");
    fp = fopen("/tmp/wdsup","r");
    if (fp != NULL)
    {
        fgets(status, sizeof(status), fp);
        if (!strncmp(status, "true", 4))
        {
            wdsBasicCfg->status = 1;
            rtcfgUciGet("network.lan.ipaddr",lanIp);
            rtcfgUciGet("network.lan.netmask",lanNetmask);
            ip = inet_addr(lanIp);
            netmask = inet_addr(lanNetmask);
            ip &= netmask;printf("ip:%d\n", ip);

            QtWdsGetStatus(&wdsInfo);
            wdsIp = inet_addr(wdsInfo.ipaddr);
            wdsNetmask = inet_addr(wdsInfo.netmask);
            wdsIp &= wdsNetmask;
            
            if (wdsIp == ip)
            {
                wdsBasicCfg->isChangeLanIp = 1;
                lanSugIp = inet_addr(WDS_LAN_IP);
                lanSugNetmask = inet_addr(WDS_LAN_NETMASK);
                lanSugIp &= lanSugNetmask;

                if (wdsIp != lanSugIp)
                {
                    UTIL_STRNCPY(wdsBasicCfg->suggestLanIp, WDS_LAN_IP, sizeof(wdsBasicCfg->suggestLanIp));
                }
                else
                {
                    UTIL_STRNCPY(wdsBasicCfg->suggestLanIp, WDS_LAN_IP_2, sizeof(wdsBasicCfg->suggestLanIp));
                }
            }
            else
            {
                wdsBasicCfg->isChangeLanIp = 0;
            }
        }
        else
        {
            wdsBasicCfg->status = 0;
            wdsBasicCfg->isChangeLanIp = 0;
        }

        fclose(fp);
    }
    return 0;
}


int QtWdsScanWifi(QT_WDS_WIFI_INFO *wds, int *len)
{
    FILE *fp1 = NULL, *fp2 = NULL, *fp3 = NULL, *fp4 = NULL, *fp5 = NULL;
    char buf[4096] = {0};
    char *infoStr;
    char *tmpStr;
    char *tmpBuf;
    int i = 0, ret = 0, n;
    char mac[64] = {0};
    char ssid[64] = {0};
    char channel[8] = {0};
    char power[8] = {0};
    char enc[64] = {0};
    
    system("iwinfo ath0 scan > /tmp/2_4gwifiinfo");
    system("iwinfo ath1 scan > /tmp/5gwifiinfo");

    system("grep Address: /tmp/2_4gwifiinfo |sed -e 's/^.*Address: \\([^\\n\\r]*\\)/\\1/g' > /tmp/mac");
    system("grep ESSID: /tmp/2_4gwifiinfo |sed -e 's/^.*: \"\\([^\\n\\r]*\\)\"/\\1/g' > /tmp/ssid");
    system("grep Channel: /tmp/2_4gwifiinfo |sed -e's/^.*l: \\(.*\\)/\\1/g' > /tmp/channel");
    system("grep Quality: /tmp/2_4gwifiinfo |sed -e's/^.*y: \\(.*\\)\\/.*/\\1/g' > /tmp/power");
    system("grep Encryption: /tmp/2_4gwifiinfo |sed -e's/^.*n: \\([^\\n\\r]*\\)/\\1/g' > /tmp/encrypt");

    fp1 = fopen("/tmp/mac", "r");
    if (!fp1)
    {
        printf("no mac file!\n");
        return -1;
    }

    fp2 = fopen("/tmp/ssid", "r");
    if (!fp1)
    {
        printf("no ssid file!\n");
        return -1;
    }

    fp3 = fopen("/tmp/channel", "r");
    if (!fp1)
    {
        printf("no channel file!\n");
        return -1;
    }

    fp4 = fopen("/tmp/power", "r");
    if (!fp1)
    {
        printf("no power file!\n");
        return -1;
    }

    fp5 = fopen("/tmp/encrypt", "r");
    if (!fp1)
    {
        printf("no encrypt file!\n");
        return -1;
    }

    while (fgets(mac, sizeof(mac), fp1))
    {
        sscanf(mac,"%[^\n\r]", wds[i].mac);

        if (fgets(ssid, sizeof(ssid), fp2))
        {
            sscanf(ssid,"%[^\n\r]", wds[i].ssid);
        }

        if (fgets(channel, sizeof(channel), fp3))
        {
            wds[i].channel = atoi(channel);
        }

        if (fgets(power, sizeof(power), fp4))
        {
            wds[i].power = atoi(power);
        }

        if (fgets(enc, sizeof(enc), fp5))
        {
            sscanf(enc,"%[^\n\r]", wds[i].encryption);
        }

        wds[i].mode = 0;
        i++;

        if (i == *len)
            break;
    }

    if (i < *len)
    {
        system("grep Address: /tmp/5gwifiinfo |sed -e 's/^.*Address: \\([^\\n\\r]*\\)/\\1/g' > /tmp/mac");
        system("grep ESSID: /tmp/5gwifiinfo |sed -e 's/^.*: \"\\([^\\n\\r]*\\)\"/\\1/g' > /tmp/ssid");
        system("grep Channel: /tmp/5gwifiinfo |sed -e's/^.*l: \\(.*\\)/\\1/g' > /tmp/channel");
        system("grep Quality: /tmp/5gwifiinfo |sed -e's/^.*y: \\(.*\\)\\/.*/\\1/g' > /tmp/power");
        system("grep Encryption: /tmp/5gwifiinfo |sed -e's/^.*n: \\([^\\n\\r]*\\)/\\1/g' > /tmp/encrypt");

        fp1 = fopen("/tmp/mac", "r");
        if (!fp1)
        {
            printf("no mac file!\n");
            return -1;
        }

        fp2 = fopen("/tmp/ssid", "r");
        if (!fp1)
        {
            printf("no ssid file!\n");
            return -1;
        }

        fp3 = fopen("/tmp/channel", "r");
        if (!fp1)
        {
            printf("no channel file!\n");
            return -1;
        }

        fp4 = fopen("/tmp/power", "r");
        if (!fp1)
        {
            printf("no power file!\n");
            return -1;
        }

        fp5 = fopen("/tmp/encrypt", "r");
        if (!fp1)
        {
            printf("no encrypt file!\n");
            return -1;
        }

        while (fgets(mac, sizeof(mac), fp1))
        {
            sscanf(mac,"%[^\n]", wds[i].mac);

            if (fgets(ssid, sizeof(ssid), fp2))
            {
                sscanf(ssid,"%[^\n]", wds[i].ssid);
            }

            if (fgets(channel, sizeof(channel), fp3))
            {
                wds[i].channel = atoi(channel);
            }

            if (fgets(power, sizeof(power), fp4))
            {
                wds[i].power = atoi(power);
            }

            if (fgets(enc, sizeof(enc), fp5))
            {
                sscanf(enc,"%[^\n]", wds[i].encryption);
            }

            wds[i].mode = 1;
            i++;

            if (i == *len)
                break;
        }
    }
    
    *len = i;
    if (fp1)
        fclose(fp1);
    if (fp2)
        fclose(fp2);
    if (fp3)
        fclose(fp3);
    if (fp4)
        fclose(fp4);
    if (fp5)
        fclose(fp5);
    return 0;
}


int QtWdsSetUp(QT_WDS_WIFI_INFO *wdsInfo)
{
    char cmd[128]={0};
    char *device = wdsInfo->mode?"wifi1":"wifi0";
    char dev[32] = {0};
    char network[32] = {0};
    int i = 0, ret = 0;
    int disabled[8] = {0};
    char value[32] = {0};
    char *encryption;
    QT_GUEST_WIFI_CFG guestWifiCfg = {0};
    int athNum = 0;
    char ifName[16] = {0};
    
    if (strlen(wdsInfo->password) < 8 && strlen(wdsInfo->password) != 0)
    {
        printf("wifi password can not be less than 8!\n");
        return -2;
    }

    while (0 == ret)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].device", i);
        memset(dev, 0, sizeof(dev));
        ret = rtcfgUciGet(cmd, dev);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].network", i);
        memset(network, 0, sizeof(network));
        rtcfgUciGet(cmd, network);
        if (!util_strncmp(dev, device, sizeof(dev)) && util_strncmp(network, "wwan", sizeof(network)))
        {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].disabled", i);
            rtcfgUciGet(cmd, disabled);
            if (!(util_strlen(disabled)&&atoi(disabled)))
                athNum++;
        }
        i++;
    }

    if (athNum > 0)
    {
        UTIL_SNPRINTF(ifName, sizeof(ifName), "%s%d", wdsInfo->mode?"ath1":"ath0", athNum);
    }
    else
    {
        UTIL_STRNCPY(ifName, wdsInfo->mode?"ath1":"ath0", sizeof(ifName));
    }

    rtcfgUciSet("wireless.wds=wifi-iface");

    snprintf(cmd, sizeof(cmd), "wireless.wds.ssid=%s", wdsInfo->ssid);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "wireless.wds.key=%s", wdsInfo->password);
    rtcfgUciSet(cmd);

    if (strncmp(wdsInfo->encryption, "WPA2 PSK", 8) == 0)
    {
        encryption = "psk2";
    }
    else if (strncmp(wdsInfo->encryption, "mixed", 5) == 0)
    {
        encryption = "psk-mixed";
    }
    else if (strncmp(wdsInfo->encryption, "WPA PSK", 7) == 0)
    {
        encryption = "psk";
    }
    else if (strncmp(wdsInfo->encryption, "none", 4) == 0)
    {
        encryption = "none";
    }
    else
    {
        encryption = "psk2";
    }
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "wireless.wds.encryption=%s", encryption);
    rtcfgUciSet(cmd);

    rtcfgUciSet("wireless.wds.mode=sta");
    rtcfgUciSet("wireless.wds.network=wwan");
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "wireless.wds.device=%s", wdsInfo->mode?"wifi1":"wifi0");
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "wireless.wds.mac_addr=%s", wdsInfo->mac);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("wireless");
    
    rtcfgUciSet("network.wwan=interface");
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.wwan.ifname=%s", ifName);
    rtcfgUciSet(cmd);
    rtcfgUciSet("network.wwan.proto=dhcp");
    rtcfgUciSet("network.wan=tmp");
    rtcfgUciCommit("network");

    rtcfgUciSet("firewall.@zone[1].network=wan\ wan6\ wwan");
    rtcfgUciCommit("firewall");

    
    return 0;
}


int QtWdsSetBasicCfg(QT_WDS_BASIC_CFG *wdsBasicCfg)
{
    char cmd[128] = {0};
    char value[64] = {0};
    int ret = 0;
    int i = 0;
    char guestWifiIfname[BUFLEN_32] = {0};

    rtcfgUciSet("wireless.wdscfg=config");
    snprintf(cmd, sizeof(cmd), "wireless.wdscfg.enable=%d", wdsBasicCfg->enable);
    rtcfgUciSet(cmd);
    if (wdsBasicCfg->enable == 0)
    {
        rtcfgUciDel("wireless.wds");
        rtcfgUciDel("wireless.wds_client");
        #if 0
        while(0 == ret)
        {   
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].network", i);
            ret = rtcfgUciGet(cmd, value);
            if (ret != 0)
            {
                break;
            }
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "wireless.@wifi-iface[%d].disabled=0", i);
            rtcfgUciSet(cmd);
            i++;
        }
        #endif
        rtcfgUciSet("firewall.@zone[1].network=wan\ wan6");
        rtcfgUciCommit("firewall");

        rtcfgUciDel("network.wwan");
        rtcfgUciGet("network.guest.ifname", guestWifiIfname);
        if (util_strlen(guestWifiIfname) != 0)
        {
            rtcfgUciSet("network.guest.ifname=ath01");
            rtcfgUciSet("network.guest._orig_ifname=ath01");
        }
        rtcfgUciSet("network.wan=interface");
        rtcfgUciCommit("network");
    }
    rtcfgUciCommit("wireless");

    

    return 0;
}

int QtWdsGetStatus(QT_WDS_WIFI_NET_INFO *wdsNetInfo)
{
    FILE *fp;
    char netmasklen[8] = {0};
    int len;
    char lanIp[32] = {0};
    char lanNetmask[32] = {0};
    unsigned long ip, netmask;
    
    system("ubus call network.interface.wwan status | grep \"address\" | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' >/tmp/wwanaddr");
    system("ubus call network.interface.wwan status | grep nexthop | grep -oE '([0-9]{1,3}.){3}[0-9]{1,3}' | sed -n 's/\\(^[^0].*\\)/\\1/p' > /tmp/wwangateway");
    system("ubus call network.interface.wwan status | grep mask | sed -n 's/^.*\"mask\": \\([^,]*\\)$/\\1/p' > /tmp/wwanmask");
    system("ubus call network.interface.wwan status | sed -n 's/^\\t*\"\\([0-9\.]*\\)\"$/\\1/p' > /tmp/wwandns");

    fp = fopen("/tmp/wwanaddr", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%s",wdsNetInfo->ipaddr);
        fclose(fp);
    }

    fp = fopen("/tmp/wwangateway", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%s",wdsNetInfo->gateway);
        fclose(fp);
    }

    fp = fopen("/tmp/wwanmask", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%s",netmasklen);
        len = atoi(netmasklen);
        UTIL_subnetLenToSubnetMask((UINT32)len, wdsNetInfo->netmask, (UINT32)sizeof(wdsNetInfo->netmask));
        
        fclose(fp);
    }

    fp = fopen("/tmp/wwandns", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%s",wdsNetInfo->dns);
        fclose(fp);
    }

    return 0;
}

