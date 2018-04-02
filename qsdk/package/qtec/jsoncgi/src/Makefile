LDLIBS+= -lm -lrtcfg -lcrypto -lrtfwk -lspidev -lwebsockets -lrtfwk
WEB_SRC = web-cgi.c basic.c getKey.c guipassword.c lan.c cgimsgreq.c ntp.c pf.c rsa.c system.c wan.c wireless.c cJSON.c timertask.c clientinfo.c arpbandinfo.c macblock.c ddos.c childrule.c speedlimit.c dmz.c security.c vpn.c wan_bandwidth.c antiwifimanager.c qos.c specialcare.c ebtables_speedlimit.c qtec_disk.c
JSON_SRC = basic.c getKey.c guipassword.c lan.c cgimsgreq.c ntp.c  rsa.c system.c wan.c wireless.c cJSON.c timertask.c security.c vpn.c childrule.c antiwifimanager.c macblock.c wan_bandwidth.c qos.c specialcare.c  speedlimit.c ebtables_speedlimit.c qtec_disk.c
CGICFG_SRC = wireless.c wan.c timertask.c system.c rsa.c pf.c ntp.c lan.c guipassword.c getKey.c clientinfo.c cgimsgreq.c basic.c arpbandinfo.c security.c vpn.c childrule.c antiwifimanager.c macblock.c wan_bandwidth.c qos.c specialcare.c  speedlimit.c ebtables_speedlimit.c qtec_disk.c
CFLAGS+= -I../../../staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/mipsel-openwrt-linux-uclibc/include/c++/4.8.3
CFLAGS+= -I../../../staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/mipsel-openwrt-linux-uclibc/include/c++/4.8.3/mipsel-openwrt-linux-uclibc
QTEC_SPEEDTEST_SRC= qtec_speedtest.c

all:libkeyapi.so json.cgi web.cgi qtkeyManager cgimsgmanager antiwifireload libcgicfgapi.so qtec_speedtest imgUpgrade

web.cgi: $(WEB_SRC) libkeyapi.so
	$(CC) $(CFLAGS) -L . $(LDFLAGS) -o web.cgi $(WEB_SRC) $(LDLIBS) -lpthread -lqtecfirewall -lkeyapi -lqtecebtables

qtec_speedtest: $(QTEC_SPEEDTEST_SRC)
	$(CC) $(CFLAGS) -L . $(LDFLAGS) -g -o qtec_speedtest $(QTEC_SPEEDTEST_SRC) $(LDLIBS) -lpthread 

json.cgi: json-cgi.o keyapi.o
	$(CC) $(LDFLAGS) keyapi.o json-cgi.o -o json.cgi $(JSON_SRC) $(LDLIBS) -lstdc++ -lqtmangent -lqtencrypt -lsqlite3 -lpthread -l:libutilpack.a -lqtecfirewall -lqtecebtables

qtkeyManager: qtkeymanager.c keyapi.o
	$(CC) $(LDFLAGS) keyapi.o -o qtkeyManager qtkeymanager.c $(LDLIBS) -lstdc++ -lqtmangent -lqtencrypt -lsqlite3 -lpthread -l:libutilpack.a

cgimsgmanager: cgimsgmanager.c keyapi.o
	$(CC) $(LDFLAGS) keyapi.o -o cgimsgmanager cgimsgmanager.c $(LDLIBS) -lstdc++ -lqtmangent -lqtencrypt -lsqlite3 -lpthread -l:libutilpack.a

antiwifireload: antiwifireload.c
	$(CC) $(LDFLAGS) -o antiwifireload antiwifireload.c $(LDLIBS) -lstdc++ -lqtmangent -lqtencrypt -lsqlite3 -lpthread -l:libutilpack.a

libkeyapi.so: keyapi.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) -fPIC -shared keyapi.cpp -o libkeyapi.so $(LDLIBS) -lstdc++ -lqtmangent -lqtencrypt -lsqlite3 -lpthread -l:libutilpack.a

libcgicfgapi.so: $(CGICFG_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -shared  $(CGICFG_SRC) -o libcgicfgapi.so $(LDLIBS) -lpthread -lqtecfirewall -lqtecebtables

keyapi.o:keyapi.cpp
	$(CXX) $(CFLAGS) -c keyapi.cpp $(LDLIBS) -lstdc++ -lqtmangent -lqtencrypt -lsqlite3 -lpthread -l:libutilpack.a

cJSON.o:cJSON.c
	$(CC) $(CFLAGS) -c cJSON.c

json-cgi.o: json-cgi.c
	$(CC) $(CFLAGS) -c json-cgi.c

web-cgi.o: web-cgi.c
	$(CC) $(CFLAGS) -c web-cgi.c

imgUpgrade:imgupgrade.c
	$(CC) $(CFLAGS) -o imgupgrade imgupgrade.c $(LDLIBS)

clean:
	rm *.o *.so json-cgi
