#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "librtcfg.h"
#include "firewall_set.h"
#include "rtcfg_uci.h"

#define FIREWALL_REDIRECT_IP "128.1.1.1"
#define FIREWALL_REDIRECT_PORT   82

/**
 *
 * funcname: getPortForwardRuleTable
 *           get whole portForwardRule table from firewall
 * output:   one pointer point to new malloc memory to store the portForwardRule table, so this function caller need to free this memory
 */
struct portForwardRule * getPortForwardRuleTable(int *tablenum)
{
    DEBUG_PRINTF("===============getPortForwardingRuleTable=====\n");

    struct portForwardRule *output=NULL;
    int ret=0;
    int index=-1;
    int num=0;
    char cmd[256]={0};
    char tmp_store[16]={0};
    while(ret == 0)
    {
        index++;
        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@redirect[%d]",index);
        memset(tmp_store,0,16);
        ret = rtcfgUciGet(cmd,tmp_store);
    }
    
    num = index;

    if(num == 0)
    {
        output=NULL;
        *tablenum=0;
        printf("===current no portforwardRule exist====\n");
        return output;
    }

    output=malloc(num*sizeof(struct portForwardRule));

    for(index=0;index<num;index++)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@redirect[%d].name",index);
        rtcfgUciGet(cmd,output[index].name);

        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@redirect[%d].proto",index);
        rtcfgUciGet(cmd,output[index].proto);

        memset(cmd,0,256);
        memset(tmp_store,0,16);
        snprintf(cmd,256,"firewall.@redirect[%d].src_dport",index);
        rtcfgUciGet(cmd,tmp_store);
        output[index].src_dport=atoi(tmp_store);

        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@redirect[%d].dest_ip",index);
        rtcfgUciGet(cmd,output[index].dest_ip);

        memset(cmd,0,256);
        memset(tmp_store,0,16);
        snprintf(cmd,256,"firewall.@redirect[%d].dest_port",index);
        rtcfgUciGet(cmd,tmp_store);
        output[index].dest_port=atoi(tmp_store);
   
    }

    *tablenum =num;
    
    return output;
}

/**
 * funcname: AddPortForwardRule
 *          add one portForward Rule into firewall
 */
int addPortForwardRule(struct portForwardRule *input)
{
    printf("=====AddPortForwardRule=====\n");
    rtcfgUciAdd("firewall","redirect");

    rtcfgUciSet("firewall.@redirect[-1].target=DNAT");
    rtcfgUciSet("firewall.@redirect[-1].src=wan");
    rtcfgUciSet("firewall.@redirect[-1].dest=lan");
    
    char cmd[256]={0};
    snprintf(cmd,256,"firewall.@redirect[-1].name=%s",input->name);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].proto=%s",input->proto);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].src_dport=%d",input->src_dport);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].dest_ip=%s",input->dest_ip);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].dest_port=%d",input->dest_port);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("firewall");
    system("/etc/init.d/firewall reload");
    return 0;
}

/**
 * funcname: delPortForwardRule
 *           del one portForward Rule from firewall
 */
int delPortForwardRule(int index)
{
    printf("==========delPortForwardRule===========\n");
    char cmd[256]={0};
    int ret=0;
    snprintf(cmd,256,"firewall.@redirect[%d]",index);
    ret =rtcfgUciDel(cmd);

    if(ret!=0)
        return ret;
    
    rtcfgUciCommit("firewall");
    system("/etc/init.d/firewall reload");
    return 0;
}

/**
 * funcname: editPortForwardRule
 *           edie one PortForwardRule 
 */
int editPortForwardRule(struct portForwardRule *input, int index)
{
    printf("========editPortForwardRule=================\n");

    int ret1,ret2,ret3,ret4,ret5;
    char cmd[256]={0};
    snprintf(cmd,256,"firewall.@redirect[%d].name=%s",index,input->name);
    ret1=rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].proto=%s",index,input->proto);
    ret2=rtcfgUciSet(cmd);
   
    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].src_dport=%d",index,input->src_dport);
    ret3=rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].dest_ip=%s",index,input->dest_ip);
    ret4=rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].dest_port=%d",index,input->dest_port);
    ret5=rtcfgUciSet(cmd);

    if(ret1 | ret2 | ret3 | ret4 | ret5)
        return -1;
    
    rtcfgUciCommit("firewall");
    system("/etc/init.d/firewall reload");
    return 0;
}

