#SRC = timer*.c cJSON.c
SRCBIN = spi_dev_init.c common.c
SRCLIB = interface.c common.c
SRCTEST = spidev_test.c
all: spiinit libspidev.so spidev_test
LIBS = -lm

spiinit: $(SRCBIN)
	$(CC) $(LDFLAGS) -o spiinit $(SRCBIN) $(LIBS)
	
spidev_test: $(SRCTEST) libspidev.so
	$(CC) $(LDFLAGS) -o spidev_test $(SRCTEST) $(LIBS) -lspidev -L./

libspidev.so: $(SRCLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o libspidev.so $(SRCLIB)
	
clean:
	rm *.o *.so spiinit spidev_test
