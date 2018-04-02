#ifndef RTCFG_UCI_H
#define RTCFG_UCI_H

#include <stdio.h>
#include <stdlib.h>
#include <uci.h>



int rtcfgUciGet(const char *cmd, char *value);
int rtcfgUciSet(const char *cmd);
int rtcfgUciAddList(const char *cmd);
int rtcfgUciDelList(const char *cmd);
int rtcfgUciAdd(const char *config_name, const char *section_name);
int rtcfgUciDel(const char *cmd);
int rtcfgUciCommit(const char *cmd);



#endif
