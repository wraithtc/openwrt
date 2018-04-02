/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 *
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

*/

/* C and system library includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <fcntl.h>
#include <wifison_event.h>

int main(int argc, char **argv)
{
    int socket = 0, error = 0;
    struct sonEventInfo info;

    socket = wifison_event_init();

    wifison_event_register(RE_JOIN_EVENT);
    wifison_event_register(RE_LEAVE_EVENT);

    while (1)
    {
        error = wifison_event_get(&info);

        if (error == EVENT_SOCKET_ERROR)
            goto err;

        if (error == EVENT_OK)
        {
            switch (info.eventMsg)
            {
                case CLIENT_START:
                    printf("HYD restart, td database information is dispeer (RE maybe leave and wait for new Database update)\r\n");
                    break;
                case RE_JOIN_EVENT:
                    printf("RE MAC %x:%x:%x:%x:%x:%x is Join as %s\r\n",info.data.re.macaddress[0],info.data.re.macaddress[1],info.data.re.macaddress[2],
                               info.data.re.macaddress[3],info.data.re.macaddress[4],info.data.re.macaddress[5],info.data.re.isDistantNeighbor?"Distant Neighbor":"Direct Neighbor");
                    break;
                case RE_LEAVE_EVENT:
                    printf("RE MAC %x:%x:%x:%x:%x:%x is leave\r\n",info.data.re.macaddress[0],info.data.re.macaddress[1],info.data.re.macaddress[2],
                               info.data.re.macaddress[3],info.data.re.macaddress[4],info.data.re.macaddress[5]);
                    break;
            }
        }
    }

err:
    wifison_event_deregister(RE_JOIN_EVENT);
    wifison_event_deregister(RE_LEAVE_EVENT);
    wifison_event_deinit();

    return 0;
}
