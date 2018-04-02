/*------------------------------------------------------*/
/* Data Block interface and classes                     */
/*                                                      */
/* QtDataBlock.h                                        */
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

#ifndef QTDATABLOCK_H
#define QTDATABLOCK_H

#include "QtReferenceControl.h"
#include "QtStdCpp.h"

/**
 * The concept of <CQtDataBlock> is mainly copyed by <ACE_Data_Block>
 * http://www.cs.wustl.edu/~schmidt/ACE.html
 *
 * @brief Stores the data payload that is accessed via one or more
 * <CQtMessageBlock>s.
 *
 * This data structure is reference counted to maximize sharing.
 * memory pool is used to allocate the memory.
 * Only allocate once including <CQtDataBlock> and size of buffer.
 *
 * The internal structure of <CQtDataBlock>:
 *              ------------
 *              | m_dwSize |
 *           -----m_pData  |
 *           |  |----------|
 *           -->| (buffer) |
 *              |          |
 *              ------------
 */
class QT_OS_EXPORT CQtDataBlock : public CQtReferenceControlMutilThread
{
public:
	// Malloc a buffer with <aSize> and copy <aData> into it.
	static QtResult CreateInstance(CQtDataBlock *&aDb, DWORD aSize, LPCSTR aData = NULL);

	// interface CQtReferenceControlMutilThread.
	virtual void OnReferenceDestory();

	LPSTR GetBasePtr() const 
	{
		return m_pData;
	}

	DWORD GetLength() const 
	{
		return m_dwSize;
	}

	DWORD GetCapacity() const 
	{
		return m_dwSize;
	}

private:
	DWORD m_dwSize;
	LPSTR m_pData;

private:
	// = Prevent assignment and initialization.
	void operator = (const CQtDataBlock&);
	CQtDataBlock(const CQtDataBlock&);

	// Make constructor and Destructor functions private,
	// so that we will do our memery allocation.
	CQtDataBlock(DWORD aSize, LPSTR aData = NULL);
	virtual ~CQtDataBlock();
};

#endif // !QTDATABLOCK_H
