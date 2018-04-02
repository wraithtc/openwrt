
#include "QtBase.h"
#include "QtMessageBlock.h"
#include "QtDataBlock.h"
/*
That operator= has higher performance than memcpy, 
so we need replace the memcpy with operator= if the length of the target is same as base type, 
such as char/byte, short, long and int
*/

#ifdef QT_DEBUG
	#define SELFCHECK_MessageBlock(pmb) \
	do { \
		QT_ASSERTE(pmb->m_pBeginPtr <= pmb->m_pReadPtr); \
		QT_ASSERTE(pmb->m_pReadPtr <= pmb->m_pWritePtr); \
		QT_ASSERTE(pmb->m_pWritePtr <= pmb->m_pEndPtr); \
	} while (0)
#else
	#define SELFCHECK_MessageBlock(pmb)
#endif // QT_DEBUG

CQtMessageBlock::
CQtMessageBlock(DWORD aSize, LPCSTR aData, MFlag aFlag, DWORD aAdvanceWritePtrSize)
	: m_pNext(NULL)
	, m_Flag(0)
{
	if (aData && QT_BIT_DISABLED(aFlag, CQtMessageBlock::MALLOC_AND_COPY)) {
		QT_SET_BITS(aFlag, CQtMessageBlock::DONT_DELETE);
		m_pBeginPtr = aData;
		m_pReadPtr = m_pBeginPtr;
		m_pWritePtr = const_cast<LPSTR>(m_pBeginPtr);
		m_pEndPtr = aData + aSize;
	}
	else {
#ifdef QT_DEBUG
		if (aData)
			QT_ASSERTE(QT_BIT_DISABLED(aFlag, CQtMessageBlock::DONT_DELETE));
#endif // QT_DEBUG
		QT_CLR_BITS(aFlag, CQtMessageBlock::DONT_DELETE);
		if (aSize > 0)
			//CQtDataBlock::CreateInstance(m_pDataBlock.ParaOut(), aSize);
		Reset_i(m_pDataBlock.ParaIn());
	}
	
	if (aAdvanceWritePtrSize > 0)
		AdvanceTopLevelWritePtr(aAdvanceWritePtrSize);
	
	m_Flag = aFlag;
	QT_CLR_BITS(m_Flag, CQtMessageBlock::MALLOC_AND_COPY);
	QT_CLR_BITS(m_Flag, CQtMessageBlock::INTERNAL_MASK);
}

CQtMessageBlock::CQtMessageBlock(CQtDataBlock *aDb, MFlag aFlag)
	: m_pNext(NULL)
{
	QT_ASSERTE(QT_BIT_DISABLED(aFlag, CQtMessageBlock::DONT_DELETE));
	QT_CLR_BITS(aFlag, CQtMessageBlock::DONT_DELETE);
	
	Reset_i(aDb);

	m_Flag = aFlag;
	QT_CLR_BITS(m_Flag, CQtMessageBlock::MALLOC_AND_COPY);
	QT_CLR_BITS(m_Flag, CQtMessageBlock::INTERNAL_MASK);
}

#if 0
CQtMessageBlock::CQtMessageBlock(const CQtMessageBlock &aCopy)
	: m_pNext(NULL)
	, m_pReadPtr(NULL)
	, m_pWritePtr(NULL)
	, m_pBeginPtr(NULL)
	, m_pEndPtr(NULL)
	, m_Flag(0)
{
	(*this) = aCopy;
}

void CQtMessageBlock::operator = (const CQtMessageBlock &aCopy)
{
	// We can't malloc m_pNext in this assign operator.
	QT_ASSERTE(!aCopy.m_pNext);
	m_pNext = NULL;

	Reset_i(aCopy.m_pDataBlock.ParaIn());
	m_pReadPtr += aCopy.m_pReadPtr - aCopy.m_pBeginPtr;
	m_pWritePtr += aCopy.m_pWritePtr - aCopy.m_pBeginPtr;
	m_Flag = aCopy.m_Flag;
	SELFCHECK_MessageBlock(this);
}
#endif

