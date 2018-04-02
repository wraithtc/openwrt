#ifndef SYSTEMINFOGET_H
#define SYSTEMINFOGET_H

/*
 * basic system info
 */
struct systemInfo{
    char product[64];
    char serialnum[64];
    char productVersion[64];
    int configured;
};

int getSystemInfo(struct systemInfo *output);

int SetSystemConfigured(int value);

int GetSystemConfigured(int *output_value);

int SetGuiPassword(char *password);

int GetGuiPassword(char *password);

int CheckGuiPassword(char *password);

int setTokenId(char *inputTokenId);

int getTokenId(char *outputTokenId);

int checkTokenId(char *inputTokenId);


#endif
