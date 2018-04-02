#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "network_speed.h"
#include "rtcfg_uci.h"

void main()
{
    DEBUG_PRINTF("===network_speed test ====\n");
    char output_upspeed[64]={0};
    char output_downspeed[64]={0};
    get_cur_wan_speed(output_upspeed, output_downspeed);
}