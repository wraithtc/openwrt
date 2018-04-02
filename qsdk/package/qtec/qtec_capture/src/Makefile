LDLIBS+= -lm -lrtcfg -lpcap -luci

QTEC_CAPTURE_SRC = qtec_capture.c main.c


all: qtec_capture

qtec_capture: $(QTEC_CAPTURE_SRC)
	$(CC) $(CFLAGS) -L . $(LDFLAGS) -o qtec_capture $(QTEC_CAPTURE_SRC) $(LDLIBS) -lpthread -g

