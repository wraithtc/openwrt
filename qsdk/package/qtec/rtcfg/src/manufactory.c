#include "manufactory.h"
#include <errno.h>
#include <unistd.h> 

#define  _O_RDONLY    0x0000

int QtGetFactoryMode(UINT8 *isFactoryMode)
{
    int fd;
    char readBuf[BUFLEN_128] = {0};
    int ret, i;

    fd = open("/dev/mtdblock8", _O_RDONLY);

    if (fd <= 0)
    {
        printf("Fail to read mtd8, errno = %d\n", errno);
        return -errno;
    }

    ret = read(fd, readBuf, sizeof(readBuf));
    if (ret <= 0)
    {
        printf("read from mtd8 failed, errno = %d,\n", errno);
        close(fd);
        return -errno;
    }

    for (i = 0 ; i < sizeof(readBuf); i++)
    {
        printf("0x%x ", readBuf[i]);
    }
    printf("\n");
    *isFactoryMode = (readBuf[0x40] == 0)?0:1;
    close(fd);

    return 0;
}
