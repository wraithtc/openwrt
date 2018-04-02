
USER_DEPTH = ../../../../

include $(USER_DEPTH)/config/makeinclude/env.make
include $(USER_DEPTH)/config/makeinclude/platform.make


USER_BIN = keydistribute_test

USER_SRCS = \
	QtKeyDistribute_test.cpp

USER_INCLDIRS = -I../../../include/ \
				-I../include/ \
				-I./../../QtkeyMangent/include \
				-I../../../../utilpack/h/ 

USER_LINK_DLLS = -lqtmangent -lsqlite3 -lutilpack

USER_LINK_LIBS = \
				$(USER_DEPTH)/utilpack/libutilpack.a \
				$(USER_DEPTH)/bin/libs/libqtmangent.a \
				$(USER_DEPTH)/bin/libs/libkeydistribute.a 

USER_OBJ_DIR = .

USER_BIN_DIR = .

optimize = 1
OPTFLAGS=$(USER_INCLDIRS)  -L../../../../utilpack/ -L../../../../bin/libs

include $(USER_DEPTH)/config/makeinclude/all_in_one.make