/**
 *  func_name: setLimitLanSpeedRule
 *          set rx tx speed limit for one lan client
 */
int setLimitLanSpeedRule(struct limitLanSpeedEntry *input)
{
    char cmd[256]={0};

    if(input->rx !=0)
    {
        snprintf(cmd,256,"iptables -A FORWARD -m limit -d %s --limit %d/s --limit-burst 100 -j ACCEPT",input->ipaddr,input->rx);
        system(cmd);
    
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -A FORWARD -d %s -j DROP",input->ipaddr);
        system(cmd);
    }

    if(input->tx !=0)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -A FORWARD -m limit -s %s --limit %d/s --limit-burst 100 -j ACCEPT",input->ipaddr, input->tx);
        system(cmd);
    
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -A FORWARD -s %s -j DROP", input->ipaddr);
        system(cmd);
    }


    return 0;
}

/**
 *  func_name: declineLimitLanSpeedRule
 *              decline rx tx speed limit for one lan client
 */
int declineLimitLanSpeedRule(struct limitLanSpeedEntry *input)
{
    char cmd[256]={0};
    
    if(input->rx !=0)
    {
        snprintf(cmd,256,"iptables -D FORWARD -m limit -d %s --limit %d/s --limit-burst 100 -j ACCEPT", input->ipaddr, input->rx);
        system(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -D FORWARD -d %s -j DROP", input->ipaddr);
        system(cmd);
    }

    if(input->tx !=0)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -D FORWARD -m limit -s %s --limit %d/s --limit-burst 100 -j ACCEPT", input->ipaddr, input->tx);
        system(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"iptables -D FORWARD -s %s -j DROP", input->ipaddr);
        system(cmd);
    }

    return 0;
}

/**
 *  func_name: setBlockLan
 *          block one lan client 
 */
int setBlockLan(char* inputMac)
{
    char cmd[256]={0};
    snprintf(cmd,256,"ebtables -A FORWARD -s %s -j DROP",inputMac);
    system(cmd);

    return 0;
}

/**
 *  func_name: declineBlockLan
 *      decline the rule that block one lan client
 */
int declineBlockLan(char* inputMac)
{
    char cmd[256]={0};
    snprintf(cmd,256,"ebtables -D FORWARD -s %s -j DROP", inputMac);
    system(cmd);
    return 0;
}


/*********************************************************************
 *  childRuleEntry
 *
 * time match options:
 *     --datestart time     Start and stop time, to be given in ISO 8601
 *     --datestop time      (YYYY[-MM[-DD[Thh[:mm[:ss]]]]])
 *     --timestart time     Start and stop daytime (hh:mm[:ss])
 *     --timestop time      (between 00:00:00 and 23:59:59)
 * [!] --monthdays value    List of days on which to match, separated by comma
 *                          (Possible days: 1 to 31; defaults to all)
 * [!] --weekdays value     List of weekdays on which to match, sep. by comma
 *                          (Possible days: Mon,Tue,Wed,Thu,Fri,Sat,Sun or 1 to 7
 *                          Defaults to all weekdays.)
 *     --kerneltz           Work with the kernel timezone instead of UTC
 * 
 * 
 * mac match options:
 * [!] --mac-source XX:XX:XX:XX:XX:XX
 *                                 Match source MAC address
 * 
 * iptables -I FORWARD -m mac --mac-source 00:0E:C6:D2:32:74 -m time --timestart 09:00 --timestop 12:00 --weekdays Mon,Wed --monthdays 1,4,5 -j DROP
 *
 *  to do:
 *  add into one info mangle mechanism to store child rule 
 *
 * *********************************************************************
 */
int addChildRule(struct childRuleEntry *input)
{
    if( (strlen(input->mac)==0) || (strlen(input->timestart)==0) || (strlen(input->timestop)==0))
    {
        printf("===ERROR!!! === %s === input parameter is wrong===\n",__func__);
        return -1;
    }
    
    char cmd[1024]={0};
    snprintf(cmd,1024,"iptables -I FORWARD -m mac --mac-source %s -m time --timestart %s --timestop %s", input->mac, input->timestart, input->timestop);
    
    if(strlen(input->weekdays)!=0)
    {
        strcat(cmd," --weekdays ");
        strcat(cmd,input->weekdays);
    }

    if(strlen(input->monthdays)!=0)
    {
        strcat(cmd," --monthdays ");
        strcat(cmd, input->monthdays );
    }
    strcat(cmd," -j DROP");

    DEBUG_PRINTF("==%s===cmd: %s ====\n",__func__,cmd);
    int ret=0;
    ret=system(cmd);
    

    if(ret !=0)
    {
        printf("====ERROR!!! === %s === cmd exec fail===\n",__func__);
    }

    return 0;
}

