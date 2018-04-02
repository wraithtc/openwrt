#include "QtkeyMangent.h"
#include <iostream>
#include <sstream>
#include <string>

void Change(char s[],char bits[]) 
{
    int i,n = 0;
    for(i = 0; s[i]; i += 2) {
        if(s[i] >= 'A' && s[i] <= 'F')
            bits[n] = s[i] - 'A' + 10;
        else bits[n] = s[i] - '0';
        if(s[i + 1] >= 'A' && s[i + 1] <= 'F')
            bits[n] = (bits[n] << 4) | (s[i + 1] - 'A' + 10);
        else bits[n] = (bits[n] << 4) | (s[i + 1] - '0');
        ++n;
    }
    return;
}

void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
	ddh = 48 + pbSrc[i] / 16;
	ddl = 48 + pbSrc[i] % 16;
	if (ddh > 57) ddh = ddh + 7;
	if (ddl > 57) ddl = ddl + 7;
	pbDest[i*2] = ddh;
	pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}

int main()
{
	int nRet = 0;
	CQtQkMangent *qkmangent = new CQtQkMangent();

	QT_LONG UserId = 0;
	CQtDeviceId DeviceId;
	DeviceId.SetValue((BYTE *)"iphone", BYTE(strlen("iphone")));
	ListKeyID lKeyId;
	ListKey lKey;
	ListKeyID::iterator KeyIdItor;
	ListKey::iterator KeyItor;
	CQtKeyId cKeyId;
	CQtKey cKey;

	//raw_key
	/*PRINT("begin get count\n");
	int UsedCount = 0;
	int UnusedCount = 0;
	nRet = qkmangent->GetCount(POOL_TYPE_RAW, &UsedCount, &UnusedCount, UserId, DeviceId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetCount error\n");
		return QKPOOL_FAIL;
	}
	cout << "UsedCount = " << UsedCount << endl;
	cout << "UnusedCount = " << UnusedCount << endl;
	*/

	/*
	PRINT("begin get some keys by node\n");
	ListKeyID lKeyId1;
	ListKey lKey1;
	lKeyId.clear();
	lKey.clear();
	nRet = qkmangent->GetKeyByNode(POOL_TYPE_RAW, 2, UserId, DeviceId, lKeyId1, lKey1);
	if(0 != nRet)
	{
		PRINT("main, qkmangent->GetKeyByNode error\n");
		return QKPOOL_FAIL;
	}
	for(KeyIdItor = lKeyId1.begin(),KeyItor = lKey1.begin(); \
					KeyIdItor != lKeyId1.end(),KeyItor != lKey1.end(); \
					++KeyIdItor,++KeyItor)
	{
		cout << "keyId:" << KeyIdItor->GetBuffer() << endl;;
		cout << "key:" << KeyItor->GetBuffer() << endl;;
	}
	*/
	
	/*
	PRINT("begin get some keys by keyid\n");
	lKeyId.clear();
	lKey.clear();
	cKeyId.SetValue((BYTE *)"1234567812341235", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	cKeyId.SetValue((BYTE *)"1234567812341236", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	nRet = qkmangent->GetKeyById(POOL_TYPE_RAW, 2, UserId, DeviceId, lKeyId, lKey);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetKeyById error\n");
		return QKPOOL_FAIL;
	}
	for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
					KeyIdItor != lKeyId.end(),KeyItor != lKey.end(); \
					++KeyIdItor,++KeyItor)
	{
		cout << "keyId:" << KeyIdItor->GetBuffer() << endl;;
		cout << "key:" << KeyItor->GetBuffer() << endl;;
	}
	*/

	/*
	nRet = qkmangent->DeleteKeyByNode(POOL_TYPE_RAW, UserId, DeviceId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetKeyByIdNode error\n");
		return QKPOOL_FAIL;
	}
	*/
	
	/*
	lKeyId.clear();
	cKeyId.SetValue((BYTE *)"1234567812341236", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	nRet = qkmangent->DeleteKeyById(POOL_TYPE_RAW, lKeyId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.DeleteKeyById error\n");
		return QKPOOL_FAIL;
	}
	*/

	/*
	nRet = qkmangent->DeleteKeyByNode(POOL_TYPE_SYNC, UserId, DeviceId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetKeyByIdNode error\n");
		return QKPOOL_FAIL;
	}
	*/
	
	/*
	lKeyId.clear();
	lKey.clear();
	cKeyId.SetValue((BYTE *)"1234567812341235", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	nRet = qkmangent->DeleteKeyById(POOL_TYPE_SYNC, lKeyId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.DeleteKeyById error\n");
		return QKPOOL_FAIL;
	}
	*/



	//syn_key test
	/*PRINT("begin get count\n");
	int UsedCount = 0;
	int UnusedCount = 0;
	nRet = qkmangent->GetCount(POOL_TYPE_SYNC, &UsedCount, &UnusedCount, UserId, DeviceId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetCount error\n");
		return QKPOOL_FAIL;
	}
	cout << "UsedCount = " << UsedCount << endl;
	cout << "UnusedCount = " << UnusedCount << endl;
	*/
	
	/*
	PRINT("begin get some keys by node\n");
	ListKeyID lKeyId1;
	ListKey lKey1;
	nRet = qkmangent->GetKeyByNode(POOL_TYPE_SYNC, 2, UserId, DeviceId, lKeyId1, lKey1);
	if(0 != nRet)
	{
		PRINT("main, qkmangent->GetKeyByNode error\n");
		return QKPOOL_FAIL;
	}
	for(KeyIdItor = lKeyId1.begin(),KeyItor = lKey1.begin(); \
					KeyIdItor != lKeyId1.end(),KeyItor != lKey1.end(); \
					++KeyIdItor,++KeyItor)
	{
		cout << "keyId:" << KeyIdItor->GetBuffer() << endl;;
		cout << "key:" << KeyItor->GetBuffer() << endl;;
	}
	*/
	
	/*
	PRINT("begin get some keys by keyid\n");
	lKeyId.clear();
	lKey.clear();
	cKeyId.SetValue((BYTE *)"1234567812341237", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	cKeyId.SetValue((BYTE *)"1234567812341238", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	nRet = qkmangent->GetKeyById(POOL_TYPE_SYNC, 2, UserId, DeviceId, lKeyId, lKey);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetKeyById error\n");
		return QKPOOL_FAIL;
	}
	for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
					KeyIdItor != lKeyId.end(),KeyItor != lKey.end(); \
					++KeyIdItor,++KeyItor)
	{
		cout << "keyId:" << KeyIdItor->GetBuffer() << endl;;
		cout << "key:" << KeyItor->GetBuffer() << endl;;
	}
	*/

	/*
	PRINT("begin get some keys by keyidnode\n");
	lKeyId.clear();
	lKey.clear();
	cKeyId.SetValue((BYTE *)"1234567812341235", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	cKeyId.SetValue((BYTE *)"1234567812341236", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	nRet = qkmangent->GetKeyByIdNode(POOL_TYPE_SYNC, 2, UserId, DeviceId, lKeyId, lKey);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetKeyByIdNode error\n");
		return QKPOOL_FAIL;
	}
	for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
					KeyIdItor != lKeyId.end(); \
					++KeyIdItor,++KeyItor)
	{
		cout << "keyId:" << KeyIdItor->GetBuffer() << endl;;
		cout << "key:" << KeyItor->GetBuffer() << endl;;
	}
	*/

	/*
	nRet = qkmangent->DeleteKeyByNode(POOL_TYPE_SYNC, UserId, DeviceId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.GetKeyByIdNode error\n");
		return QKPOOL_FAIL;
	}
	*/
	
	/*
	lKeyId.clear();
	lKey.clear();
	cKeyId.SetValue((BYTE *)"1234567812341235", BYTE(QT_MAX_KEY_ID_LEN));
	lKeyId.push_back(cKeyId);
	nRet = qkmangent->DeleteKeyById(POOL_TYPE_SYNC, lKeyId);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.DeleteKeyById error\n");
		return QKPOOL_FAIL;
	}
	*/
	
	/*
	string keyId_tmp("");
	int id = 1234;
	cKey.SetValue((BYTE *)"1234567812345678", BYTE(QT_MAX_KEY_LEN));
	for(int i = 0; i < 10; i++){
		keyId_tmp.clear();
		id++;
		keyId_tmp += "123456781234";
		std::stringstream ss;
		string tmp;
        ss << id;
		tmp = ss.str();
		keyId_tmp += tmp.c_str();
		cKeyId.SetValue((BYTE *)keyId_tmp.c_str(), BYTE(QT_MAX_KEY_ID_LEN));

		lKeyId.push_back(cKeyId);
		lKey.push_back(cKey);
	}
	
	/*
	nRet = qkmangent->AddKey(POOL_TYPE_RAW, UserId, DeviceId, lKeyId, lKey);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.AddKey error\n");
		return QKPOOL_FAIL;
	}
	*/
	
	/*
	nRet = qkmangent->AddKey(POOL_TYPE_SYNC, UserId, DeviceId, lKeyId, lKey);
	if(0 != nRet)
	{
		PRINT("main, qkmangent.AddKey error\n");
		return QKPOOL_FAIL;
	}
	*/
	
	return 0;
}
