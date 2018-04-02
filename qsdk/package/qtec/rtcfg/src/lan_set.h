#ifndef LANSET_H
#define LANSET_H


/*
 *lanConfig
 *
 */
struct lanConfig{
    char ipaddress[64];
    char netmask[64];
    char dhcpPoolStart[64];
    char dhcpPoolLimit[64];
    int dhcpEnable;
};

/*
 *struct staticLease: pre-static-lease for lan clients
 *
 */
struct staticLeaseConfig{
    char hostname[64];
    char mac[256];
    char ipaddress[64];
};

int lanConfigSet(struct lanConfig *input);
int lanConfigGet(struct lanConfig *output);
struct staticLeaseConfig * getStaticLeaseArray(int* array_num);
int addStaticLeaseEntry(struct staticLeaseConfig *input);
int delStaticLeaseEntry(int index);

#endif
