all: bin_target so_target

BIN_SRC = qtec_ebtables_main.c  qtec_ebtables_logic.c qtec_ebtables_basic.c
SO_SRC = qtec_ebtables_logic.c  qtec_ebtables_basic.c qtec_ebtables_speedlimit.c
LDLIBS+= -luci -lubox -lrtcfg

bin_target: $(BIN_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o qtec_ebtables $(BIN_SRC) $(LDLIBS)

so_target: $(SO_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC  -shared -o libqtecebtables.so $(SO_SRC) $(LDLIBS)

clean:
	rm *.o qtec_ebtables *.so
	
