#SRC = timer*.c cJSON.c
SRC = ${wildcard *.c}
all: qtec_consoled
LIBS = -lm -lrtcfg -lrtfwk -lqteccli

qtec_consoled: $(SRC)
	$(CC) $(LDFLAGS) -o consoled $(SRC) $(LIBS)
	
spidev_test: $(SRCTEST) libspidev.so
	$(CC) $(LDFLAGS) -o spidev_test $(SRCTEST) $(LIBS) -lspidev -L./

libqteccli.so: $(SRCLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o libqteccli.so $(SRCLIB)
	
clean:
	rm *.o consoled
