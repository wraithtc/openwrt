#SRC = timer*.c cJSON.c
SRCBIN = tcpproxy.c cJSON.c thread_pool.c
LIBS = -lm -lrtcfg -lrtfwk -lpthread -lspidev
all: tcpproxy


tcpproxy: $(SRCBIN)
	$(CC) $(LDFLAGS) -o tcpproxy $(SRCBIN) $(LIBS)
	
clean:
	rm *.o *.so tcpproxy
