#ifndef __QTEC_EBTABLES_BASIC_H
#define __QTEC_EBTABLES_BASIC_H

#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <ctype.h>
#include <string.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>

#include <time.h>

#include <uci.h>

#include <libubox/list.h>
#include <libubox/utils.h>
#include <libubox/blobmsg.h>
#include <librtcfg.h>
#include "ebtables_struct.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//调试用的函数
#undef DEBUG
#define DEBUG 1
#ifdef DEBUG 

#undef DEBUG_PRINTF

#define DEBUG_PRINTF(format,...)   printf(format, ##__VA_ARGS__); fflush(stdout);
#else
#define DEBUG_PRINTF(format,...)
#endif



#define QTEC_EBTABLES_LOCKFILE	"/var/run/qtec_ebtables.lock"
extern bool qtec_ebtables_pr_debug;


//全局变量
struct list_head global_ebtables_speedlimit_rule;




#endif
