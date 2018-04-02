SRC = *.c
all: lanHostManager
LIBS = -lm -lubus -lubox -lpthread -luci -ljson-c -lrtfwk -lrtcfg -lqtecfirewall
DEBUG_FLAGS = -g -O0

lanHostManager: $(SRC)
	$(CC) $(LDFLAGS) -o lanHostManager $(SRC) $(LIBS) $(DEBUG_FLAGS)

clean:
	rm *.o lanHostManager

