
#include "QtBase.h"
#include "QtDataBlock.h"

QtResult CQtDataBlock::
CreateInstance(CQtDataBlock *&aDb, DWORD aSize, LPCSTR aData)
{
	QT_ASSERTE(!aDb);
	QT_ASSERTE_RETURN(aSize > 0, QT_ERROR_INVALID_ARG);

	// alloc sizeof(CQtDataBlock) and <aSize> at one time.
	std::allocator<char> allocChar;
	char *pBuf = allocChar.allocate(sizeof(CQtDataBlock) + aSize, NULL);
	if (!pBuf)
		return QT_ERROR_OUT_OF_MEMORY;

	// call constructor function of CQtDataBlock.
	LPSTR pDbData = pBuf + sizeof(CQtDataBlock);
	if (aData)
		::memcpy(pDbData, aData, aSize);
	new (pBuf) CQtDataBlock(aSize, pDbData);
	
	aDb = static_cast<CQtDataBlock*>(static_cast<void*>(pBuf));
	aDb->AddReference();
	return QT_OK;
}

CQtDataBlock::CQtDataBlock(DWORD aSize, LPSTR aData)
	: m_dwSize(aSize)
	, m_pData(aData)
{
}

CQtDataBlock::~CQtDataBlock()
{
	// Don't delete m_pData because it is combiled with <CQtDataBlock>.
}

void CQtDataBlock::OnReferenceDestory()
{
	DWORD dwLen = m_dwSize + sizeof(CQtDataBlock);
	this->~CQtDataBlock();

	std::allocator<char> allocChar;
	allocChar.deallocate(static_cast<char*>(static_cast<void*>(this)), dwLen);
}
