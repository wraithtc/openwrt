#SRC = timer*.c cJSON.c
SRCBIN = scrctrl.c cJSON.c
SRCLIB = interface.c common.c
SRCTEST = spidev_test.c
all: scrctrl
LIBS = -lm -lrtcfg -lrtfwk

scrctrl: $(SRCBIN)
	$(CC) $(LDFLAGS) -o scrctrl $(SRCBIN) $(LIBS)
	
spidev_test: $(SRCTEST) libspidev.so
	$(CC) $(LDFLAGS) -o spidev_test $(SRCTEST) $(LIBS) -lspidev -L./

libspidev.so: $(SRCLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o libspidev.so $(SRCLIB)
	
clean:
	rm *.o *.so spiinit spidev_test
