#SRC = timer*.c cJSON.c
SRCBIN = smb_init.c 

all: smbinit
LIBS = -lm -lspidev -lrtcfg -lrtfwk

smbinit: $(SRCBIN)
	$(CC) $(CFLAGS) $(LDFLAGS) -o smbinit $(SRCBIN) $(LIBS)
	
clean:
	rm *.o smbinit