QtResult CQtMessageBlock::
Read(LPVOID aDst, DWORD aCount, DWORD *aBytesRead, BOOL aAdvance) 
{
	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, READ_LOCKED));
	DWORD dwLen = GetTopLevelLength();
	DWORD dwHaveRead = 0;
	
	QT_ASSERTE(m_pWritePtr >= m_pReadPtr);
	if (dwLen >= aCount) {
		if (aDst)
		{
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
			if(sizeof(char) == aCount)
				*(char *)((LPSTR)aDst+dwHaveRead) = *(char *)m_pReadPtr;
			else if(sizeof(short) == aCount)
				*(short *)((LPSTR)aDst+dwHaveRead) = *(short *)m_pReadPtr;
			else if(sizeof(long) == aCount)
				*(long *)((LPSTR)aDst+dwHaveRead) = *(long *)m_pReadPtr;
			else
#endif
			::memcpy((LPSTR)aDst+dwHaveRead, m_pReadPtr, aCount);
		}
		dwHaveRead += aCount;
		if (aAdvance) {
			m_pReadPtr += aCount;
			QT_ASSERTE(m_pReadPtr <= m_pWritePtr);
		}
		if (aBytesRead)
			*aBytesRead = dwHaveRead;
		return QT_OK;
	}
	else {
		if (aDst)
		{
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
			if(sizeof(char) == dwLen)
				*(char *)((LPSTR)aDst+dwHaveRead) = *(char *)m_pReadPtr;
			else if(sizeof(short) == dwLen)
				*(short *)((LPSTR)aDst+dwHaveRead) = *(short *)m_pReadPtr;
			else if(sizeof(long) == dwLen)
				*(long *)((LPSTR)aDst+dwHaveRead) = *(long *)m_pReadPtr;
			else
#endif
			::memcpy((LPSTR)aDst+dwHaveRead, m_pReadPtr, dwLen);
		}
		dwHaveRead += dwLen;
		if (aAdvance) {
			m_pReadPtr += dwLen;
			QT_ASSERTE(m_pReadPtr == m_pWritePtr);
		}
		if (m_pNext) {
			DWORD dwNextRead;
			QtResult rv = m_pNext->Read(
				aDst ? (LPSTR)aDst+dwHaveRead : aDst, 
				aCount-dwLen, 
				&dwNextRead, 
				aAdvance);
			dwHaveRead += dwNextRead;
			if (aBytesRead)
				*aBytesRead = dwHaveRead;
			return rv;
		}
		else {
			if (aBytesRead)
				*aBytesRead = dwHaveRead;
			return QT_ERROR_PARTIAL_DATA;
		}
	}
}

QtResult CQtMessageBlock::
Write(LPCVOID aSrc, DWORD aCount, DWORD *aBytesWritten)
{
	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, WRITE_LOCKED));
	DWORD dwSpace = GetTopLevelSpace();
	DWORD dwHaveWritten = 0;

	if (dwSpace >= aCount) {
		dwHaveWritten = aCount;
		if (aSrc)
		{
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
			if(sizeof(char) == aCount)
				*(char *)(m_pWritePtr) = *(char *)aSrc;
			else if(sizeof(short) == aCount)
				*(short *)(m_pWritePtr) = *(short *)aSrc;
			else if(sizeof(long) == aCount)
				*(long *)(m_pWritePtr) = *(long *)aSrc;
			else
#endif
			::memcpy(m_pWritePtr, aSrc, aCount);
		}
		m_pWritePtr += aCount;
		if (aBytesWritten)
			*aBytesWritten = dwHaveWritten;
		return QT_OK;
	}
	else {
		dwHaveWritten = dwSpace;
		if (aSrc)
		{
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
			if(sizeof(char) == dwSpace)
				*(char *)(m_pWritePtr) = *(char *)aSrc;
			else if(sizeof(short) == dwSpace)
				*(short *)(m_pWritePtr) = *(short *)aSrc;
			else if(sizeof(long) == dwSpace)
				*(long *)(m_pWritePtr) = *(long *)aSrc;
			else
#endif
				::memcpy(m_pWritePtr, aSrc, dwSpace);
		}
		m_pWritePtr += dwSpace;
		QT_ASSERTE(m_pWritePtr == m_pEndPtr);
		if (m_pNext) {
			DWORD dwNextWrite;
			QtResult rv = m_pNext->Write(
				(LPSTR)aSrc + dwHaveWritten, 
				aCount-dwHaveWritten, 
				&dwNextWrite);
			dwHaveWritten += dwNextWrite;
			if (aBytesWritten)
				*aBytesWritten = dwHaveWritten;
			return rv;
		}
		else if (aBytesWritten)
			*aBytesWritten = dwHaveWritten;
		return QT_ERROR_PARTIAL_DATA;
	}
}

