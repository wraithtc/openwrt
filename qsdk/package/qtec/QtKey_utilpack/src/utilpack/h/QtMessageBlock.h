/*------------------------------------------------------*/
/* Message blocks                                       */
/*                                                      */
/* QtMessageBlock.h                                     */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    zhubin (zhubin@qtec.cn)                           */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/

#ifndef QTMESSAGEBLOCK_H
#define QTMESSAGEBLOCK_H

#include "QtReferenceControl.h"
#include "QtStdCpp.h"
#include "QtDebug.h"
#include "QtDataBlock.h"

/**
 * The concept of <CQtMessageBlock> is mainly copyed by <ACE_Message_Block>
 * http://www.cs.wustl.edu/~schmidt/ACE.html
 *
 * An <CQtMessageBlock> is modeled after the message data
 * structures used in System V STREAMS.  Its purpose is to
 * enable efficient manipulation of arbitrarily-large messages
 * without incurring much memory copying overhead.  Here are the
 * main characteristics of an <CQtMessageBlock>:
 * 1. Contains a pointer to a reference-counted
 *    <CQtDataBlock>, which in turn points to the actual data
 *    buffer.  This allows very flexible and efficient sharing of
 *    data by multiple <CQtMessageBlock>s.
 * 2. One or more <CQtMessageBlock> can be linked to form a
 *    'fragment chain.'
 *
 *	The internal structure of <CQtMessageBlock>:
 *     |<----------------------(Chained CQtMessageBlock)---------------------->|
 *     |<---(TopLevel CQtMessageBlock)--->|           | (Next CQtMessageBlock) |
 *     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx (m_pNext) xxxxxxxxxxxxxxxxxxxxxxxxxx
 *     ^           ^          ^           ^---------->^
 *     |           |          |           |
 *     m_pBeginPtr m_pReadPtr m_pWritePtr m_pEndPtr 
 *                 |<-Length->|<--Space-->|
 *     |<------Capacity------------------>|
 *
 *  If <m_Flag> is set with <DONT_DELETE>, <m_pBeginPtr> and <m_pEndPtr> are  
 *  pointing to the actual data while <m_pDataBlock> is NULL.
 */
class QT_OS_EXPORT CQtMessageBlock 
{
public:
	enum 
	{
		/// Don't delete the data on exit since we don't own it.
		DONT_DELETE = 1 << 0,

		/// Malloc and copy data internally
		MALLOC_AND_COPY = 1 << 1,

		/// Can't read/write.
		READ_LOCKED = 1 << 8,
		WRITE_LOCKED = 1 << 9,

		/// the following flags are only for internal purpose.
		INTERNAL_MASK = 0xFF00,
		DUPLICATED = 1 << 17
	};
	typedef unsigned long MFlag;
	
	/**
	 * Create an initialized message containing <aSize> bytes. 
	 * AdvanceTopLevelWritePtr for <aAdvanceWritePtrSize> bytes.
	 * If <aData> == 0 then we create and own the <aData>;
	 * If <aData> != 0 
	 *   If <aFlag> == DONT_DELETE, do nothing when destruction;
	 *   Else delete it when destruction;
	 */
	explicit CQtMessageBlock(
		DWORD aSize, 
		LPCSTR aData = NULL, 
		MFlag aFlag = 0, 
		DWORD aAdvanceWritePtrSize = 0);
	
	explicit CQtMessageBlock(CQtDataBlock *aDb, MFlag aFlag = 0);

//	~CQtMessageBlock();

	/// Read <aCount> bytes, advance it if <aAdvance> is TRUE,
	/// if <aDst> != NULL, copy data into it.
	QtResult Read(LPVOID aDst, DWORD aCount, DWORD *aBytesRead = NULL, BOOL aAdvance = TRUE);
	
	/// Write and advance <aCount> bytes from <aSrc> to the top-level <CQtMessageBlock>
	QtResult Write(LPCVOID aSrc, DWORD aCount, DWORD *aBytesWritten = NULL);

	/// Get the length of the <CQtMessageBlock>s, including chained <CQtMessageBlock>s.
	DWORD GetChainedLength() const ;
	
	/// Get the space of the <CQtMessageBlock>s, including chained <CQtMessageBlock>s.
	DWORD GetChainedSpace() const ;

	// Don't export <CQtDataBlock> because it maybe realloc when AddReference(),
	// the Base() is not the same as previous one.
	//CQtDataBlock* GetDataBlock();

	// Append <aMb> to the end chain.
	void Append(CQtMessageBlock *aMb);