int delChildRule(struct childRuleEntry *input)
{
    if( (strlen(input->mac)==0) || (strlen(input->timestart)==0) || (strlen(input->timestop)==0))
    {
        printf("===ERROR!!! === %s === input parameter is wrong===\n",__func__);
        return -1;
    }
    
    char cmd[1024]={0};
    snprintf(cmd,1024,"iptables -D FORWARD -m mac --mac-source %s -m time --timestart %s --timestop %s", input->mac, input->timestart, input->timestop);
    
    if(strlen(input->weekdays)!=0)
    {
        strcat(cmd," --weekdays ");
        strcat(cmd,input->weekdays);
    }

    if(strlen(input->monthdays)!=0)
    {
        strcat(cmd," --monthdays ");
        strcat(cmd, input->monthdays );
    }
    strcat(cmd," -j DROP");

    DEBUG_PRINTF("==%s===cmd: %s ====\n",__func__,cmd);
    int ret=0;
    ret=system(cmd);
    

    if(ret !=0)
    {
        printf("====ERROR!!! === %s === cmd exec fail===\n",__func__);
    }

    return 0;
}

int editChildRule(struct childRuleEntry *oldInput, struct childRuleEntry *newInput)
{
    delChildRule(oldInput);
    addChildRule(newInput);
    return 0;
}

/***********************************************************************************
 *   URL BLOCK
 *   
 *   in FORWARD chain filter block url's dns packet 
 *   iptables -I FORWARD -m string --hex-string "www|05|baidu|03|com" --algo bm -j DROP
 */

/**
 *  func_name: praseUrlToString
 *  input:
 *          inputUrl: char pointer point to domain url string, for example www.baidu.com
 *          urlString: char pointer point to memory has been malloced for store the prase result, for example |03|www|05|baidu|03|com 
 */
int praseUrlToString(char *inputUrl, char *urlString)
{
    DEBUG_PRINTF("====%s====inputUrl: %s====\n",__func__,inputUrl);
    char count[10]={0};
    char *tmp1=malloc(sizeof(char)*256);
    memset(tmp1,0,256);
    strcpy(tmp1,inputUrl);

    char *tmp2=tmp1;
    
    char delim[]=".";
    char *token=NULL;
    for(token=strsep(&tmp2,delim); token!=NULL; token=strsep(&tmp2,delim))
    {
        if(strlen(token)==0)
        {
            printf("===ERROR!!!, %s: inputUrl format wrong ====\n", __func__);
            return -1;
        }
        memset(count,0,10);
        snprintf(count,10,"|%02x|",strlen(token));
        strcat(urlString,count);
        strcat(urlString,token);

    }
    free(tmp1);
    DEBUG_PRINTF("====%s===urlString: %s ====\n",__func__,urlString);
    return 0;
}

int addUrlBlockRule(char *inputUrl)
{
    //step one: prase inputUrl into string in dns packet
    char urlString[256]={0};
    praseUrlToString(inputUrl,urlString);
   
    if(strlen(urlString) != 0)
    {
        char cmd[256]={0};
        snprintf(cmd,256,"iptables -I FORWARD -m string --hex-string \"%s\" --algo bm -j DROP",urlString);
        system(cmd);
    }
    else 
    {
        printf("===ERROR! %s === fail ===\n",__func__);
    }

    return 0;
}

int delUrlBlockRule(char *inputUrl)
{
    char urlString[256]={0};
    praseUrlToString(inputUrl,urlString);

    if(strlen(urlString) != 0 )
    {   
        char cmd[256]={0};
        snprintf(cmd,256,"iptables -D FORWARD -m string --hex-string \"%s\" --algo bm -j DROP", urlString);
        system(cmd);
    }
    else
    {
        printf("====ERROR! %s ==== fail===\n",__func__);
    }

    return 0;
}

