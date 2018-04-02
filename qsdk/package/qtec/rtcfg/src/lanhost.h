#ifndef LANHOST_H
#define LANHOST_H
#include "stdio.h"

/*
 * lanhost config
 */
struct lanHostEntry{
    int online;
    char ipaddr[64];
    char macaddr[64];
    char hostname[64];
    int connection_type; //0: static  1:dhcp
    int eth_type;   //0:ethernet 2.wifi
    int rx;         // the speed of rx
    int tx;         // the speed of tx
};


struct arpEntry{
    char ipaddr[64];
    int HWType;
    int flags;    //0x02 stand for online; 0x04 stand for permannet entry
    char macaddr[64];
    char device[64]; 
};

int getArpEntryTableNum(int *output);
int getArpEntryTable(struct arpEntry *inputArray, int *arraynum);

int getLanHostEntryTableNum(int *output);
int getLanHostEntryTable(struct lanHostEntry *inputArray, int *arraynum);

int lanHostMainLogic();

struct lanHostEntry * outputAllLanHostInfo( int *arraynum);

struct lanHostEntry * outputOnlineLanHostInfo( int *arraynum);

static int arpEntryTableNum=0;
static struct arpEntry * arpEntryTable=NULL;

static int lanHostEntryTableNum=0;
static struct lanHostEntry * lanHostEntryTable=NULL;

#endif
