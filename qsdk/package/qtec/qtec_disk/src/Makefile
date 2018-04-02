LDLIBS+= -lm -lrtcfg -lcrypto -lrtfwk -lspidev -lrtfwk

QTEC_GETKEY_SRC = qtec_getkey.c
QTEC_DISKMANAGER_SRC = qtec_diskmanager.c

all: qtec_getkey qtec_diskmanager

qtec_getkey: $(QTEC_GETKEY_SRC)
	$(CC) $(CFLAGS) -L . $(LDFLAGS) -o qtec_getkey $(QTEC_GETKEY_SRC) $(LDLIBS) -lpthread

qtec_diskmanager: $(QTEC_DISKMANAGER_SRC)
	$(CC) $(CFLAGS) -L. $(LDFLAGS) -o qtec_diskmanager $(QTEC_DISKMANAGER_SRC) $(LDLIBS) -lpthread
