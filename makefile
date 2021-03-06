SOURCES=main.cpp server.cpp network.cpp lib/timesync.cpp lib/udp.cpp lib/misc.cpp
ONLYH=lib/blockq.h lib/spinlock.h lib/icluster.h lib/timebuff.h lib/main.h

CFLAGS=-g -Wall -std=c++11 -I. -I./lib/
LDFLAGS=-ldl -lpthread #-latomic # GCC __atomic_* built-ins # dynamic loader and pthread libs
CC=g++

OBJECTS=$(SOURCES:.cpp=.o)
HEADERS=$(SOURCES:.cpp=.h)
EXECUTABLE=nets

all: $(EXECUTABLE)  so  utils

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

.cpp.o: $(SOURCES) $(HEADERS) $(ONLYH)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: so utils clean nobin # tell make these are not real files to be built

so:
	@echo "----------------------------------------------------------"
	cd dummy;	make -f makefile --no-print-directory; cd ..

utils:
	@echo "----------------------------------------------------------"
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
	rm -f utils/net?
