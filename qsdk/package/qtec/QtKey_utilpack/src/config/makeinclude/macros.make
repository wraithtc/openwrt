#
# modified from ACE_wrappers\include\makeinclude\wrapper_macros.GNU
#

SRC_ROOT = $(USER_DEPTH)

ifdef USER_BIN
  BIN = $(USER_BIN)
endif # USER_BIN

ifdef USER_LIB
  LIB = lib$(USER_LIB).$(LIBEXT)
endif # USER_LIB

ifdef USER_DLL
  DLL = lib$(USER_DLL).$(DLLEXT)
endif # USER_DLL

include $(SRC_ROOT)/config/makeinclude/platform.make


#----------------------------------------------------------------------------
#      Platform-independent macro definitions
#----------------------------------------------------------------------------

####
#### Defaults are exceptions off (0), fast off (0), rtti off (0), and
#### versioned_so on (1).
####
exceptions ?= 0
fast ?= 0
rtti ?= 0

INCLDIRS += $(USER_INCLDIRS)
DEFFLAGS += $(USER_DEFFLAGS) 
LIBS += $(USER_LINK_LIBS)
ifeq (1, $(MMP_OCTEON))
LIBS    += -ldl  -lm -lc -lnsl -lresolv
LIBS += -lpthread -lnss_files -lnss_dns 
LIBS += -lrt
endif
DLLS += $(USER_LINK_DLLS)

ifeq (1,$(debug))
  DEFFLAGS += -DQT_DEBUG
  CFLAGS += $(DCFLAGS)
  CPPFLAGS += $(DCFLAGS)
endif # debug

ifeq (1,$(disable_assert))
  DEFFLAGS += -DQT_DISABLE_ASSERTE
  CFLAGS += $(DCFLAGS)
  CPPFLAGS += $(DCFLAGS)
endif # disable_assert

ifeq (1,$(optimize))
  CFLAGS += $(OPTFLAGS)
  CPPFLAGS += $(OPTFLAGS)
endif # optimize

CPPFLAGS += $(DEFFLAGS) $(INCLDIRS)

# Let users override the default output directories
ifdef USER_OBJ_DIR
  OBJDIR = $(USER_OBJ_DIR)
else
  OBJDIR = $(SRC_ROOT)/bin/objects
endif # USER_OBJ_DIR

#ifeq (1,$(debug))
#  OBJDIR = $(OBJDIR)_debug
#endif # debug

ifdef USER_LIB_DIR
  LIB_DIR = $(USER_LIB_DIR)
else
  LIB_DIR = $(SRC_ROOT)/bin/libs
endif # USER_LIB_DIR

#share library, Field
ifdef USER_DLL_DIR
  DLL_DIR = $(USER_DLL_DIR)
else
  DLL_DIR = $(SRC_ROOT)/bin/libs
endif # USER_DLL_DIR

ifdef USER_BIN_DIR
  BIN_DIR = $(USER_BIN_DIR)
else
  BIN_DIR = $(SRC_ROOT)/bin
endif # USER_BIN_DIR

OBJEXT ?= o
LIBEXT ?= a
DLLEXT ?= so

CC_OUTPUT_FLAG ?= -o
DLL_OUTPUT_FLAG ?= -o
LINK_OUTPUT_FLAG ?= -o

COMPILE.cc  = $(CC) $(CFLAGS) -c
COMPILE.cpp = $(CXX) $(CPPFLAGS) -c

LINK.cc  = $(LD) $(CFLAGS) $(LDFLAGS) 
LINK.cpp = $(LD) $(CPPFLAGS) $(LDFLAGS)

NULL_STDERR = 2>$(/dev/null) || true

ifneq (1, $(MMP_OCTEON))
MYSQL_HOME=./mysql
ORACLE_HOME=/home/oracle/OraHome1
endif

#add by quarkz to support build with oracle 8i and 9i
ORACLE_8_HOME=/home/oracle/OraHome1
ORACLE_9_HOME=/home/oracle/OraHome9
ORACLE_10_HOME=/home/oracle/OraHome10

ifeq (1,$(ORACLE_8))
	ORACLE_HOME=$(ORACLE_8_HOME)
endif
ifeq (1,$(ORACLE_9)) 
	ORACLE_HOME=$(ORACLE_9_HOME)
endif
ifeq (1,$(ORACLE_10))
	ORACLE_HOME=$(ORACLE_10_HOME)
endif

MODULE = Atlas
SERVER_VERSION = 3.6.0
BUILD_DATE= $(shell date +%m%d%Y)

