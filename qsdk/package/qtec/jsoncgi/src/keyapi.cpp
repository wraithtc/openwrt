#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <QtKeyEncrypt.h>
#include <QtkeyMangent.h>
#include "keyapi.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C"{
#endif

#include <sec_api.h>

struct tagCQtQkMangent
{
	CQtQkMangent cqtqkmangent;
};

struct tagCQtKeyEncrypt
{
	CQtKeyEncrypt cqtkeyencrypt;
};


struct tagCQtQkMangent *GetCQtQkMangent(void)  
{  
    return new struct tagCQtQkMangent;
} 

static int GetStartkey(unsigned long int *startkeyid)
{    
    int nRet = 0;
    int i = 0;
    int nRow = 0; 
    int nColumn = 0;
    sqlite3 *dbHangle;
    char szSql[4096] = {0};
    char** pResult;
	char* cErrMsg;
    
    nRet = sqlite3_open("/etc/testDB.db", &dbHangle);
 	if(nRet)
    {
		printf("CQtQkMangentCommon::QkPoolOpen, sqlite3_open error\n");
        return -1;
 	}

    while(i < 10)
    {
        nRet = sqlite3_get_table(dbHangle, "select * from startkey;", &pResult, &nRow, &nColumn, &cErrMsg);
        if((nRet != 0) || (nRow == 0))
    	{
            printf("get startkey failed.\n");
            sqlite3_free_table(pResult);
            i++;
            usleep(300000);
            continue;
    	}
        else
        {
            *startkeyid = atol(pResult[1]);
            printf("startkeyid is %d.\n", atol(pResult[1]));
            break;
        }
    }
	sqlite3_free_table(pResult);
    
	sqlite3_close(dbHangle);
    
    return nRet;
}

static int UpdateStartkey(unsigned long int startkeyid)
{
    int nRet = 0;
    int i = 0;
    sqlite3 *dbHangle;
    char szSql[4096] = {0};
    
    nRet = sqlite3_open("/etc/testDB.db", &dbHangle);
 	if(nRet)
    {
		printf("CQtQkMangentCommon::QkPoolOpen, sqlite3_open error\n");
        return -1;
 	}

    sprintf(szSql,"update startkey set keyid = '%d';", startkeyid);
    while(i < 10)
    {
        nRet = sqlite3_exec(dbHangle, szSql, NULL, 0, NULL);
        if(nRet)
        {
            i++;
            usleep(300000);
            continue;
        }
        else
        {
            break;
        }
    }
    
	sqlite3_close(dbHangle);
    
    return nRet;
}

int AddRawKey(void)
{
	ListKeyID lKeyId;
	ListKey lKey;
	CQtKey cKey;
	CQtKeyId cKeyId;
	CQtDeviceId    cqtdeviceid;
	CQtUserId	   cqtuserid;
	FILE *fp = NULL;
	char buf[32] = {0};
    char randBuf[16] = {0};
    char mainKey[17] = {0};
	unsigned long int startkeyid = 0;
	char keyid[17] = {0};	
	cqtdeviceid.Clear();
	cqtuserid.Clear();
	int nRet = 0;

    nRet = GetStartkey(&startkeyid);
    if(nRet)
    {
        printf("get startkeyid failed.\n");
        return nRet;
    }
    
	for(int i = 0; i < 500; i++)
	{
        memset(randBuf, 0, 16);
        GetRandom(randBuf, 16);
        int j = 0;
		printf("rawkey:");
		for(j = 0; j < 16;j++)
		{
			printf("0x%2x ",randBuf[j]);
		}
		printf("\n");
	    cKey.Clear();
	    cKey.SetValue((BYTE *)randBuf, BYTE(QT_MAX_KEY_LEN));
        
		memset(keyid, 0, 17);
		snprintf(keyid, 17, "10000000%8ld", startkeyid);
		printf("keyid is %s.\n", keyid);
		cKeyId.SetValue((BYTE *)keyid, 16);
		lKeyId.push_back(cKeyId);
		lKey.push_back(cKey);
		startkeyid++;
	}

    fp = fopen("/etc/info/mainkey", "r+");	
	if(fp)
	{
        memset(buf, 0, 32);
		if(fgets(buf, 32, fp) != NULL)
		{
			sscanf(buf, "%s", mainKey);
			fclose(fp);
			fp = NULL;			
		}
		else
		{
			return -1;
		}
	}

	CQtQkMangent *qkmangent = new CQtQkMangent();
	nRet = qkmangent->AddKey(POOL_TYPE_RAW, cqtuserid, cqtdeviceid, lKeyId, lKey, (BYTE *)mainKey);
	delete qkmangent;  
    qkmangent = 0;
	if(0 != nRet)
	{
		printf("main, qkmangent.AddKey error\n");
		return nRet;
	}
    
    nRet = UpdateStartkey(startkeyid);
    if(nRet)
    {
        printf("update startkeyid failed.\n");
        return nRet;
    }
    
	return 0;
}

