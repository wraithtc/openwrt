#
# modified from ACE_wrappers\include\makeinclude\platform_linux.GNU
#

#debug ?= 0
#optimize ?= 1
#profile ?= 1
disable_assert=1

ifeq (1,$(BUILD_SPA))
MMP_OCTEON = 1
CPPFLAGS += -D_MMP_SPA_
LIBDIR=octeon
OCTEON_BUILD_ROOT=$(MV_SDK_ROOT)
endif

ifeq (1, $(BUILD_SPA_CENTOS))
MMP_OCTEON = 1
CPPFLAGS += -D_MMP_SPA_
LIBDIR=centos_spa
endif

#ifeq (1, $(MMP_OCTEON))
#ifneq ($(LIBDIR), centos_spa)
export CC = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
export CPP = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-g++
export GCC= /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
export CXX = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-g++
export AR = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-ar
#export LD = mips64_octeon_v2_be-gnu-ld
#CFLAGS_GLOBAL = -DOCTEON_MODEL=OCTEON_CN58XX -DOCTEON_TARGET=linux_n32 -mabi=n32 -march=octeon -msoft-float
#CFLAGS += $(CFLAGS_GLOBAL)
#CPPFLAGS += $(CFLAGS_GLOBAL)
#LDFLAGS += $(LDFLAGS_GLOBAL)
#endif
#else
#export CC = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
#export CPP = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-g++
#export GCC= /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
#export CXX= /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-g++
#endif

#add by quarkz to support 8i or 9i 
SIP = 1
SSE2 = 1

ifneq (1,$(MMP_OCTEON))
#ORACLE_8 = 1
#ORACLE_9 = 1
ORACLE_10 = 1
endif

#build macro, is it NBR Streaming server
#streaming_server = 1

ifeq (,$(ORACLE_8))
  ORACLE_8 = 0
endif

ifeq (,$(ORACLE_9))
  ORACLE_9 = 0
endif

ifeq (,$(ORACLE_10))
  ORACLE_10 = 0
endif

ifeq (,$(SIP))
  SIP = 0
endif
#
ifeq (,$(streaming_server))
  streaming_server = 0
endif

ifeq (1,$(streaming_server))
  SIP = 0
  SSE2 = 0
endif
#
exceptions ?= 1
threads ?= 1

ifeq (,$(debug))
  debug = 1
endif
ifeq (,$(optimize))
  optimize = 0
endif
ifeq (,$(profile))
  profile = 0
endif
ifeq (,$(disable_assert))
  disable_assert = 0
endif

#Field,2005-6-7
DEFFLAGS += -DQT_UNIX 
SYSTEM = $(shell uname -s)
ifeq ($(SYSTEM), Linux)
  DEFFLAGS += -DQT_LINUX
endif

ifeq (1,$(profile))
  DEFFLAGS += -pg
endif # debug

ifndef CC
CC  = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
endif
ifndef CPP
CPP = /usr/local/openwrt/RouterPre/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-g++
endif

LINUX_FLAGS = -Wall -Wpointer-arith -pipe
CFLAGS  += $(LINUX_FLAGS)
CPPFLAGS += $(LINUX_FLAGS)
ifeq ($(threads),1)
  CPPFLAGS  += -D_REENTRANT
endif # threads

# 
# Rely on _GNU_SOURCE to set these defaults defined in /usr/include/features.h 
# instead of setting them directly here (older versions of gcc don't set it
# for you): _SVID_SOURCE _BSD_SOURCE _POSIX_SOURCE _POSIX_C_SOURCE=199506L, etc...
CPPFLAGS += -D_GNU_SOURCE

DCFLAGS += -g

#OPTFLAGS += -O2
OPTFLAGS += -O3
LD      = $(CXX)
LIBS    += -ldl -lnsl -lresolv

ifeq ($(threads),1)
  LIBS += -lpthread
endif
ifneq (1, MMP_OCTEON)
LIBS += $(shell test -e /usr/lib/librt.so && echo -lrt)
endif

OCFLAGS += -O3
PIC     = -fPIC
ifndef AR
AR      = ar
endif
ARFLAGS = rsuv
RM      = rm -rf

ifeq ($(debug),1)
  NASMFLAG = -g
else
  NASMFLAG = -O3
endif
