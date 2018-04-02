all: bin_target so_target

BIN_SRC = qtec_firewall_main.c arpbound.c basic.c qtec_firewall.c macblock.c ddos.c childrule.c speedlimit.c dmz.c portforward.c
SO_SRC = qtec_firewall_logic.c arpbound.c basic.c qtec_firewall.c macblock.c ddos.c childrule.c speedlimit.c dmz.c portforward.c
LDLIBS+= -luci -lubox -lrtcfg

bin_target: $(BIN_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o qtec_firewall $(BIN_SRC) $(LDLIBS)

so_target: $(SO_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC  -shared -o libqtecfirewall.so $(SO_SRC) $(LDLIBS)

clean:
	rm *.o qtec_firewall *.so
	
