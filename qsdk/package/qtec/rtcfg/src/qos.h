#ifndef QOS_H
#define QOS_H
#include "stdio.h"

int Get_qosmode();
int qosmode_init();
int qosmode_funtion(int *input_qosmode);
int Set_qosmode(int *input_qosmode,int flag);
#endif