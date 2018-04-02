
USER_DEPTH = ../../../../

include $(USER_DEPTH)/config/makeinclude/env.make
include $(USER_DEPTH)/config/makeinclude/platform.make


USER_BIN = keymangent_test

USER_SRCS = \
	KeyMangent_test.cpp

USER_INCLDIRS = -I../../../include/ \
				-I../include/ \
				-I./../../QtKeyEncrypt/include \
				-I../../../../utilpack/h 

USER_LINK_DLLS = -lqtmangent -lsqlite3 -lqtencrypt -lutilpack  -lrt

USER_OBJ_DIR = .

USER_BIN_DIR = .

optimize = 1
OPTFLAGS=$(USER_INCLDIRS)  -L../../../../utilpack/ -L../../../../bin/libs

include $(USER_DEPTH)/config/makeinclude/all_in_one.make
