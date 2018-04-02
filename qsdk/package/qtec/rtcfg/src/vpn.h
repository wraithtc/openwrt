#ifndef __VPN_H__
#define __VPN_H__
#include<uci.h>

#define VPN_IFNAME_FILE    "/etc/vpn_if"

typedef struct{
    char description[128];
    char ifname[32];
    char  proto[8];
    char username[64];
    char password[64];
    char serverip[64];
    int enable;
    int status;
}QT_VPN_CFG;

int QtEditVpn(QT_VPN_CFG *vpnCfg);

int QtAddVpn(QT_VPN_CFG *vpnCfg);

int QtDelVpn(const char *ifName);

int QtGetVpn(QT_VPN_CFG *vpnCfgArray, int *len);

int QtSetVpnSw(int enable);

int QtSetVpnIfSw(char *ifname, int enable, int vpnenable);

#endif /* __VPN_H__ */
