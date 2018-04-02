SRC = $(wildcard *.cpp)

all: timertask
LIBS = -lm -lrtcfg -lrtfwk -lstdc++
wifitimer.cgi: $(SRCCGI)
	$(CC) $(LDFLAGS) -o wifitimer.cgi $(SRCCGI) $(LIBS)

timertask: $(SRC)
	$(CXX) $(LDFLAGS) -o timertask $(SRC) $(LIBS)
	
clean:
	rm *.o timertask
