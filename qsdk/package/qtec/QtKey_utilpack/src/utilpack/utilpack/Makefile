
USER_DEPTH = ../..

include $(USER_DEPTH)/config/makeinclude/env.make
include $(USER_DEPTH)/config/makeinclude/platform.make

#USER_DLL = utilpack

USER_LIB = utilpack

USER_SRCS = \
	QtHashMapT.cpp\
	QtAcceptorBase.cpp \
	QtAcceptorTcp.cpp \
	QtAcceptorThreadProxy.cpp \
	QtAcceptorUdp.cpp \
	QtAtomicOperationT.cpp \
	QtConfigInitFile.cpp \
	QtConditionVariable.cpp \
	QtConnectionInterface.cpp \
	QtConnectorThreadProxy.cpp \
	QtConnectorWrapper.cpp \
	QtDataBlock.cpp \
	QtEventQueueBase.cpp \
	QtInetAddr.cpp \
	QtMessageBlock.cpp \
	QtMutex.cpp \
	QtPipe.cpp \
	QtReactorBase.cpp \
	QtReactorInterface.cpp \
	QtReactorNotifyPipe.cpp \
	QtReactorRealTimeSignal.cpp \
	QtReactorThreadProxy.cpp \
	QtReactorSelect.cpp \
	QtSocket.cpp \
	QtSessionInterface.cpp \
	QtSessionOneForwardMany.cpp \
	QtThread.cpp \
	QtThreadInterface.cpp \
	QtThreadManager.cpp \
	QtThreadProxyManager.cpp \
	QtThreadReactor.cpp \
	QtThreadTask.cpp \
	QtTimerQueueBase.cpp \
	QtTimerQueueOrderedList.cpp \
	QtTimerQueueCalendar.cpp \
	QtTimerWrapperID.cpp \
	QtTimeValue.cpp \
	QtTraceFromT120.cpp \
	QtTransportBase.cpp \
	QtTransportTcp.cpp \
	QtTransportThreadProxy.cpp \
	QtTransportUdp.cpp \
	QtUtilClasses.cpp \
	QtDetectionConnector.cpp \
	QtsConnectionAdapter.cpp \
	QtsPacketConnection.cpp \
	QtsReconn.cpp \
	QtsRlb.cpp \
	QtsBase.cpp \
	QtsPdu.cpp \
	QtsPkg.cpp \
	QtsPkgSender.cpp \
	QtsRlbTcp.cpp \
	QtsLenPkg.cpp \
	QtsSendBuf.cpp \
	paramlist.cpp \
	QtTimerPreciseWrapperID.cpp \
	timer.cpp \
	QtDnsManager.cpp


ifeq ($(QTEC_HAVE_EPOLL), 1)
	USER_SRCS += QtReactorEpoll.cpp
endif # QTEC_HAVE_EPOLL

ifeq ($(MMP_OCTEON), 1)
	ifeq ($(LIBDIR), octeon))
		USER_INCLDIRS += -I../h -I$(OCTEON_BUILD_ROOT)/services/buildroot/cge/devkit/mips64/octeon_v2_be/target/usr/include
	else
     		USER_INCLDIRS += -I../h -I../h/openssl-0.9.7e/include
	endif
else
USER_INCLDIRS = -I../h -I../h/openssl-0.9.7e/include
endif

ifeq (1 ,$(streaming_server))
USER_DEFFLAGS = -DQT_SUPPORT_T120_UTIL -DQT_DISABLE_EVENT_REPORT -D__STREAMING_SERVER__
else
USER_DEFFLAGS = -DQT_SUPPORT_T120_UTIL -DQT_DISABLE_EVENT_REPORT 
endif

ifeq ($(QTEC_HAVE_EPOLL), 1)
	USER_DEFFLAGS += -DQT_ENABLE_EPOLL
endif # QTEC_HAVE_EPOLL

USER_LINK_LIBS =

USER_OBJ_DIR = $(USER_DEPTH)/bin/platform/utiltp

USER_LIB_DIR = $(USER_DEPTH)/utilpack

include $(USER_DEPTH)/config/makeinclude/all_in_one.make