DWORD CQtMessageBlock::GetChainedLength() const
{
	DWORD dwRet = 0;
	for (const CQtMessageBlock *i = this; NULL != i; i = i->m_pNext)
		dwRet += i->GetTopLevelLength();
	return dwRet;
}

DWORD CQtMessageBlock::GetChainedSpace() const
{
	DWORD dwRet = 0;
	for (const CQtMessageBlock *i = this; NULL != i; i = i->m_pNext)
		dwRet += i->GetTopLevelSpace();
	return dwRet;
}

void CQtMessageBlock::Append(CQtMessageBlock *aMb)
{
#if 1
	CQtMessageBlock *pMbMove = this;
	while (pMbMove) {
		QT_ASSERTE(aMb != pMbMove);
		if (pMbMove->m_pNext)
			pMbMove = pMbMove->m_pNext;
		else {
			pMbMove->m_pNext = aMb;
			return;
		}
	}

#else // Don't use recursion to avoid stack overflow
	QT_ASSERTE(aMb != this);
	if (m_pNext) {
		m_pNext->Append(aMb);
		return;
	}
	else {
		m_pNext = aMb;
	}
#endif // 1
}

QtResult CQtMessageBlock::AdvanceChainedReadPtr(DWORD aCount, DWORD *aBytesRead)
{
//	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, READ_LOCKED));
	QtResult rv = Read(NULL, aCount, aBytesRead);
	return rv;
}

QtResult CQtMessageBlock::AdvanceChainedWritePtr(DWORD aCount, DWORD *aBytesWritten)
{
	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, WRITE_LOCKED));
	DWORD dwNeedWrite = aCount;
	for (CQtMessageBlock *pCurrent = this; 
		 NULL != pCurrent && dwNeedWrite > 0; 
		 pCurrent = pCurrent->m_pNext) 
	{
		QT_ASSERTE(pCurrent->m_pBeginPtr == pCurrent->m_pReadPtr);
		if (pCurrent->m_pBeginPtr != pCurrent->m_pReadPtr) {
			QT_ERROR_TRACE_THIS("CQtMessageBlock::AdvanceChainedWritePtr, can't advance."
				" m_pBeginPtr=" << (LPVOID)pCurrent->m_pBeginPtr <<
				" m_pReadPtr=" << (LPVOID)pCurrent->m_pReadPtr);
			if (aBytesWritten)
				*aBytesWritten = aCount - dwNeedWrite;
			return QT_ERROR_PARTIAL_DATA;
		}

		DWORD dwLen = pCurrent->GetTopLevelSpace();
		if (dwNeedWrite <= dwLen) {
			pCurrent->AdvanceTopLevelWritePtr(dwNeedWrite);
			if (aBytesWritten)
				*aBytesWritten = aCount;
			return QT_OK;
		}
		else {
			dwNeedWrite -= dwLen;
			pCurrent->AdvanceTopLevelWritePtr(dwLen);
		}
	}

	QT_ASSERTE(aCount > dwNeedWrite);
	if (aBytesWritten)
		*aBytesWritten = aCount - dwNeedWrite;
	return QT_ERROR_PARTIAL_DATA;
}

CQtMessageBlock* CQtMessageBlock::DuplicateChained()
{
	
/*
#if 1
	// change, lets all data in one data block
	DWORD nDataLen = GetChainedLength();
	if(!nDataLen) //have no data 
		return NULL;

	char *pBuff = new char[GetChainedLength()];
	char *pPostion = pBuff;
	QT_ASSERTE_RETURN(pBuff, NULL); //out of memory

	DWORD nTopLen = GetTopLevelLength();
	CQtMessageBlock *pMB = this;
	while(pMB)
	{
		memcpy(pPostion, pMB->GetTopLevelReadPtr(), nTopLen);
		pPostion += nTopLen;

		pMB = pMB->GetNext();
		if(!pMB) //got end
			break;
		nTopLen = pMB->GetTopLevelLength();
	}
	return new CQtMessageBlock(nDataLen, pBuff, CQtMessageBlock::DUPLICATED, nDataLen);

#endif
*/

#if 1
	CQtMessageBlock *pRet = NULL;
	CQtMessageBlock *pNewMove = NULL;
	CQtMessageBlock *pOldMove = this;

	while (pOldMove) {
		CQtMessageBlock *pDuplicate = pOldMove->DuplicateTopLevel();
		if (!pDuplicate) {
			if (pRet)
				pRet->DestroyChained();
			return NULL;
		}

		if (!pRet) {
			pRet = pDuplicate;
			QT_ASSERTE(!pNewMove);
			pNewMove = pDuplicate;
		}
		else {
			QT_ASSERTE(pNewMove);
			pNewMove->m_pNext = pDuplicate;
			pNewMove = pDuplicate;
		}

		pOldMove = pOldMove->m_pNext;
	}
	return pRet;

#else // Don't use recursion to avoid stack overflow
	CQtMessageBlock *pRet = DuplicateTopLevel();
	if (pRet) {
		if (m_pNext) {
			pRet->m_pNext = m_pNext->DuplicateChained();
			if (!pRet->m_pNext) {
				pRet->DestroyChained();
				return NULL;
			}
		}
	}
	return pRet;
#endif // 1
}

