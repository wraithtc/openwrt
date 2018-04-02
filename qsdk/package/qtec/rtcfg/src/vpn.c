#include "librtcfg.h"
#include "vpn.h"

//static struct uci_context * ctx = NULL;


static void QtAddFireWallRule()
{
    char cmd[128] = {0};
    printf("=====Add vpn firewall rule=====\n");

    rtcfgUciDel("firewall.vpnrule_esp");
    rtcfgUciDel("firewall.vpnrule_udp");
    rtcfgUciCommit("firewall");

    rtcfgUciSet("firewall.vpnrule_esp=rule");
    rtcfgUciSet("firewall.vpnrule_esp.src=wan");
    rtcfgUciSet("firewall.vpnrule_esp.dest=lan");
    rtcfgUciSet("firewall.vpnrule_esp.proto=esp");
    rtcfgUciSet("firewall.vpnrule_esp.target=ACCEPT");

    rtcfgUciSet("firewall.vpnrule_udp=rule");
    rtcfgUciSet("firewall.vpnrule_udp.src=wan");
    rtcfgUciSet("firewall.vpnrule_udp.dest=lan");
    rtcfgUciSet("firewall.vpnrule_udp.proto=udp");
    rtcfgUciSet("firewall.vpnrule_udp.dest_port=500");
    rtcfgUciSet("firewall.vpnrule_udp.target=ACCEPT");

    rtcfgUciCommit("firewall");
}

unsigned int QtGetVpnId()
{
    char cmd[128] = {0};
    char value[64] = {0};

    
    snprintf(cmd, sizeof(cmd), "vpnid.taskid.id");
    rtcfgUciGet(cmd, value);

    if (strlen(value) == 0 || atoi(value) == 0)
    {
        return 0;
    }
    else
    {
        return atoi(value);
    }
}

int QtSetVpnId(unsigned int id)
{
    char cmd[128] = {0};
    char value[64] = {0};
    if (access("/etc/config/vpnid", F_OK) != 0)
    {
        system("touch /etc/config/vpnid");
    }
    rtcfgUciSet("vpnid.taskid=idcount");
    snprintf(cmd, sizeof(cmd), "vpnid.taskid.id=%d", id);
    rtcfgUciSet(cmd);
    rtcfgUciCommit("vpnid");
}


int QtAddVpn(QT_VPN_CFG *vpnCfg)
{
    char cmd[128] = {0};
    unsigned int i;
    char ifname[32] = {0};
    FILE *fp;
    printf("=====AddVpn=====\n");

    i = QtGetVpnId();
    i++;
    QtSetVpnId(i);
    printf("ifname index:%d\n", i);

    snprintf(ifname, sizeof(ifname), "vpn_%d", i);

    snprintf(cmd, sizeof(cmd), "network.%s=vpn", ifname);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.idname=%s", ifname, ifname);
    rtcfgUciSet(cmd);
    
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.description=%s", ifname, vpnCfg->description);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.proto=%s", ifname, vpnCfg->proto);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.server=%s", ifname, vpnCfg->serverip);
    rtcfgUciSet(cmd);
    
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.username=%s", ifname, vpnCfg->username);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.password=%s", ifname, vpnCfg->password);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.type=vpn", ifname);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("network");

    QtAddFireWallRule();

    return 0;

}


int QtEditVpn(QT_VPN_CFG *vpnCfg)
{
    char cmd[128] = {0};
    printf("=====edit vpn=====\n");
    snprintf(cmd, sizeof(cmd), "network.%s.proto=%s", vpnCfg->ifname, vpnCfg->proto);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.server=%s", vpnCfg->ifname, vpnCfg->serverip);
    rtcfgUciSet(cmd);
    
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.username=%s", vpnCfg->ifname, vpnCfg->username);
    rtcfgUciSet(cmd);

    if (strlen(vpnCfg->password) > 0)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "network.%s.password=%s", vpnCfg->ifname, vpnCfg->password);
        rtcfgUciSet(cmd);
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.description=%s", vpnCfg->ifname, vpnCfg->description);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("network");

}

