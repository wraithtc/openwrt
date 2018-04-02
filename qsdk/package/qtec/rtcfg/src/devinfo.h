#ifndef DEVINFO_H
#define DEVINFO_H

struct devInfo{
    char name[64];
    char ver[64];
    char model[64]; 
    int id;
    int type;     
};

int searchDevList(struct devInfo *output);
int addBandDev(int devid);
int delBandDev(int devid);
int getBandDev(struct devInfo *output);
#endif