int QtSetUrlFirewall(int enable)
{
    FILE *fp;
    char cmd[128] = {0};
    char iptable_cmd[128] = {0};
    char lanIp[32] = {0};

    rtcfgUciGet("network.lan.ipaddr",lanIp);
    if (enable)
    {
        snprintf(cmd , sizeof(cmd), "echo 'conf-file=/etc/dnsmasq_hosts.conf' >> %s", DNSMASQ_CONF_FILE);
        system(cmd); 

        memset(cmd,0,sizeof(cmd));
        snprintf(iptable_cmd, sizeof(iptable_cmd), "iptables -t nat -I PREROUTING -p tcp -d %s -j REDIRECT --to-ports %d", FIREWALL_REDIRECT_IP, FIREWALL_REDIRECT_PORT);
        snprintf(cmd, sizeof(cmd), "sed -i '/%s/d' /etc/firewall.user", iptable_cmd);
        system(cmd);
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "echo '\n%s' >> /etc/firewall.user", iptable_cmd);
        system(cmd);
        
        memset(cmd,0,sizeof(cmd));
        memset(iptable_cmd,0,sizeof(iptable_cmd));
        snprintf(iptable_cmd, sizeof(iptable_cmd), "iptables -t nat -I PREROUTING 2 -p tcp -d %s -j DNAT --to-destination %s", FIREWALL_REDIRECT_IP, lanIp);
        snprintf(cmd, sizeof(cmd), "sed -i '/%s/d' /etc/firewall.user", iptable_cmd);
        system(cmd);
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "echo '\n%s' >> /etc/firewall.user", iptable_cmd);
        system(cmd);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "sed -i '/conf-file=\\/etc\\/dnsmasq_hosts.conf/d' %s", DNSMASQ_CONF_FILE);
        system(cmd);
        system("sed -i '/iptables -I FORWARD -i ath01 -d/d' /etc/firewall.user");
        memset(cmd,0,sizeof(cmd));
        snprintf(iptable_cmd, sizeof(iptable_cmd), "iptables -t nat -I PREROUTING -p tcp -d %s -j REDIRECT --to-ports %d", FIREWALL_REDIRECT_IP, FIREWALL_REDIRECT_PORT);
        snprintf(cmd, sizeof(cmd), "sed -i '/%s/d' /etc/firewall.user", iptable_cmd);
        system(cmd);
        memset(cmd,0,sizeof(cmd));
        memset(iptable_cmd,0,sizeof(iptable_cmd));
        snprintf(iptable_cmd, sizeof(iptable_cmd), "iptables -t nat -I PREROUTING 2 -p tcp -d %s -j DNAT --to-destination %s", FIREWALL_REDIRECT_IP, lanIp);
        snprintf(cmd, sizeof(cmd), "sed -i '/%s/d' /etc/firewall.user", iptable_cmd);
        system(cmd);
    }
}

int QtSetFirewallStatus(QT_FIREWALL_CFG_T *firewallCfg)
{
    char cmd[128]={0};
    char value[8] = {0};
    int oldValue = 0;

    rtcfgUciSet("firewall.global_sw=sw");

    rtcfgUciGet("firewall.global_sw.url_firewall", value);
    oldValue = (strlen(value) == 0)?0:atoi(value);
    if (oldValue != firewallCfg->url)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "firewall.global_sw.url_firewall=%d", firewallCfg->url);
        rtcfgUciSet(cmd);
        QtSetUrlFirewall(firewallCfg->url);
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "firewall.global_sw.family_firewall=%d", firewallCfg->family);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "firewall.global_sw.hijack_firewall=%d", firewallCfg->hijack);
    rtcfgUciSet(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "firewall.global_sw.pwd_firewall=%d", firewallCfg->pwd);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("firewall");

    return 0;
}

int QtGetFirewallStatus(QT_FIREWALL_CFG_T *firewallCfg)
{
    char cmd[128]={0};
    char value[16] = {0};

    snprintf(cmd, sizeof(cmd), "firewall.global_sw.url_firewall");
    rtcfgUciGet(cmd, value);
    firewallCfg->url = (strlen(value) == 0)?0:atoi(value);


    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "firewall.global_sw.family_firewall");
    memset(value, 0, sizeof(value));
    rtcfgUciGet(cmd, value);
    firewallCfg->family = (strlen(value) == 0)?0:atoi(value);
    
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "firewall.global_sw.hijack_firewall");
    memset(value, 0, sizeof(value));
    rtcfgUciGet(cmd, value);
    firewallCfg->hijack = (strlen(value) == 0)?0:atoi(value);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "firewall.global_sw.pwd_firewall");
    memset(value, 0, sizeof(value));
    rtcfgUciGet(cmd, value);
    firewallCfg->pwd = (strlen(value) == 0)?0:atoi(value);


    return 0;
}

