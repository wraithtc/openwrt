WIFISONEVENTSAMPLEDIR := $(shell pwd)

ifneq ($(strip $(TOOLPREFIX)),)
export  CROSS:=$(TOOLPREFIX)
endif


export INCLUDES = -I$(WIFISONEVENTSAMPLEDIR)/../
export CC = $(CROSS)gcc
export CFLAGS += -O2 -Wall -Werror -fPIC
export CFLAGS += $(QCACFLAGS)
export LDFLAGS += ${QCALDFLAGS}
export DEFS =
export OBJEXT = o
export EXEEXT =
export RANLIB = $(CROSS)ranlib
export STRIP = $(CROSS)strip
export ARFLAGS = cru
export AR = $(CROSS)ar
export COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)
export LINK = $(CC) $(CFLAGS) $(LDFLAGS)


LIBS += -lqca_wifison_ext

TARTGET_OBJECTS = qca_event_sample.o


all: qca_event_sample$(EXEEXT) install

qca_event_sample$(EXEEXT): $(TARTGET_OBJECTS)
	$(LINK) -o $@ $(TARTGET_OBJECTS) -Xlinker --start-group $(LIBS) -Xlinker --end-group

.c.o:
	$(COMPILE) -c $<

install:
	@cp qca_event_sample$(EXEEXT) $(WIFISONEVENTSAMPLEDIR)/../install/sbin

clean:
	@rm -f *.o *~ hyd$(EXEEXT)

.PHONY: all clean install
