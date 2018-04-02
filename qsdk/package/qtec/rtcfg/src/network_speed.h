#ifndef NETWORK_SPEED_H
#define NETWORK_SPEED_H
#include "stdio.h"

int get_cur_wan_speed(char *output_upspeed, char* output_downspeed);
int get_wanbandwidth_config(bool *output_enabled, int *output_upload, int *output_download);
int set_wanbandwidth_config(bool input_enabled, int input_upload, int input_download,int flag);
int wan_speedtest(float *output_upload, float *output_download);
#endif