CQtMessageBlock* CQtMessageBlock::DuplicateTopLevel() const 
{
	CQtMessageBlock *pRet = NULL;
	if (QT_BIT_ENABLED(m_Flag, CQtMessageBlock::DONT_DELETE)) {
		// <m_pBeginPtr> and <m_pEndPtr> are pointing to the actual data,  
		// and m_pDataBlock is NULL.
		QT_ASSERTE(!m_pDataBlock);
		
		DWORD dwLen = m_pEndPtr - m_pBeginPtr;
		MFlag flagNew = m_Flag;
		QT_CLR_BITS(flagNew, CQtMessageBlock::DONT_DELETE);
		QT_SET_BITS(flagNew, CQtMessageBlock::MALLOC_AND_COPY);
		pRet = new CQtMessageBlock(
			dwLen, 
			m_pBeginPtr, 
			flagNew);

		if (pRet && dwLen)
			::memcpy(pRet->GetTopLevelWritePtr(), m_pBeginPtr, dwLen);
	}
	else {
		pRet = new CQtMessageBlock(m_pDataBlock.ParaIn(), m_Flag);
	}

	if (pRet) {
		/// <CQtDataBlock> maybe realloc if DONT_DELETE,
		/// so can't do "pRet->m_pReadPtr = m_pReadPtr;"
		pRet->m_pReadPtr += m_pReadPtr - m_pBeginPtr;
		pRet->m_pWritePtr += m_pWritePtr - m_pBeginPtr;
		QT_SET_BITS(pRet->m_Flag, DUPLICATED);
		SELFCHECK_MessageBlock(pRet);
	}
	return pRet;
}

CQtMessageBlock* CQtMessageBlock::Disjoint(DWORD aStart)
{
	QT_ASSERTE(aStart <= GetChainedLength());

	// find the start point of disjointing.
	CQtMessageBlock *pFind = NULL;
	for (CQtMessageBlock *pCurrent = this; pCurrent; pCurrent = pCurrent->m_pNext) 
	{
		DWORD dwLen = pCurrent->GetTopLevelLength();
		if (aStart == 0 && dwLen == 0) {
		}
		else if (aStart == dwLen) {
			pFind = pCurrent->m_pNext;
			pCurrent->m_pNext = NULL;
			break;
		}
		else if (aStart < dwLen) {
			pFind = pCurrent->DuplicateTopLevel();
			if (pFind) {
				pFind->m_pNext = pCurrent->m_pNext;
				pFind->m_pReadPtr += aStart;
				SELFCHECK_MessageBlock(pFind);

				pCurrent->m_pWritePtr -= dwLen - aStart;
				pCurrent->m_pNext = NULL;
				SELFCHECK_MessageBlock(pCurrent);
			}
			break;
		}
		else {
			aStart -= dwLen;
		}
	}
	
	// duplicate from <pFind>, if is DUPLICATED, need not duplicate again.
	CQtMessageBlock *pPrevious = NULL;
	CQtMessageBlock *pMove = pFind;
	while (pMove) {
		if (QT_BIT_DISABLED(pMove->m_Flag, DUPLICATED)) {
			QT_WARNING_TRACE_THIS("CQtMessageBlock::Disjoint, there are not DUPLICATED "
				"blocks behind the disjointed block.");
			CQtMessageBlock *pNew = pMove->DuplicateTopLevel();
			if (!pNew) {
				// the best way is rollback to destroy what duplicated in this function.
				// but duplicate failed will rare happen.
				return NULL;
			}
			if (pFind == pMove) {
				// the <pFind> is not DUPLICATED, replace it
				pFind = pNew;
			}
			else if (pPrevious) {
				QT_ASSERTE(pPrevious->m_pNext == pMove);
				pPrevious->m_pNext = pNew;
			}
			pNew->m_pNext = pMove->m_pNext;
			pMove->m_pNext = NULL;
			pMove = pNew->m_pNext;
			pPrevious = pNew;
		}
		else {
			pPrevious = pMove;
			pMove = pMove->m_pNext;
		}
	}
	return pFind;
}

