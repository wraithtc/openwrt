#ifndef ARPBAND_SET_H
#define ARPBAND_SET_H

int setarpband(char *mac, char *ip);
int delarpband(char *mac, char *ip);
int cleararpband();
int getarpband(char *buf, int len);

#endif