void ReleaseCQtQkMangent(struct tagCQtQkMangent **pstcqtqkmangent)  
{  
    delete *pstcqtqkmangent;  
    *pstcqtqkmangent = 0;  
} 

struct tagCQtKeyEncrypt *GetCQtKeyEncrypt(void)  
{  
    	return new struct tagCQtKeyEncrypt;
} 

void ReleaseCQtKeyEncrypt(struct tagCQtKeyEncrypt **pstcqtkeyencrypt)  
{  
    delete *pstcqtkeyencrypt;  
    *pstcqtkeyencrypt = 0;      
} 

int C_GetKeyByNode(struct tagCQtQkMangent *p,char nPoolType, unsigned char nNumber, char *userId, char *deviceid, unsigned char *lKeyId, unsigned char *lKey)
{
	ListKeyID   listkeyid;
	ListKey     listkey;
	ListKeyID::iterator KeyIdItor;
	ListKey::iterator KeyItor;
	CQtDeviceId cqtdeviceid;
	CQtUserId	   cqtuserid;
	unsigned char *keytemp = lKey;
	unsigned char *keyidtemp = lKeyId;
	int result = 0;
    char mainKey[17] = {0};
	char buf[32] = {0};
	FILE *fp = NULL;

	cqtdeviceid.SetValue((unsigned char *)deviceid, strlen(deviceid));
	cqtuserid.SetValue((unsigned char *)userId, strlen(userId));

    fp = fopen("/etc/info/mainkey", "r+");	
	if(fp)
	{
        memset(buf, 0, 32);
		if(fgets(buf, 32, fp) != NULL)
		{
			sscanf(buf, "%s", mainKey);
            fclose(fp);
            fp = NULL;
		}
		else
		{
			return -1;
		}
	}
	if(1 == nPoolType)
	{
        result = p->cqtqkmangent.GetRawKeyByNode((unsigned char)nPoolType, (unsigned char)nNumber, cqtuserid, cqtdeviceid, listkeyid, listkey, (BYTE *)mainKey);
    }
    else
    {
	    result = p->cqtqkmangent.GetKeyByNode((unsigned char)nPoolType, (unsigned char)nNumber, cqtuserid, cqtdeviceid, listkeyid, listkey, (BYTE *)mainKey);
    }
	if(result)
	{
		printf("get key by userid failed.\r\n");
		return result;
	}

	KeyIdItor = listkeyid.begin();
	KeyItor = listkey.begin();
	
	for(KeyIdItor = listkeyid.begin(),KeyItor = listkey.begin(); \
		KeyIdItor != listkeyid.end(),KeyItor != listkey.end(); \
			++KeyIdItor,++KeyItor)
	{
		memset(keyidtemp, 0, KEYIDLEN);
		memset(keytemp, 0, KEYLEN);
		memcpy(keyidtemp, KeyIdItor->GetBuffer(), KEYIDLEN);
		memcpy(keytemp, KeyItor->GetBuffer(), KEYLEN);
		keytemp += KEYLEN;
		keyidtemp += KEYIDLEN;
	}
	return 0;
}
 
