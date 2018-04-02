#ifndef __SEC_API_H__
#define __SEC_API_H__

int GetRandom(char *randBuf, int len);

int SaveData(const char *data, int len);

int LoadData(char *data, int len);

#endif /* __SEC_API_H__ */
