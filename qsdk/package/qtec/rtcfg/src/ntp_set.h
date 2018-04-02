#ifndef NTPSET_H
#define NTPSET_H
#include "stdio.h"

/*
 *ntpconfig
 *
 */
struct ntpConfig{
    int enable;
    char timezone[64];
    char ntpServers[1024]; // forexample: 1.1.1.1 2.2.2.2 
};

int ntpConfigSet(struct ntpConfig *input);
int ntpConfigGet(struct ntpConfig *output);
#endif
