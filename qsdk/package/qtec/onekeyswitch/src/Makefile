SRC = *.c
all: onekeyswitch
LIBS += -lm -lpthread -lrtfwk -lrtcfg
DEBUG_FLAGS = -g -O0

onekeyswitch: $(SRC)
	$(CC) $(LDFLAGS) -o onekeyswitch $(SRC) $(LIBS) $(DEBUG_FLAGS)

clean:
	rm *.o onekeyswitch