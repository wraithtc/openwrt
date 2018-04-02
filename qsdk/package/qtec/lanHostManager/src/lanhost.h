#ifndef LANHOST_H
#define LANHOST_H
#include "stdio.h"


//this is just for debug
#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINTF(format,...) printf(format, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(format,...)
#endif

#define MIN(a, b) ((a) > (b))?(b):(a) 

/*
 * lanhost config
 */
struct lanHostEntry{
    int online;
    char ipaddr[64];
    char macaddr[64];
    char hostname[64];
    char eth_type[64];   //eth0, ath0, ath1
    int devicetype; //0pc,1android,2ios,3others
    int connection_type; //0: static  1:dhcp
    int rx;         // the speed of rx
    int tx;         // the speed of tx
	long int time;  //online,ofline time
};


struct arpEntry{
    char ipaddr[64];
    int HWType;
    int flags;    //0x02 stand for online; 0x04 stand for permannet entry
    char macaddr[64];
    char device[64]; 
};

//特殊关注
struct specialcareEntry{
    char macaddr[64];
    int flag;   //0代表上线和下线都不做提醒; 1代表上线做提醒; 2代表下线做提醒; 3代表上线和下线都做提醒
    char hostname[64];
    char devicetype[64];
};

int getArpEntryTableNum(int *output);
int getArpEntryTable(struct arpEntry *inputArray, int *arraynum);

int getLanHostEntryTableNum(int *output);
int getLanHostEntryTable(struct lanHostEntry *inputArray, int *arraynum);

int lanHostMainLogic();

void outputAllLanHostInfo(struct lanHostEntry *input, int *arraynum);


struct lanHostEntry * outputOnlineLanHostInfo( int *arraynum);

static int arpEntryTableNum=0;
static struct arpEntry arpEntryTable[128]={0};

static int lanHostEntryTableNum=0;
static struct lanHostEntry lanHostEntryTable[128]={0};

static int backupLanHostEntryTableNum =0;
static struct lanHostEntry backupLanHostEntryTable[128]={0};

static int tcpdumpLanHostEntryTableNum =0;
static struct lanHostEntry tcpdumpLanHostEntryTable[128]={0};
#endif
