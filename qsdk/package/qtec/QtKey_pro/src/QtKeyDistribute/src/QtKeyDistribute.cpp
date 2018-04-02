#include "QtKeyDistributeService.h"
#include "QtKeyDistributePDU.h"
#include "QtkeyMangent.h"
#include "QtMessageBlock.h"


CQtKeyDistribute::CQtKeyDistribute()
{	

}


CQtKeyDistribute::~CQtKeyDistribute()
{

}

int CQtKeyDistribute::Initialize(void *)
{
	
	return QT_OK;
}


int CQtKeyDistribute::UnInitialize(void *)
{

	return QT_OK;
}

void CQtKeyDistribute::PrintBuffer(const char *pBuff1, int nLen)
{
	char *pBuff2 = new char[nLen*2+1];
	
	char *pTransform = "0123456789abcdef";
	for(int i=0; i<nLen; i++)
	{
		BYTE first4 = (pBuff1[i]&0xf0) >> 4;
		BYTE last4 = (pBuff1[i]&0x0f);
		pBuff2[i*2] = pTransform[first4];
		pBuff2[i*2+1] = pTransform[last4];
	}

	pBuff2[nLen*2] = 0;

	printf("%s\n", pBuff2);
	QT_INFO_TRACE(pBuff2);
	
	delete[] pBuff2;
}

int CQtKeyDistribute::DealClientSynReq(char *pDest, DWORD &nDestLen, const char *pSrc, DWORD nSrcLen)
{
	int nRet = QT_OK;

	printf("enter DealClientSynReq ok.\r\n");
	unsigned int index =0;
	printf("src is:");
	for(index = 0;index < nSrcLen; index++)
	{
		printf("%0.2x ", *(pSrc + index));
	}
	printf("\r\n");

	fflush(stdout);
	

	/* decode */
	CQtMessageBlock aMessage(nSrcLen);
	aMessage.Write(pSrc, nSrcLen);


	CQtKeySynReq req;
	if(QT_OK != req.Decode(aMessage)){
		PRINT("CQtKeyServiceModule::DealClientReq, dec error\n");
		return QT_ERROR_FAILURE;
	}
	
	/* RequestId */
	WORD wRequestId = req.GetRequestID();
	
	/* byKeyType */
	BYTE byKeyType = req.GetKeyType();
	if(!(QT_KEY_TYPE_GLOBAL == byKeyType)){
		PRINT("CQtKeyServiceModule::DealClientReq, byKeyType error\n");
		return QT_ERROR_FAILURE;
	}

	/* byKeyNum */
	BYTE byKeyNum = req.GetKeyNum();
	if(MIN_SYN_NUMBER > byKeyNum || MAX_SYN_NUMBER < byKeyNum )
	{
		PRINT("CQtKeyServiceModule::DealClientReq, byKeyNum is too long\n");
		return QT_ERROR_FAILURE;
	}
	
	/* byKeyIDLength */
	BYTE byKeyIDLength = req.GetKeyLength();
	if(QT_MAX_KEY_LEN != byKeyIDLength){
		PRINT("CQtKeyServiceModule::DealClientReq, byKeyLength error\n");
		return QT_ERROR_FAILURE;
	}

	/* bySpecified */
	BYTE bySpecified = req.GetSpecified();
	if(!(0 != bySpecified || 1 != bySpecified)){
		PRINT("CQtKeyServiceModule::DealClientReq, bySpecified error\n");
		return QT_ERROR_FAILURE;
	}
	
	/* bShareNumber */
	BYTE byShareNumber = req.GetShareNumber();

	QT_LONG 	LocalUserId = req.GetLocalUserId();
	CQtDeviceId LocalDeviceId = req.GetLocalDeviceId();
	
	/* get rawkey */
	ListKeyID lKeyId;
	ListKey lKey;
	nRet = m_QkManganet.GetKeyByNode(POOL_TYPE_RAW, byKeyNum, (QT_LONG)0, LocalDeviceId, lKeyId, lKey);
	if(QT_OK != nRet)
	{
		PRINT("CQtKeyDistribute::DealClientReq, get rawkey error!\n");
		return QT_ERROR_FAILURE;
	}

	ListKeyID::iterator KeyIdItor;
	ListKey::iterator KeyItor;
	for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
			KeyIdItor != lKeyId.end(),KeyItor != lKey.end(); \
			++KeyIdItor,++KeyItor)
	{
		cout << "keyId:" << KeyIdItor->GetBuffer() << endl;;
		cout << "key:" << KeyItor->GetBuffer() << endl;;
	}
	
	/* delete rawkey */
	nRet = m_QkManganet.DeleteKeyById(POOL_TYPE_RAW, lKeyId);
	if(QT_OK != nRet)
	{
		PRINT("CQtKeyDistribute::DealClientReq, delete rawkey error!\n");
		return QT_ERROR_FAILURE;
	}

	/* save synkey */
	nRet = m_QkManganet.AddKey(POOL_TYPE_SYNC, LocalUserId, LocalDeviceId, lKeyId, lKey);
	if(QT_OK != nRet)
	{
		PRINT("CQtKeyDistribute::DealClientReq, get rawkey error!\n");
		return QT_ERROR_FAILURE;
	}
	
	/* Packet message */
	CQtKeySynResp relayresp(wRequestId,
			0,
			byKeyType,
			byKeyNum,
			QT_MAX_KEY_LEN,
			LocalUserId,
			LocalDeviceId,
			lKeyId,
			lKey);
	CQtMessageBlock mbBlcok(relayresp.GetLength());
	relayresp.Encode(mbBlcok);
	
	mbBlcok.Read(pDest, relayresp.GetLength());
	
	return QT_OK;
}