int C_GetKeyByIdNode(struct tagCQtQkMangent *p,char nPoolType, unsigned char nNumber, char *userId, char *deviceid, unsigned char *lKeyId, unsigned char *lKey)
{
	ListKey     listkey;
	ListKeyID      listkeyid;
	CQtDeviceId    cqtdeviceid;
	CQtUserId	   cqtuserid;
	CQtKeyId       cktkeyid;
	ListKey::iterator KeyItor;
	int index;
	unsigned char *keytemp = lKey;
	int result = 0;
    char mainKey[17] = {0};
	char buf[32] = {0};
	FILE *fp = NULL;

	cqtdeviceid.Clear();
	cqtdeviceid.SetValue((unsigned char *)deviceid, strlen(deviceid));
	cqtuserid.SetValue((unsigned char *)userId, strlen(userId));
	
	for(index = 0; index < nNumber; index++)
	{
		cktkeyid.Clear();
		cktkeyid.SetValue((unsigned char *)(lKeyId + KEYLEN * index), KEYLEN);
		listkeyid.push_back(cktkeyid);
	}

    fp = fopen("/etc/info/mainkey", "r+");	
	if(fp)
	{
        memset(buf, 0, 32);
		if(fgets(buf, 32, fp) != NULL)
		{
			sscanf(buf, "%s", mainKey);
            fclose(fp);
            fp = NULL;
		}
		else
		{
			return -1;
		}
	}
	result = p->cqtqkmangent.GetKeyByIdNode((unsigned char)nPoolType, (unsigned char)nNumber, cqtuserid, cqtdeviceid, listkeyid, listkey, (BYTE *)mainKey);
	if(result)
	{
		printf("get key by keyid failed.\r\n");
		return result;
	}
	for(KeyItor = listkey.begin(); KeyItor != listkey.end(); ++KeyItor)
	{
		memcpy(keytemp, KeyItor->GetBuffer(), KEYLEN);
		keytemp += KEYLEN;
	}

	return 0;
}

int C_DeleteKeyById(struct tagCQtQkMangent *p, char nPoolType, int keynumber, unsigned char *lKeyId)
{

	ListKeyID      listkeyid;
	CQtKeyId       cktkeyid;
	int index;
	int result = 0;

	for(index = 0; index < keynumber; index++)
	{
		cktkeyid.Clear();
		cktkeyid.SetValue((unsigned char *)(lKeyId + KEYIDLEN * index), KEYIDLEN);
		listkeyid.push_back(cktkeyid);
	}
	
	result = p->cqtqkmangent.DeleteKeyById((unsigned char)nPoolType, listkeyid);
	if(result)
	{
		printf("DeleteKeyById failed.\r\n");
		return result;
	}

	return 0;
}

int C_DeleteKeyByNode(struct tagCQtQkMangent *p, char nPoolType, char *userId, char *deviceid)
{
	CQtDeviceId    cqtdeviceid;
	CQtUserId	   cqtuserid;
	int result = 0;

	cqtdeviceid.SetValue((unsigned char *)deviceid, strlen(deviceid));
	cqtuserid.SetValue((unsigned char *)userId, strlen(userId));

	result = p->cqtqkmangent.DeleteKeyByNode((unsigned char)nPoolType, cqtuserid, cqtdeviceid);
	if(result)
	{
		printf("DeleteKeyByNode failed.\r\n");
		return result;
	}

	return 0;
}

int C_DeleteSynKeyByNodeId(struct tagCQtQkMangent *p, unsigned char *lKeyId, char *userId, char *deviceid)
{
	CQtDeviceId    cqtdeviceid;
	CQtUserId	   cqtuserid;
	int result = 0;

#if 0    
    unsigned long int originKeyId = 0;    
    unsigned long int targetKeyId = 0;
    char tarKeyId[KEYIDLEN+1] = {0};
    char tmpbuf[KEYIDLEN+1] = {0};

    /* 删除比keyid小20的之前的key */
    memcpy(tmpbuf, lKeyId, KEYIDLEN);
    sscanf(tmpbuf,"10000000%8ld", &originKeyId);
    targetKeyId = originKeyId - 20;
    snprintf(tarKeyId, 17, "10000000%8ld", targetKeyId);
    printf("tarKeyId is %s.\n", tarKeyId);
#endif
    
	cqtdeviceid.SetValue((unsigned char *)deviceid, strlen(deviceid));
	cqtuserid.SetValue((unsigned char *)userId, strlen(userId));

	result = p->cqtqkmangent.DeleteSynKeyByNodeId((unsigned char *)lKeyId, cqtuserid, cqtdeviceid);
	if(result)
	{
		printf("DeleteSynKeyByNodeId failed.\r\n");
		return result;
	}

	return 0;
}