void CQtMessageBlock::DestroyChained()
{
#if 1
	CQtMessageBlock *pMbMove = this;
	while (pMbMove) {
		QT_ASSERTE(QT_BIT_ENABLED(pMbMove->m_Flag, DUPLICATED));
		if (QT_BIT_DISABLED(pMbMove->m_Flag, DUPLICATED))
			continue;

		CQtMessageBlock *pTmp = pMbMove->m_pNext;
		delete pMbMove;
		pMbMove = pTmp;
	}
#else // Don't use recursion to avoid stack overflow
	if (m_pNext)
		m_pNext->DestroyChained();
	m_pNext = NULL;

	QT_ASSERTE_RETURN_VOID(QT_BIT_ENABLED(m_Flag, DUPLICATED));
	delete this;
#endif // 1
}

DWORD CQtMessageBlock::FillIov(iovec aIov[], DWORD aMax) const
{
	DWORD dwFill =0;
	for (const CQtMessageBlock *i = this; 
		 NULL != i && dwFill < aMax; 
		 i = i->m_pNext) 
	{
		DWORD dwLen = i->GetTopLevelLength();
		if (dwLen > 0) {
			aIov[dwFill].iov_base = const_cast<char*>(i->GetTopLevelReadPtr());
			aIov[dwFill].iov_len = dwLen;
			dwFill++;
		}
	}
	return dwFill;
}

void CQtMessageBlock::RewindChained()
{
	// TODO: need record first read ptr
	for (CQtMessageBlock *i = this; NULL != i; i = i->m_pNext) {
		SELFCHECK_MessageBlock(i);
		i->m_pReadPtr = i->m_pBeginPtr;
		i->m_pWritePtr = const_cast<LPSTR>(m_pBeginPtr);
	}
}

CQtMessageBlock* CQtMessageBlock::ReclaimGarbage()
{
	// find the start point of disjointing.
	CQtMessageBlock *pCurrent = this;
	while (pCurrent) 
	{
		DWORD dwLen = pCurrent->GetTopLevelLength();
		if (dwLen == 0) {
			CQtMessageBlock *pTmp = pCurrent->m_pNext;
			if (QT_BIT_ENABLED(pCurrent->m_Flag, DUPLICATED))
				delete pCurrent;
			pCurrent = pTmp;
		}
		else {
			return pCurrent;
		}
	}
	return NULL;
}

CQtString CQtMessageBlock::FlattenChained()
{
	CQtString strRet;
	strRet.reserve(GetChainedLength() + 1);

	for (CQtMessageBlock *i = this; NULL != i; i = i->m_pNext)
		strRet.append(
			i->GetTopLevelReadPtr(),
			i->GetTopLevelLength());

	return strRet;
}