	// Get the next <CQtMessageBlock>
	CQtMessageBlock* GetNext();

	/// Advance <aCount> bytes for reading in chained <CQtMessageBlock>s.
	QtResult AdvanceChainedReadPtr(DWORD aCount, DWORD *aBytesRead = NULL);

	/// Advance <aCount> bytes for writing in chained <CQtMessageBlock>s.
	/// <CQtMessageBlock> must never be read before, and could write continually.
	QtResult AdvanceChainedWritePtr(DWORD aCount, DWORD *aBytesWritten = NULL);

	/// Return a "shallow" copy that not memcpy actual data buffer.
	/// Use DestroyChained() to delete the return <CQtMessageBlock>.
	CQtMessageBlock* DuplicateChained();

	/// Disjoint the chained <CQtMessageBlock>s at the start point <aStart>.
	/// return the new <CQtMessageBlock> that advanced <aStart> read bytes from the old.
	/// <aStart> must be less than ChainedLength.
	/// Use DestroyChained() to delete the return <CQtMessageBlock>.
	CQtMessageBlock* Disjoint(DWORD aStart);

	/**
	 * Decrease the shared CQtDataBlock's reference count by 1.  If the
	 * CQtDataBlock's reference count goes to 0, it is deleted.
	 * In all cases, this CQtMessageBlock is deleted - it must have come
	 * from the heap, or there will be trouble.
	 *
	 * DestroyChained() is designed to DestroyChained the continuation chain; the
	 * destructor is not.  If we make the destructor DestroyChained the
	 * continuation chain by calling DestroyChained() or delete on the message
	 * blocks in the continuation chain, the following code will not
	 * work since the message block in the continuation chain is not off
	 * the heap:
	 *
	 *  CQtMessageBlock mb1(1024);
	 *  CQtMessageBlock mb2(1024);
	 *
	 *  mb1.SetNext(&mb2);
	 *
	 * And hence, call DestroyChained() on a dynamically allocated message
	 * block. This will DestroyChained all the message blocks in the
	 * continuation chain.  If you call delete or let the message block
	 * fall off the stack, cleanup of the message blocks in the
	 * continuation chain becomes the responsibility of the user.
	 *
	 * None <CQtMessageBlock> but returned by DuplicateChained() or 
	 * Disjoint() could be destroyed by this DestroyChained().
	 */
	void DestroyChained();

	/// Fill iovec to make socket read/write effectively.
	DWORD FillIov(iovec aIov[], DWORD aMax) const;

	/// For enhanced checking, mainly for debug purpose.
	void LockReading();
	void LockWriting();

	/// Destory chained <CQtMessageBlock>s whose length is 0.
	/// return the number of destroyed <CQtMessageBlock>.
	CQtMessageBlock* ReclaimGarbage();

	/// Copy all chained data.
	CQtString FlattenChained();

public:
	/// Get <m_pReadPtr> of the top-level <CQtMessageBlock>
	LPCSTR GetTopLevelReadPtr() const ;
	/// Advance <aStep> bytes from <m_pReadPtr> of the top-level <CQtMessageBlock>
	QtResult AdvanceTopLevelReadPtr(DWORD aStep);

	/// Get <m_pWritePtr> of the top-level <CQtMessageBlock>
	LPSTR GetTopLevelWritePtr() const ;
	/// Advance <aStep> bytes from <m_pWritePtr> of the top-level <CQtMessageBlock>
	QtResult AdvanceTopLevelWritePtr(DWORD aStep);

	/// Message length is (<m_pWritePtr> - <m_pReadPtr>).
	/// Get the length in the top-level <CQtMessageBlock>.
	DWORD GetTopLevelLength() const ;

	/// Get the number of bytes available after the <m_pWritePtr> in the top-level <CQtMessageBlock>.
	DWORD GetTopLevelSpace() const ;

	/// Rewind <m_pReadPtr> of chained <CQtMessageBlock>s to their beginnings,
	/// It's not safe because it don't record first read ptr if it not equals <m_pBeginPtr>.
	void RewindChained();

	/// Return a "shallow" copy of the top-level <CQtMessageBlock>,
	/// if flag set DONT_DELETE, malloc and memcpy actual data,
	/// else just does CQtDataBlock::AddReference().
	CQtMessageBlock* DuplicateTopLevel() const ;

	//update the data that store in MessageBlock, 10/13 2007
	/*
	 *	@ pSrc Input, the data that we want update into the MessageBlock
	 *  @ nSrcLen Input, the data length that we want upate into the MessageBlock
	 *  @ nOffset Input, the offset of the data we want update
	 *  @ return, QT_OK is successful, otherwise is failure
	*/
	QtResult Update(const BYTE *pSrc, DWORD nSrcLen, DWORD nOffset = 0);

