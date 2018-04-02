SRC = *.c
all: myclient
LIBS += -lm -lpthread -luci -lsqlite3 -lrtfwk -lrtcfg -lwebsockets -lkeyapi -lcrypto -lcgicfgapi -lspidev
DEBUG_FLAGS = -g -O0

myclient: $(SRC)
	$(CC) $(LDFLAGS) -o myclient $(SRC) $(LIBS) $(DEBUG_FLAGS)

clean:
	rm *.o myclient
