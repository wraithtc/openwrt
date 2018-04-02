/*------------------------------------------------------*/
/*backup controller notify when primary controller is up*/
/*                                                      */
/*    netscalenotify.h                                  */
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

#ifndef __QTEC_NETSCALER_NOTIFY_H__
#define __QTEC_NETSCALER_NOTIFY_H__

/*
 *	we use netscale to do controller's load balance. if primary controller is down,
 *  then all connnection will connect to backup controller, after this, primary controller
 *  is up, then new connetions will connect to primary controller, in this case, the old
 *  connections will still connect to backup controller, it will cause problem.
 *  so we have to drop all connections in backup controller when primary controller is up.
 */
class INetScalerNotify
{
public:
	//in this API, should close all connections.
	virtual void OnPrimaryUp() = 0;
};

#endif //!__QTEC_NETSCALER_NOTIFY_H__