int C_AddKey(struct tagCQtQkMangent *p,char nPoolType, int keynumber, char *userId, char *deviceid, unsigned char *lKeyId, unsigned char *lKey)
{

	ListKeyID      listkeyid;
	CQtKeyId       cktkeyid;
	ListKey        listkey;
	CQtKey         cktkey;
	ListKey::iterator KeyItor;
	int index;
	CQtDeviceId    cqtdeviceid;
	CQtUserId	   cqtuserid;
	int result = 0;
    char mainKey[17] = {0};
	char buf[32] = {0};
	FILE *fp = NULL;

	for(index = 0; index < keynumber; index++)
	{
		cktkeyid.Clear();
		cktkeyid.SetValue((unsigned char *)(lKeyId + KEYIDLEN * index), KEYIDLEN);
		listkeyid.push_back(cktkeyid);

		cktkey.Clear();
		cktkey.SetValue((unsigned char *)(lKey + KEYLEN * index), KEYLEN);
		listkey.push_back(cktkey);
	}
	
	cqtdeviceid.SetValue((unsigned char *)deviceid, strlen(deviceid));
	cqtuserid.SetValue((unsigned char *)userId, strlen(userId));

    fp = fopen("/etc/info/mainkey", "r+");	
	if(fp)
	{
        memset(buf, 0, 32);
		if(fgets(buf, 32, fp) != NULL)
		{
			sscanf(buf, "%s", mainKey);
            fclose(fp);
            fp = NULL;
		}
		else
		{
			return -1;
		}
	}
	result = p->cqtqkmangent.AddKey((unsigned char)nPoolType, cqtuserid, cqtdeviceid, listkeyid, listkey, (BYTE *)mainKey);
	if(result)
	{
		printf("addkey failed.\r\n");
		return result;
	}

	return 0;
}

int C_GetCount(struct tagCQtQkMangent *p,char nPoolType, int *pUsedCount, int *pUnusedCount, char *userId, char *deviceid)
{
	CQtDeviceId    cqtdeviceid;
	CQtUserId	   cqtuserid;
	int result = 0;
	
	cqtdeviceid.SetValue((unsigned char *)deviceid, strlen(deviceid));
	cqtuserid.SetValue((unsigned char *)userId, strlen(userId));
	result = p->cqtqkmangent.GetCount((unsigned char)nPoolType, pUsedCount, pUnusedCount, cqtuserid, cqtdeviceid);
	if(result)
	{
		printf("C_GetCount failed.\r\n");
		return result;
	}

	return 0;
}


int C_EncryptOrDecrypt(struct tagCQtKeyEncrypt *p, char *dest, int *destLen, char *source, int sourceLen, char encryAlgType, unsigned char *key)
{
	int destlen;
	int result = 0;
	result = p->cqtkeyencrypt.EncryptOrDecrypt((unsigned char *)dest, destlen, (unsigned char *)source, sourceLen, (unsigned char)encryAlgType, (unsigned char *)key);
	if(result)
	{
		printf("C_EncryptOrDecrypt failed.\r\n");
		return result;
	}
	return 0;
}


int C_DealClientSynReq(DestMessage *pDest, SrcMessage *pSrc)
{
	int result = 0;
	struct timeval tv;
	struct tagCQtQkMangent *pstcqtqkmangent;
	pstcqtqkmangent = GetCQtQkMangent();

	pDest->keyNumber = pSrc->keyNumber;
	pDest->keyLength = pSrc->keyLength;
	strcpy(pDest->pushDeviceId, pSrc->localDeviceId);
	strcpy(pDest->pushUserId, pSrc->localUserId);

	/* get rawkey */
	gettimeofday(&tv, NULL);  
    printf("before C_GetKeyByNode.%ld.%ld \n", tv.tv_sec, tv.tv_usec);  
	result += C_GetKeyByNode(pstcqtqkmangent, POOL_TYPE_RAW, pSrc->keyNumber, pSrc->localUserId, pSrc->localDeviceId, pDest->keyId, pDest->key);
	
	/* delete rawkey */
	gettimeofday(&tv, NULL);  
    printf("before C_DeleteKeyById.%ld.%ld \n", tv.tv_sec, tv.tv_usec); 
	result += C_DeleteKeyById(pstcqtqkmangent, POOL_TYPE_RAW, pSrc->keyNumber, pDest->keyId);
	
	/* add synkey */
	gettimeofday(&tv, NULL);  
    printf("before C_AddKey.%ld.%ld \n", tv.tv_sec, tv.tv_usec); 
	result += C_AddKey(pstcqtqkmangent, POOL_TYPE_SYNC, pSrc->keyNumber, pSrc->localUserId, pSrc->localDeviceId, pDest->keyId, pDest->key);
	gettimeofday(&tv, NULL);  
    printf("after C_AddKey.%ld.%ld \n", tv.tv_sec, tv.tv_usec); 
	
	ReleaseCQtQkMangent(&pstcqtqkmangent);
	if(result)
	{
		printf("C_DealClientSynReq failed.\r\n");
		return result;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif



