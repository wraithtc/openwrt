USER_DEPTH = ../..

include $(USER_DEPTH)/config/makeinclude/env.make
include $(USER_DEPTH)/config/makeinclude/platform.make

USER_DLL = qtmangent

USER_SRCS = \
	QtkeyMangent_common.cpp \
	QtkeyMangent_raw.cpp \
	QtkeyMangent_syn.cpp \
	QtkeyMangent.cpp 

USER_INCLDIRS = -I../../include/ \
		-I./../include \
		-I./../../QtKeyEncrypt/include \
		-I../../../../QtKey_utilpack/src/utilpack/h \
		-I../../../../../staging_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/root-ipq806x/usr/include

USER_LINK_DLLS = -lsqlite3 -lqtencrypt -lutilpack

USER_OBJ_DIR = ../../../build/QtkeyMangent
USER_DLL_DIR = ../../libs

optimize = 1
OPTFLAGS=-fPIC

include $(USER_DEPTH)/config/makeinclude/all_in_one.make
