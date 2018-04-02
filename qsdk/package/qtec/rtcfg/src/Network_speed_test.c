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

    bool output_enabled;
    int output_upload;
    int output_download;
    get_wanbandwidth_config(&output_enabled, &output_upload, &output_download);

    DEBUG_PRINTF("===output_enabled:%d output_upload:%d  output_download:%d==\n",output_enabled, output_upload,output_download);

    bool input_enabled=0;
    int input_upload=2048;
    int input_download=2048;
    set_wanbandwidth_config(input_enabled, input_upload, input_download,1);

    get_wanbandwidth_config(&output_enabled, &output_upload, &output_download);

  
    wan_speedtest(&output_upload, &output_download);
}