	//Peek the data that store in MessageBlock, and it has not change the message block, 10/15 2007
	/*
	 *	@ pPeekBuffer Input/OutPut, the data buffer that we want peek from the MessageBlock
	 *  @ nPeekLen Input, the data length that we want to peek from the MessageBlock, it must big than pPeekBuffer size
	 *  @ nOffset Input, the offset of the data we want to peek
	 *  @ return, QT_OK is successful, otherwise is failure
	*/
	QtResult Peek(BYTE *pPeekBuffer, DWORD nPeekLen, DWORD nOffset = 0);
private:
	void Reset_i(CQtDataBlock *aDb);

private:
	CQtMessageBlock *m_pNext;
	CQtComAutoPtr<CQtDataBlock> m_pDataBlock;
	LPCSTR m_pReadPtr;
	LPSTR m_pWritePtr;
	LPCSTR m_pBeginPtr;
	LPCSTR m_pEndPtr;
	MFlag m_Flag;

private:
	// = Prevent assignment and initialization.
	CQtMessageBlock(const CQtMessageBlock &);
	void operator = (const CQtMessageBlock&);
};


// inline functions.
inline void CQtMessageBlock::Reset_i(CQtDataBlock *aDb)
{
	m_pDataBlock = aDb;
	m_pBeginPtr = m_pDataBlock ? m_pDataBlock->GetBasePtr() : NULL;
	m_pReadPtr = m_pBeginPtr;
	m_pWritePtr = const_cast<LPSTR>(m_pBeginPtr);
	m_pEndPtr = m_pBeginPtr + (m_pDataBlock ? m_pDataBlock->GetLength() : (DWORD)0);
}

inline DWORD CQtMessageBlock::GetTopLevelLength() const
{
	QT_ASSERTE(m_pWritePtr >= m_pReadPtr);
	return m_pWritePtr - m_pReadPtr;
}

inline DWORD CQtMessageBlock::GetTopLevelSpace() const 
{
	QT_ASSERTE(m_pEndPtr >= m_pWritePtr);
	return m_pEndPtr - m_pWritePtr;
}

inline CQtMessageBlock* CQtMessageBlock::GetNext()
{
	return m_pNext;
}

inline LPCSTR CQtMessageBlock::GetTopLevelReadPtr() const 
{
	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, READ_LOCKED));
	return m_pReadPtr;
}

inline LPSTR CQtMessageBlock::GetTopLevelWritePtr() const 
{
	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, WRITE_LOCKED));
	return m_pWritePtr;
}

inline QtResult CQtMessageBlock::AdvanceTopLevelWritePtr(DWORD aStep)
{
	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, WRITE_LOCKED));
	QT_ASSERTE_RETURN(m_pWritePtr + aStep <= m_pEndPtr, QT_ERROR_NOT_AVAILABLE);
	m_pWritePtr += aStep;
	return QT_OK;
}

inline QtResult CQtMessageBlock::AdvanceTopLevelReadPtr(DWORD aStep)
{
	QT_ASSERTE(QT_BIT_DISABLED(m_Flag, READ_LOCKED));
	QT_ASSERTE_RETURN(m_pWritePtr >= m_pReadPtr + aStep, QT_ERROR_NOT_AVAILABLE);
	m_pReadPtr += aStep;
	return QT_OK;
}

inline void CQtMessageBlock::LockReading()
{
	QT_SET_BITS(m_Flag, READ_LOCKED);
}

inline void CQtMessageBlock::LockWriting()
{
	QT_SET_BITS(m_Flag, WRITE_LOCKED);
}


// Design Note:
// 1. <CQtDataBlock>* instead <CQtDataBlock> :
//    <CQtMessageBlock> can DuplicateChained without copying <CQtDataBlock>.
//
// 2. <CQtMessageBlock>* m_pNext instead <CQtDataBlock> list :
//    There is no offset in <CQtDataBlock>.
//
// 3. <CQtMessageBlock> has ReferenceControl :
//    Easy to add in list.
//    Need thread mutex when do ReferenceControl.
//    but how to deal with Func(const CQtMessageBlock &aMb)?
//    we can't invoke Write, Read or Advance in Func().
//    so we have to add a function DuplicateChained?
//
// 4. SendData(const CQtMessageBlock &aMb);
//    How do do ReferenceControl?
//
// 5. OnReceive(CQtMessageBlock &aMb);

#endif // !QTMESSAGEBLOCK_H
