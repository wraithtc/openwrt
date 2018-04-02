/*------------------------------------------------------*/
/* Provides a bidirectional "pipe"                      */
/*                                                      */
/* QtPipe.h                                             */
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
#ifndef QTPIPE_H
#define QTPIPE_H

#define QT_DEFAULT_MAX_SOCKET_BUFSIZ 65535

class CQtPipe  
{
public:
	CQtPipe();
	~CQtPipe();

	QtResult Open(DWORD aSize = QT_DEFAULT_MAX_SOCKET_BUFSIZ);
	QtResult Close();

	QT_HANDLE GetReadHandle() const;
	QT_HANDLE GetWriteHandle() const;

private:
	QT_HANDLE m_Handles[2];
};

#endif // !QTPIPE_H