QtResult CQtMessageBlock::Update(const BYTE *pSrc, DWORD nSrcLen, DWORD nOffset)
{
	if(0 == nSrcLen)
		return QT_OK;
	QT_ASSERTE_RETURN(pSrc, QT_ERROR_FAILURE);
	QT_ASSERTE_RETURN(nSrcLen + nOffset <= this->GetChainedLength(), QT_ERROR_FAILURE);
	CQtMessageBlock *pMB = this;
	for(;;)
	{
		DWORD nTopLength = pMB->GetTopLevelLength();
		if( nTopLength <= nOffset)
		{
			nOffset -= nTopLength;
		}
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
		else if(sizeof(char) == nSrcLen) //if it is only a byte, avoid memcpy to enhance the performance
		{
			*((char *)pMB->GetTopLevelReadPtr() + nOffset) = *(char *)(pSrc);
			return QT_OK;
		}
		else if(sizeof(short) == nSrcLen && nTopLength >= nOffset + sizeof(short))
		{
			*((short *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset)) = *(short *)(pSrc);
			return QT_OK;
		}
		else if(sizeof(long) == nSrcLen && nTopLength >= nOffset + sizeof(long))
		{
			*((long *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset)) = *(long *)(pSrc);
			return QT_OK;
		}
		else if(sizeof(int) == nSrcLen && nTopLength >= nOffset + sizeof(int))
		{
			*((int *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset)) = *(int *)(pSrc);
			return QT_OK;
		}
#endif
		else if(nTopLength >= nOffset + nSrcLen)
		{
			memcpy((char *)pMB->GetTopLevelReadPtr() + nOffset, pSrc, nSrcLen);
			return QT_OK;
		}
		else
		{
			DWORD nUpdateCount = nTopLength - nOffset;
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
			if(sizeof(char) == nUpdateCount) //if it is only a byte, avoid memcpy to enhance the performance
				*((char *)pMB->GetTopLevelReadPtr() + nOffset )= *(char *)(pSrc);
			else if(sizeof(short) == nUpdateCount)
				*((short *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset))  = *(short *)(pSrc);
			else if(sizeof(long) == nUpdateCount)
				*((long *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset)) = *(long *)(pSrc);
			else if(sizeof(int) == nUpdateCount)
				*((int *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset)) = *(int *)(pSrc);
			else
#endif
				memcpy((char *)pMB->GetTopLevelReadPtr() + nOffset, pSrc, nUpdateCount);
			nSrcLen = ( nOffset + nSrcLen) - nTopLength;
			pSrc += nUpdateCount;
			nOffset = 0;
		}
		pMB = pMB->GetNext();
		QT_ASSERTE_RETURN(pMB, QT_ERROR_FAILURE);
	}
	return QT_ERROR_FAILURE;
}

QtResult CQtMessageBlock::Peek(BYTE *pPeekBuffer, DWORD nPeekLen, DWORD nOffset)
{
	if(0 == nPeekLen)
		return QT_OK;
	QT_ASSERTE_RETURN(pPeekBuffer, QT_ERROR_FAILURE);
	QT_ASSERTE_RETURN(nPeekLen + nOffset <= this->GetChainedLength(), QT_ERROR_FAILURE);
	CQtMessageBlock *pMB = this;
	for(;;)
	{
		DWORD nTopLength = pMB->GetTopLevelLength();
		if( nTopLength <= nOffset)
		{
			nOffset -= nTopLength;
		}
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
		else if(sizeof(char) == nPeekLen)//if it is only a byte, avoid memcpy to enhance the performance
		{
			*pPeekBuffer = *((char *)pMB->GetTopLevelReadPtr() + nOffset);
			return QT_OK;
		}
		else if(sizeof(short) == nPeekLen && nTopLength >= nOffset + sizeof(short))
		{
			*(short *)pPeekBuffer = *((short *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset));
			return QT_OK;
		}
		else if(sizeof(long) == nPeekLen && nTopLength >= nOffset + sizeof(long))
		{
			*(long *)pPeekBuffer = *((long *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset));
			return QT_OK;
		}
		else if(sizeof(int) == nPeekLen && nTopLength >= nOffset + sizeof(int))
		{
			*(int *)pPeekBuffer = *((int *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset));
			return QT_OK;
		}
#endif
		else if(nTopLength >= nOffset + nPeekLen)
		{
			memcpy(pPeekBuffer, (char *)pMB->GetTopLevelReadPtr() + nOffset, nPeekLen);
			return QT_OK;
		}
		else
		{
			DWORD nCopyCount = nTopLength - nOffset;
#if !defined QT_SOLARIS && !defined QT_PORT_CLIENT
			if(sizeof(char) == nCopyCount)
				*pPeekBuffer = *((char *)pMB->GetTopLevelReadPtr() + nOffset);
			else if(sizeof(short) == nCopyCount)
				*(short *)pPeekBuffer = *((short *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset));
			else if(sizeof(long) == nCopyCount)
				*(long *)pPeekBuffer = *((long *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset));
			else if(sizeof(int) == nCopyCount)
				*(int *)pPeekBuffer = *((int *)((LPSTR)pMB->GetTopLevelReadPtr() + nOffset));
			else
#endif
				memcpy(pPeekBuffer, (char *)pMB->GetTopLevelReadPtr() + nOffset, nCopyCount);
			nPeekLen = ( nOffset + nPeekLen) - nTopLength;
			pPeekBuffer += nCopyCount;
			nOffset = 0;
		}
		pMB = pMB->GetNext();
		QT_ASSERTE_RETURN(pMB, QT_ERROR_FAILURE);
	}
	return QT_ERROR_FAILURE;
}
