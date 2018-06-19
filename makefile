SOURCES=main.cpp server.cpp network.cpp lib/timesync.cpp lib/udp.cpp lib/misc.cpp
ONLYH=lib/atomq.h lib/guard.h  lib/icluster.h lib/pool.h  lib/serial.h lib/timebuff.h lib/timesync.h lib/udp.h

CFLAGS=-g -Wall -std=c++11 -I. -I./lib/
LDFLAGS=-ldl -lpthread #-latomic # GCC __atomic_* built-ins
CC=g++

OBJECTS=$(SOURCES:.cpp=.o)
HEADERS=$(SOURCES:.cpp=.h)
EXECUTABLE=nets

all: $(EXECUTABLE)  so  utils

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

.cpp.o: $(SOURCES) $(HEADERS) $(ONLYH)
	$(CC) -c $(CFLAGS) $< -o $@

so:	dummy
	cd dummy;	make -f makefile --no-print-directory; cd ..

utils:	dummy
	cd utils;	make -f makefile --no-print-directory; cd ..

clean:
	rm -f *.o
	rm -f *~
	rm -f \#*\#
	rm -f lib/*.o
	rm -f lib/*~
	rm -f lib/\#*\#
	rm -f dummy/*.o
	rm -f dummy/*~
	rm -f dummy/\#*\#
	rm -f utils/*.o
	rm -f utils/*~
	rm -f utils/\#*\#

nobin:
	rm -f nets
	rm -f *.so.?
