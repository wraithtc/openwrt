SRC = *.c
all: QtecDeviceManager
LIBS = -lm -lubus -lubox -lpthread -luci -ljson-c -lsqlite3 -lrtfwk -lrtcfg
DEBUG_FLAGS = -g -O0

QtecDeviceManager: $(SRC)
	$(CC) $(LDFLAGS) -o QtecDeviceManager $(SRC) $(LIBS) $(DEBUG_FLAGS)

clean:
	rm *.o QtecDeviceManager

