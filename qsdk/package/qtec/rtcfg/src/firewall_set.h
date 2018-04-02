#ifndef FIREWALLSET_H
#define FIREWALLSET_H

#define DIRTY_URL_HOSTS_FILE    "/etc/qtec_hosts"
#define DNSMASQ_CONF_FILE    "/etc/dnsmasq.conf"

/**
 *  portForwardRule
 *
 */
struct portForwardRule{
    char name[64];
    char proto[64];
    int src_dport;
    char dest_ip[64];
    int dest_port;
};


struct limitLanSpeedEntry{
    char ipaddr[64];
    int rx; 
    int tx;
};

struct childRuleEntry{
    char mac[64];
    char timestart[64];
    char timestop[64];
    char weekdays[128];
    char monthdays[256];
};

typedef struct {
    int url;
    int family;
    int hijack;
    int pwd;
}QT_FIREWALL_CFG_T;

int setLimitLanSpeedRule(struct limitLanSpeedEntry *input);
int declineLimitLanSpeedRule(struct limitLanSpeedEntry *input);

int setBlockLan(char *inputMac);
int declineBlockLan(char *inputMac);

struct portForwardRule * getPortForwardRuleTable(int *num);
int addPortForwardRule(struct portForwardRule *input);
int delPortForwardRule(int index);
int editPortForwardRule(struct portForwardRule *input, int index);

/*
 * childRuleEntry functions
 */
int addChildRule(struct childRuleEntry *input);
int delChildRule(struct childRuleEntry *input);
int editChildRule(struct childRuleEntry *oldinput,struct childRuleEntry *newinput);

/**
 * URL BLOCK
 */
int addUrlBlockRule(char *input_url);
int delUrlBlockRule(char *input_url);

int QtSetFirewallStatus(QT_FIREWALL_CFG_T *firewallCfg);
int QtSetFirewallStatus(QT_FIREWALL_CFG_T *firewallCfg);

#endif

