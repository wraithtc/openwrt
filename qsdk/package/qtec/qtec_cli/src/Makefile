#SRC = timer*.c cJSON.c
SRCLIB = ${wildcard *.c}
all: libqteccli.so
LIBS = -lm -lrtcfg -lrtfwk

scrctrl: $(SRCBIN)
	$(CC) $(LDFLAGS) -o scrctrl $(SRCBIN) $(LIBS)
	
spidev_test: $(SRCTEST) libspidev.so
	$(CC) $(LDFLAGS) -o spidev_test $(SRCTEST) $(LIBS) -lspidev -L./

libqteccli.so: $(SRCLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o libqteccli.so $(SRCLIB)
	
clean:
	rm *.o *.so
