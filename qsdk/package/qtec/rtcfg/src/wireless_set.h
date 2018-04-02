#ifndef WIRELESS_SET_H
#define WIRELESS_SET_H

#define GUEST_WIFI_IP    "192.168.222.1"
#define GUEST_WIFI_BRAODCAST_IP    "192.168.222.255"
#define GUEST_WIFI_IP_EX    "192.168.223.1"
#define GUEST_WIFI_BRAODCAST_IP_EX    "192.168.223.255"
#define GUEST_USER_INFO_FILE    "/tmp/guest_user_info"

#define WDS_LAN_IP    "192.168.223.1"
#define WDS_LAN_IP_2    "192.168.224.1"
#define WDS_LAN_NETMASK    "255.255.255.0"
typedef struct _WifiDevice
{
	char WifiDevice[256];
	char Type[256];
	char Country[256];
	char Channel[256];
	char Band[256];
	char Disabled[256];
	char Vendor[256];
	char Autoch[256];
	char bandwidth[256];
}WifiDevice;

typedef struct _WifiIface
{
	char Device[256];
	char Network[256];
	char Mode[256];
	char Ssid[256];
	char Encryption[256];
	char Key[256];
	char Wds[256];
	char Ifname[256];
	char Hidden[256];
}WifiIface;

typedef struct _WifiConfig
{
    char Device1[256];
    char Network1[256];
	char Disabled1[256];
	char Ssid1[256];
	char Key1[256];
	char Hidden1[256];
	char Encryption1[256];
	char Bandwith1[256];
	char Wifimode1[256];
	char Channel1[256];
    char Device2[256];
    char Network2[256];
	char Disabled2[256];
	char Ssid2[256];
	char Key2[256];
	char Hidden2[256];
	char Encryption2[256];
	char Bandwith2[256];
	char Wifimode2[256];
	char Channel2[256];
}WifiConfig;

typedef struct{
    int enable;  //开关，0表示关，1表示开
    char name[64];  //访客wifi ssid名称
    int isHide;  //是否隐藏访客wifi
    int guestUserNum;
}QT_GUEST_WIFI_CFG;

typedef struct{
    int enable;  //wds开关
    char ssid[64];  //wds连接wifi名称
    char mac[32];  //wds连接wifi mac地址
    int auto_switch;  //自动切换开关
    int status;
    int isChangeLanIp; //是否需要改变lan侧ip
    char suggestLanIp[32]; //建议更改的lan侧ip
}QT_WDS_BASIC_CFG;


typedef struct{
    char ssid[64];  //wifi名称
    int power; //wifi信号强度
    int mode; //wifi类型，0表示2.4G,1表示5G
    int channel; //wifi 信道
    char encryption[64]; //wifi加密模式
    char mac[32];  //mac地址
    char password[64];  //wifi密码
}QT_WDS_WIFI_INFO;

typedef struct{
    char ipaddr[32];  //wds ip地址
    char netmask[32]; //wds 子网掩码
    char gateway[32]; //wds 默认网关
    char dns[32]; //wds dns
}QT_WDS_WIFI_NET_INFO;


extern int WifiUciAdd(WifiDevice *WifiDeviceInData, WifiIface *WifiIfaceInData);
extern int WifiUciDel(char *WifiDevice, int index);
int WifiUciEdit(WifiDevice *WifiDeviceInData, WifiIface *WifiIfaceInData, int index);
WifiIface *WifiInfoGet(int *tablenum);

int getWifiConfig(WifiConfig *cfg);
int setWifiConfig(WifiConfig *cfg);
int QtSetGuestWifi(QT_GUEST_WIFI_CFG *guestWifiCfg);
int QtGetGuestWifi(QT_GUEST_WIFI_CFG *guestWifiCfg);
int QtWdsGetBasicCfg(QT_WDS_BASIC_CFG *wdsBasicCfg);
int QtWdsScanWifi(QT_WDS_WIFI_INFO *wds, int *len);
int QtWdsSetUp(QT_WDS_WIFI_INFO *wdsInfo);
int QtWdsSetBasicCfg(QT_WDS_BASIC_CFG *wdsBasicCfg);
int QtWdsGetStatus(QT_WDS_WIFI_NET_INFO *wdsNetInfo);

#endif
