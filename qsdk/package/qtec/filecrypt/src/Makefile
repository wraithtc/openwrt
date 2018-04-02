SRC = *.c
all: fscrypt
LIBS += -lm -lfuse -lcrypto -lspidev -lulockmgr
DEBUG_FLAGS = -g -O0

fscrypt: $(SRC)
	$(CC) $(LDFLAGS) -o fscrypt $(SRC) $(LIBS) $(DEBUG_FLAGS) -D_FILE_OFFSET_BITS=64

clean:
	rm *.o fscrypt