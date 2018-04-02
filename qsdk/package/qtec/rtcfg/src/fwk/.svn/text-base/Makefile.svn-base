include $(PFM_ROOT_PATH)/make/Makefile.config

_TARGET_NAME  := libfwk.so
_SRC_PATHS    := $(PFM_FWK_PATH)/src
_BUILD_PATH   := $(PFM_FWK_BUILD_PATH)
CFLAGS        += -I$(PFM_FWK_PATH)/include
CFLAGS        += -I$(PFM_FWK_PATH)/include/vos
CFLAGS        += -I$(PFM_FWK_PATH)/include/log
CFLAGS        += -I$(PFM_FWK_PATH)/include/util
CFLAGS        += -I$(PFM_FWK_PATH)/include/xml
CFLAGS        += -I$(PFM_FWK_PATH)/include/tmr
#CFLAGS        += -I$(PFM_HAL_UTIL_PATH)/include/modules
_LIBS         +=
LDFLAGS       += -lcrypt
ifdef PFM_SIM
LDFLAGS       += -lrt
endif
LDFLAGS       := $(filter-out -lsoft_feature,$(LDFLAGS))
_INSTALL_PATH += $(PFM_LIB_INSTALL_PATH)

.PHONY: prepare all install clean
prepare: 
all: _all_
install: _install_
clean: _clean_

include $(PFM_ROOT_PATH)/make/Makefile.user
