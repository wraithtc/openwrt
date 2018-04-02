
USER_DEPTH = ../../../../

include $(USER_DEPTH)/config/makeinclude/env.make
include $(USER_DEPTH)/config/makeinclude/platform.make


USER_BIN = keyencrypt_test

USER_SRCS = \
	QtKeyEncrypt_test.cpp

USER_INCLDIRS = -I../../../include/ \
				-I../include/ \
				-I../../../../utilpack/h/ 

USER_LINK_DLLS = -lqtencrypt -lutilpack  -lrt

USER_OBJ_DIR = .

USER_BIN_DIR = .

optimize = 1
OPTFLAGS=$(USER_INCLDIRS)  -L../../../../utilpack/ -L../../../../bin/libs

include $(USER_DEPTH)/config/makeinclude/all_in_one.make
