
USER_DEPTH = ../..

include $(USER_DEPTH)/config/makeinclude/env.make
include $(USER_DEPTH)/config/makeinclude/platform.make


USER_DLL = qtencrypt

USER_SRCS = \
	QtKeyEncrypt.cpp \
	sm4.cpp 


USER_INCLDIRS = -I../../include/ \
		-I./../include \
		-I../../../../QtKey_utilpack/src/utilpack/h \
		-I../../../libqtkey/utilpack/h 
USER_LINK_DLLS = -lutilpack 

USER_OBJ_DIR = ../../../build/QtKeyEncrypt
USER_DLL_DIR = ../../libs

optimize = 1
OPTFLAGS=-fPIC

include $(USER_DEPTH)/config/makeinclude/all_in_one.make
