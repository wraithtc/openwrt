
USER_DEPTH = ../..

include $(USER_DEPTH)/config/makeinclude/env.make
include $(USER_DEPTH)/config/makeinclude/platform.make

USER_DLL = keydistribute

USER_SRCS = \
			QtKeyDistribute.cpp \
			QtKeyTool.cpp

USER_INCLDIRS = -I../../include/ \
		-I./../include \
		-I./../../QtkeyMangent/include \
		-I./../../QtKeyEncrypt/include \
		-I../../../../QtKey_utilpack/src/utilpack/h \
		-I../../../../../staging_dir/target-arm_cortex-a7_uClibc-1.0.14_eabi/root-ipq806x/usr/include

USER_LINK_DLLS = -Lqtmangent -lqtencrypt -lutilpack

USER_OBJ_DIR = ../../../build/KeyDistr

USER_DLL_DIR = ../../libs

optimize = 1
OPTFLAGS=-fPIC

include $(USER_DEPTH)/config/makeinclude/all_in_one.make
