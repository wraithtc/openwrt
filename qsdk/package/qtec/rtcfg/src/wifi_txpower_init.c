#include "wifi_txpower.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int ret;
    int mode;
    
    ret = QtGetWifiMode(&mode);

    if (ret == 0)
    {
        QtSetWifTxpower(mode);
    }
    else
    {
        QtSetWifTxpower(2);
    }

    return 0;
}
