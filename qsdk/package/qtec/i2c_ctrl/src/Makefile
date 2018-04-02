SRCBIN = common.c
SRCLIB = interface.c common.c
SRCTEST = spidev_test.c
all: i2c_ctrl 
LIBS = -lm

i2c_ctrl: $(SRCBIN)
	$(CC) $(LDFLAGS) -o i2c_ctrl $(SRCBIN) $(LIBS)
	
spidev_test: $(SRCTEST) libspidev.so
	$(CC) $(LDFLAGS) -o spidev_test $(SRCTEST) $(LIBS) -lspidev -L./

libspidev.so: $(SRCLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o libspidev.so $(SRCLIB)
	
clean:
	rm *.o *.so i2c_ctrl spidev_test
