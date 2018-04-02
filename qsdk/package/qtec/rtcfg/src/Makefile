SRC = lanhost.c auth_set.c systeminfo_get.c firewall_set.c lan_set.c ntp_set.c network_set.c rtcfg_uci.c wireless_set.c timerTask_set.c timertask_uci.c devinfo.c system_set.c offline_get.c arpband_set.c network_speed.c security.c vpn.c wifi_txpower.c qos.c cgireq.c manufactory.c
RTFWK_SRC = $(wildcard fwk/src/vos/*.c) 
RTFWK_SRC += $(wildcard fwk/src/util/*.c)
RTFWK_SRC += $(wildcard fwk/src/util/linux/*.c)
RTFWK_SRC += $(wildcard fwk/src/vos/linux/*.c)
CFLAGS+= -fPIC -Ifwk/include -Ifwk/include/util -Ifwk/src/util/linux -Ifwk/include/vos -Ifwk/src/vos/linux
LDFLAGS+= -shared -lpthread
LDLIBS+= -luci

NETWORKTEST_SRC= network_test.c network_set.c rtcfg_uci.c
NTP_SRC= ntp_test.c ntp_set.c rtcfg_uci.c
LAN_SRC= lan_test.c lan_set.c rtcfg_uci.c
FIREWALL_SRC= firewall_test.c firewall_set.c rtcfg_uci.c
WIRELESS_SRC= auth_set.c wireless_test.c wireless_set.c rtcfg_uci.c
SYSTEM_SRC = systeminfo_test.c systeminfo_get.c rtcfg_uci.c
LANHOST_SRC = lanhost_test.c lanhost.c rtcfg_uci.c
TIMERTASK_SRC = timerTask_test.c timerTask_set.c timertask_uci.c
MSGTEST_SRC = msgtest.c
SMD_SRC = smd.c
NETWORK_SPEED_SRC=Network_speed_test.c network_speed.c rtcfg_uci.c
WIFI_TXPOWER_INIT_SRC = wifi_txpower_init.c wifi_txpower.c rtcfg_uci.c

all: librtcfg.so librtfwk.so msgtest smd  wifi_txpower_init

librtfwk.so:$(RTFWK_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o librtfwk.so $(RTFWK_SRC)

librtcfg.so: $(SRC) librtfwk.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o librtcfg.so $(SRC) $(LDLIBS) -lrtfwk -L./ -lspidev

network_test:
	$(CC) $(CFLAGS) -o network_test $(NETWORKTEST_SRC) $(LDLIBS) -g -O0

ntp_test:
	$(CC) $(CFLAGS) -o ntp_test $(NTP_SRC) $(LDLIBS) -g -O0

lan_test:
	$(CC) $(CFLAGS) -o lan_test $(LAN_SRC) $(LDLIBS) -g -O0

firewall_test:
	$(CC) $(CFLAGS) -o firewall_test $(FIREWALL_SRC) $(LDLIBS) -g -O0
	
wireless_test: librtfwk.so
	$(CC) $(CFLAGS) -o wireless_test $(WIRELESS_SRC) $(LDLIBS) -g -O0 -lrtfwk -L./

system_test:
	$(CC) $(CFLAGS) -o system_test $(SYSTEM_SRC) $(LDLIBS) -g -O0

lanhost_test:
	$(CC) $(CFLAGS) -o lanhost_test $(LANHOST_SRC) $(LDLIBS) -g -O0

timertask_test: librtfwk.so
	$(CC) $(CFLAGS) -L./ -o timertask_test $(TIMERTASK_SRC) $(LDLIBS) -lrtfwk -g
	
msgtest: librtfwk.so
	$(CC) $(CFLAGS) -L./ -o msgtest $(MSGTEST_SRC) $(LDLIBS) -lrtfwk -g
	
smd: librtfwk.so
	$(CC) $(CFLAGS) -L./ -o smd $(SMD_SRC) $(LDLIBS) -lrtfwk -g

network_speed_test: $(NETWORK_SPEED_SRC)
	$(CC) $(CFLAGS) -o network_speed_test $(NETWORK_SPEED_SRC) $(LDLIBS) -lpthread -g -O0	

wifi_txpower_init: $(WIFI_TXPOWER_INIT_SRC)
	$(CC) $(CFLAGS) -o wifi_txpower_init $(WIFI_TXPOWER_INIT_SRC) $(LDLIBS) -lpthread -g -O0	

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o *.so
