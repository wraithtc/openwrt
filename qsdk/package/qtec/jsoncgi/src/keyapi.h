#ifndef KEYAPI_H
#define KEYAPI_H

#include "encryption.h"

struct tagCQtQkMangent;
struct tagCQtKeyEncrypt;

#ifdef __cplusplus
extern "C"{
#endif

struct tagCQtQkMangent *GetCQtQkMangent(void);
void ReleaseCQtQkMangent(struct tagCQtQkMangent **pstcqtqkmangent);
struct tagCQtKeyEncrypt *GetCQtKeyEncrypt(void);
void ReleaseCQtKeyEncrypt(struct tagCQtKeyEncrypt **pstcqtkeyencrypt);
struct tagCQtKeyDistribute *GetCQtKeyDistribute(void); 
void ReleaseCQtKeyDistribute(struct tagCQtKeyDistribute **pstcqtkeydistribute);
  
int AddRawKey(void);
int C_GetKeyByNode(struct tagCQtQkMangent *p,char nPoolType, unsigned char nNumber, char *userId, char *deviceid, unsigned char *lKeyId, unsigned char *lKey);
int C_GetKeyByIdNode(struct tagCQtQkMangent *p,char nPoolType, unsigned char nNumber, char *userId, char *deviceid, unsigned char *lKeyId, unsigned char *lKey);
int C_DeleteKeyById(struct tagCQtQkMangent *p, char nPoolType, int keynumber, unsigned char *lKeyId);
int C_DeleteKeyByNode(struct tagCQtQkMangent *p, char nPoolType, char *userId, char *deviceid);
int C_DeleteSynKeyByNodeId(struct tagCQtQkMangent *p, unsigned char *lKeyId, char *userId, char *deviceid);
int C_AddKey(struct tagCQtQkMangent *p,char nPoolType, int keynumber, char *userId, char *deviceid, unsigned char *lKeyId, unsigned char *lKey);
int C_GetCount(struct tagCQtQkMangent *p,char nPoolType, int *pUsedCount, int *pUnusedCount, char *userId, char *deviceid);
int C_EncryptOrDecrypt(struct tagCQtKeyEncrypt *p, char *dest, int *destLen, char *source, int sourceLen, char encryAlgType, unsigned char *key);
int C_DealClientSynReq(DestMessage *pDest, SrcMessage *pSrc);


#ifdef __cplusplus
}
#endif

#endif
