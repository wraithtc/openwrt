#ifndef QT_KEYSYNSERVICE_H
#define QT_KEYSYNSERVICE_H

#include "QtManagent_defines.h"
#include "QtkeyMangent.h"
#include "QtKeyTool.h"

class CQtKeyDistribute
{
public:
	CQtKeyDistribute();
	~CQtKeyDistribute();

	virtual int Initialize(void *);
	virtual int UnInitialize(void *);

public:
	virtual int DealClientSynReq(char *pDest, DWORD &nDestLen, const char *pSrc, DWORD nSrcLen);

public:
	void PrintBuffer(const char *pBuff1, int nLen);

private:
	CQtQkMangent m_QkManganet;
	QtKeyTool m_QkTool;

};

#endif