int QtDelVpn(const char *ifName)
{
    char cmd[128] = {0};
    char type[32] = {0};
    int enable;
    printf("=====del vpn=====\n");
    if (!strncmp(ifName, "wan", 3) || !strncmp(ifName, "wan6", 3) || !strncmp(ifName, "lan", 3))
    {
        printf("Can't delete system inferface, router may be hacked!\n");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "network.%s", ifName);
    rtcfgUciGet(cmd, type);
    enable = !strncmp(type, "interface", 9);
    rtcfgUciDel(cmd);

    rtcfgUciCommit("network");
    
    if (enable)
    {
    
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "ubus call network.interface.%s prepare", ifName);
        system(cmd);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "ifdown %s", ifName);
        system(cmd);
        #if 0
        usleep(100);
        system("ubus call network.interface.wan prepare");
        system("ifup wan");
    
        #endif
    }
    
}

void getVpn(QT_VPN_CFG *vpnCfg, int enable, int pos)
{
    char cmd[128] = {0};
    char linkstatus[8] = {0};
    char value[128] = {0};
    FILE *fp = NULL;

    memset(value, 0, sizeof(value));
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@%s[%d].idname", enable?"interface":"vpn", pos);
    rtcfgUciGet(cmd, value);
    strncpy(vpnCfg->ifname, value, sizeof(vpnCfg->ifname));
    
    memset(value, 0, sizeof(value));
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@%s[%d].description", enable?"interface":"vpn", pos);
    rtcfgUciGet(cmd, value);
    strncpy(vpnCfg->description, value, sizeof(vpnCfg->description));

    memset(value, 0, sizeof(value));
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@%s[%d].proto", enable?"interface":"vpn", pos);
    rtcfgUciGet(cmd, value);
    strncpy(vpnCfg->proto, value, sizeof(vpnCfg->proto));

    memset(value, 0, sizeof(value));
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@%s[%d].server", enable?"interface":"vpn", pos);
    rtcfgUciGet(cmd, value);
    strncpy(vpnCfg->serverip, value, sizeof(vpnCfg->serverip));

    memset(value, 0, sizeof(value));
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@%s[%d].username", enable?"interface":"vpn", pos);
    rtcfgUciGet(cmd, value);
    strncpy(vpnCfg->username, value, sizeof(vpnCfg->username));

    memset(value, 0, sizeof(value));
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@%s[%d].password", enable?"interface":"vpn", pos);
    rtcfgUciGet(cmd, value);
    strncpy(vpnCfg->password, value, sizeof(vpnCfg->password));

    memset(value, 0, sizeof(value));
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@%s[%d].enable", enable?"interface":"vpn", pos);
    rtcfgUciGet(cmd, value);
    vpnCfg->enable = atoi(value);

    if (enable && vpnCfg->enable)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "ubus call network.interface.%s status > /tmp/vpnstatus", vpnCfg->ifname);
        system(cmd);
        system("grep '\"up\"' /tmp/vpnstatus | sed -e 's/^.*: \\(.*\\),/\\1/g' > /tmp/vpnlinkstatus");
        fp = fopen("/tmp/vpnlinkstatus", "r");
        if (fp != NULL)
        {
            fscanf(fp, "%s", linkstatus);
            if (!strncmp(linkstatus, "true", 4))
            {
                vpnCfg->status = 1;
            }
            else
            {
                vpnCfg->status = 0;
            }
            fclose(fp);
        }
        else
        {
            vpnCfg->status = 0;
        }
    }
    else
    {
        vpnCfg->status = 0;
    }
}

int QtGetVpn(QT_VPN_CFG *vpnCfgArray, int *len)
{ 
    char *ip;
    int i = 0, j=0 ,k = 0;
    char cmd[128] = {0};
    char linkstatus[32] = {0};
    char ifname[32] = {0};
    char value[64] = {0};
    FILE *fp = NULL;
    
    printf("=====get vpn=====111\n");
    snprintf(cmd, sizeof(cmd), "network.@interface[%d]", i);
    while (rtcfgUciGet(cmd, ifname) == 0)
    {
        memset(value, 0, sizeof(value));
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "network.@interface[%d].type", i);
        rtcfgUciGet(cmd, value);
        if (strncmp(value, "vpn", 3) == 0)
        {
            getVpn(&vpnCfgArray[j], 1, i);
            j++;
        }
        i++;
        

        if (j >= *len)
            goto cleanup;
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "network.@interface[%d]", i);
    }

    i = 0;
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.@vpn[%d]", i);
    while (rtcfgUciGet(cmd, ifname) == 0)
    {
        getVpn(&vpnCfgArray[j], 0, i); 
        i++;
        j++;
        if (j >= *len)
            goto cleanup;
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "network.@vpn[%d]", i);
    }
    
