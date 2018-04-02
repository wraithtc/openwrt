#SRC = timer*.c cJSON.c
SRCBIN = wdsctrl.c
LIBS = -lm -lrtcfg -lrtfwk -lpthread
all: wdsctrl


wdsctrl: $(SRCBIN)
	$(CC) $(LDFLAGS) -o wdsctrl $(SRCBIN) $(LIBS)
	
clean:
	rm *.o *.so wdsctrl