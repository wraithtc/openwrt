#SRC = timer*.c cJSON.c
SRC = ${wildcard *.c}
all: qtec_telnetd
LIBS = -lm -lrtcfg -lrtfwk -lqteccli

qtec_telnetd: $(SRC)
	$(CC) $(LDFLAGS) -o telnetd $(SRC) $(LIBS)
	
spidev_test: $(SRCTEST) libspidev.so
	$(CC) $(LDFLAGS) -o spidev_test $(SRCTEST) $(LIBS) -lspidev -L./

libqteccli.so: $(SRCLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o libqteccli.so $(SRCLIB)
	
clean:
	rm *.o telnetd
