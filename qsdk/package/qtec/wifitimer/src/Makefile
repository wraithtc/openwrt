#SRC = timer*.c cJSON.c
SRCBIN = timerTask*.c timertask_uci.c spidev_test.c
SRCCGI = timerTask_set.c timertask_uci.c cJSON.c timertask_cgi.c
all: wifitimer wifitimer.cgi
LIBS = -lm -lubus -lubox -luci
wifitimer.cgi: $(SRCCGI)
	$(CC) $(LDFLAGS) -o wifitimer.cgi $(SRCCGI) $(LIBS)

wifitimer: $(SRCBIN)
	$(CC) $(LDFLAGS) -o wifitimer $(SRCBIN) $(LIBS)
	
clean:
	rm *.o wifitimer.cgi wifitimer
