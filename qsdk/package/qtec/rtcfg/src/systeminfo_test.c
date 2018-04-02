#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "systeminfo_get.h"
#include "rtcfg_uci.h"


void main()
{

    printf("==========system  test========\n");
    struct systemInfo output;
    memset(&output,0,sizeof(struct systemInfo));
    
    getSystemInfo(&output);

    printf("====output.product : %s ====\n",output.product);
    printf("====output.serialnum: %s=====\n",output.serialnum);
    printf("====output.productVersion: %s====\n",output.productVersion);
    printf("====output.configured : %d ======\n",output.configured);



}
