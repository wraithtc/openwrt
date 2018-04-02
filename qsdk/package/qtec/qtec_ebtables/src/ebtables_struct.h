#ifndef __EBTABLES_STRUCT_H
#define __EBTABLES_STRUCT_H


#include <libubox/list.h>
#include <libubox/utils.h>
#include <libubox/blobmsg.h>

struct ebtables_speedlimit_rule
{
	struct list_head list;

	bool enabled; 
	char name[64];  

    
    char mac[64];

	int dest;  //0:INPUT; 1:OUTPUT

    int limit;
    char dest_ip[32];
    char src_if[16];
    char dst_if[16];

    char target[8];
};

#endif