#if 0
int CQtKeyDistribute::DealClientSynConf(const char*pSrc, DWORD nSrcLen)
{
	//½âÎö±¨ÎÄ
	


}

int CQtKeyDistribute::DealClientPushReq(const char*pSrc, DWORD nSrcLen)
{
	int nRet = QT_OK;
	
	/* decode */
	CQtMessageBlock aMessage(nSrcLen);
	aMessage.Write(pSrc, nSrcLen);
	CQtKeyPushReq req;
	if(QT_OK != req.Decode(aMessage)){
		PRINT("CQtKeyDistribute::DealClientPushReq, dec error\n");
		return QT_ERROR_FAILURE;
	}
	
	/* RequestId */
	WORD wRequestId = req.GetRequestID();

	/* byKeyType */
	BYTE byKeyType = req.GetKeyType();
	if(!(QT_KEY_TYPE_GLOBAL == byKeyType)){
		PRINT("CQtKeyDistribute::DealClientPushReq, byKeyType error\n");
		return QT_ERROR_FAILURE;
	}

	/* byKeyNum */
	BYTE byKeyNum = req.GetKeyNum();
	if(MIN_SYN_NUMBER > byKeyNum || MAX_SYN_NUMBER < byKeyNum )
	{
		PRINT("CQtKeyDistribute::DealClientPushReq, byKeyNum is too long\n");
		return QT_ERROR_FAILURE;
	}
	
	/* byKeyIDLength */
	BYTE byKeyLength = req.GetKeyLength();
	if(QT_MAX_KEY_LEN != byKeyLength){
		PRINT("CQtKeyDistribute::DealClientPushReq, byKeyLength error\n");
		return QT_ERROR_FAILURE;
	}

	/* UserId */
	//QT_LONG UserId = req.GetLocalNodeID();
	//QT_LONG RemoteId = req.GetRemotoNodeID();
	QT_LONG UserId = DEFAULT_USERID;
	QT_LONG RemoteId = DEFAULT_USERID;

	/* get clientA synkey */
	ListKeyID lKeyId;
	ListKey lKey;
	nRet = m_QkManganet.GetKeyByNode(POOL_TYPE_SYNC, byKeyNum, UserId, lKeyId, lKey);
	if(QT_OK != nRet)
	{
		PRINT("CQtKeyDistribute::DealClientPushReq, get synkey error!\n");
		return QT_ERROR_FAILURE;
	}

	/* get clientB synkey */
	ListKeyID lKeyId_syn;
	ListKey lKey_syn;
	nRet = m_QkManganet.GetKeyByNode(POOL_TYPE_SYNC, UserId, RemoteId, lKeyId_syn, lKey_syn);
	if(QT_OK != nRet)
	{
		PRINT("CQtKeyDistribute::DealClientPushReq, get synkey error!\n");
		return QT_ERROR_FAILURE;
	}
	
	/* push synkey */
	BYTE byVersion = 1;
	BYTE byEncryptAlgrithm = 1;
	WORD wDataLength = byKeyNum * DEFAULT_KEY_LEN;
	CQtKeyAnotherResp pushresp(wRequestId,
			0,
			byKeyType,
			byKeyNum,
			QT_MAX_KEY_LEN,
			UserId,
			lKeyId,
			byVersion,
			byEncryptAlgrithm,
			wDataLength, 
			*lKeyId_syn.begin(),
			lKey);
	CQtMessageBlock mbBlcok(pushresp.GetLength());
	pushresp.Encode(mbBlcok);

	/* send message */
	

	
	/* update peerList */
	string userIdList;
	userIdList.clear();
	userIdList += m_QkTool.ltos(UserId);	
	userIdList += ";";
	userIdList += m_QkTool.ltos(RemoteId);	
	userIdList += ";";
	nRet = m_QkManganet.UpdatePeerById(POOL_TYPE_SYNC, userIdList, lKeyId);
	if(QT_OK != nRet)
	{
		PRINT("CQtKeyDistribute::DealClientPushReq, add peer error!\n");
		return QT_ERROR_FAILURE;
	}
	
	/* send clientA conf */
	CQtKeyPushConf(wRequestId,0);

	return QT_OK;
}

int CQtKeyDistribute::DealClientSynPushConf(const char*pSrc, DWORD nSrcLen)
{



}
#endif