cleanup:  
    *len = j;
    for (i = 0; i < j - 1; i++)
    {
        for (k = i+1; k < j; k++)
        {
            QT_VPN_CFG tmpCfg = {0};
            int pos1 = 0, pos2 = 0;
            pos1 = atoi(&vpnCfgArray[i].ifname[4]);
            pos2 = atoi(&vpnCfgArray[k].ifname[4]);

            if (pos1 > pos2)
            {
                memcpy(&tmpCfg, &vpnCfgArray[i], sizeof(tmpCfg));
                memcpy(&vpnCfgArray[i], &vpnCfgArray[k], sizeof(vpnCfgArray[i]));
                memcpy(&vpnCfgArray[k], &tmpCfg, sizeof(vpnCfgArray[k]));
            }
        }
    }
    return 0;
}

int QtSetVpnSw(int enable)
{
    char cmd[128] = {0};
    int i=0, ret = 0;
    char value[64] = {0};
    
    rtcfgUciSet("network.vpnsw=sw");

    snprintf(cmd,sizeof(cmd),"network.vpnsw.enable=%d",enable);
    rtcfgUciSet(cmd);

    
    if (!enable)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"network.@interface[%d]",i);
        while ((ret = rtcfgUciGet(cmd, value)) == 0)
        {
            memset(value, 0, sizeof(value));
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "network.@interface[%d].type", i);
            rtcfgUciGet(cmd, value);
            if (strncmp(value, "vpn", 3) == 0)
            {
                memset(value, 0, sizeof(value));
                memset(cmd, 0, sizeof(cmd));
                snprintf(cmd, sizeof(cmd), "network.@interface[%d].enalbe_for_real=1", i);
                rtcfgUciSet(cmd);

                memset(cmd, 0, sizeof(cmd));
                snprintf(cmd, sizeof(cmd), "network.@interface[%d]=vpn", i);
                rtcfgUciSet(cmd);
            }
            i++;
            memset(value, 0, sizeof(value));
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.@interface[%d]",i);
        }
    }
    else
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"network.@vpn[%d]",i);
        while ((ret = rtcfgUciGet(cmd, value)) == 0)
        {
            memset(value, 0, sizeof(value));
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "network.@vpn[%d].enalbe_for_real", i);
            rtcfgUciGet(cmd, value);
            if (1 == atoi(value))
            {
                memset(cmd, 0, sizeof(cmd));
                snprintf(cmd, sizeof(cmd), "network.@vpn[%d]=interface", i);
                rtcfgUciSet(cmd);
                break;
            }
            i++;
            memset(value, 0, sizeof(value));
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"network.@vpn[%d]",i);
        }
    }
    rtcfgUciCommit("network");

    return 0;
}

int QtGetVpnSw(int *enable)
{
    char cmd[128] = {0};
    char enableStr[8] = {0};

    rtcfgUciGet("network.vpnsw.enable", enableStr);

    if (strlen(enableStr) == 0)
    {
        *enable = 0;
    }
    else
    {
        *enable = atoi(enableStr);
    }

    return 0;
}


int QtSetVpnIfSw(char *ifname, int enable, int vpnenable)
{
    char cmd[128] = {0};
    int len = 8, i = 0;

    if (enable && vpnenable)
    {
        
        snprintf(cmd, sizeof(cmd), "network.%s=interface", ifname);
        rtcfgUciSet(cmd);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "network.%s=vpn", ifname);
        rtcfgUciSet(cmd);
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "network.%s.enable=%d", ifname, enable);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("network");

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "ubus call network.interface.%s prepare", ifname);
    system(cmd);
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "ifdown %s", ifname);
    system(cmd);

    return 0;
}

