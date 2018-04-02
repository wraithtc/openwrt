#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "fwk.h"


/* Get the ipv6 address of the interface. */
VOS_RET_E oal_getIfAddr6(const char *ifname, UINT32 addrIdx,
                      char *ipAddr, UINT32 *ifIndex, UINT32 *prefixLen, UINT32 *scope, UINT32 *ifaFlags)
{
    VOS_RET_E ret = VOS_RET_NO_MORE_INSTANCES;
    FILE     *fp;
    SINT32   count = 0;
    char     line[BUFLEN_64];

    *ipAddr = '\0';

    if (NULL == (fp = fopen("/proc/net/if_inet6", "r")))
    {
        vosLog_error("failed to open /proc/net/if_inet6");
        return VOS_RET_INTERNAL_ERROR;
    }

    while (NULL != fgets(line, sizeof(line), fp))
    {
        /* remove the carriage return char */
        line[strlen(line)-1] = '\0';
        
        if (strstr(line, ifname) != NULL)
        {
            char *addr = NULL, *ifidx = NULL;
            char *plen = NULL, *scp = NULL;
            char *flags = NULL, *devname = NULL; 
            char *nextToken = NULL;

            /* the first token in the line is the ip address */
            addr = strtok_r(line, " ", &nextToken);

            /* the second token is the Netlink device number (interface index) in hexadecimal */
            ifidx = strtok_r(NULL, " ", &nextToken);
            if (NULL == ifidx)
            {
                vosLog_error("Invalid /proc/net/if_inet6 line");
                ret = VOS_RET_INTERNAL_ERROR;
                break;
            }

            /* the third token is the Prefix length in hexadecimal */
            plen = strtok_r(NULL, " ", &nextToken);
            if (NULL == plen)
            {
                vosLog_error("Invalid /proc/net/if_inet6 line");
                ret = VOS_RET_INTERNAL_ERROR;
                break;
            }

            /* the forth token is the Scope value */
            scp = strtok_r(NULL, " ", &nextToken);
            if (NULL == scp)
            {
                vosLog_error("Invalid /proc/net/if_inet6 line");
                ret = VOS_RET_INTERNAL_ERROR;
                break;
            }

            /* the fifth token is the ifa flags */
            flags = strtok_r(NULL, " ", &nextToken);
            if (NULL == flags)
            {
                vosLog_error("Invalid /proc/net/if_inet6 line");
                ret = VOS_RET_INTERNAL_ERROR;
                break;
            }

            /* the sixth token is the device name */
            devname = strtok_r(NULL, " ", &nextToken);
            if (NULL == devname)
            {
                vosLog_error("Invalid /proc/net/if_inet6 line");
                ret = VOS_RET_INTERNAL_ERROR;
                break;
            }
            else
            {
                if (0 != strcmp(devname, ifname))
                {
                    continue;
                }
                else if (count == addrIdx)
                {
                    SINT32   i = 0;
                    char     *p1 = NULL; 
                    char     *p2 = NULL;

                    *ifIndex   = strtoul(ifidx, NULL, 16);
                    *prefixLen = strtoul(plen, NULL, 16);
                    *scope     = strtoul(scp, NULL, 16);
                    *ifaFlags  = strtoul(flags, NULL, 16);

                    /* insert a colon every 4 digits in the address string */
                    p2 = ipAddr;
                    for (i = 0, p1 = addr; *p1 != '\0'; i++)
                    {
                        if (i == 4)
                        {
                            i = 0;
                            *p2++ = ':';
                        }
                        
                        *p2++ = *p1++;
                    }
                    
                    *p2 = '\0';

                    ret = VOS_RET_SUCCESS;
                    break;   /* done */
                }
                else
                {
                    count++;
                }
            }
        }
   }  /* while */

   fclose(fp);

   return ret;
}  /* End of oal_getIfAddr6() */


