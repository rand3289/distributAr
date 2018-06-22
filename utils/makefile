CC=g++
CFLAGS=-g -c -Wall -std=c++11 -I../ -I../lib
LDFLAGS=
SOURCES=client.cpp graph.cpp graphio.cpp timeserv.cpp tracker.cpp ../lib/udp.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: netg netc nett netw

.cpp.o: $(SOURCES)
	$(CC) -c $(CFLAGS) $< -o $@

netg:	$(OBJECTS)
	$(CC) $(LDFLAGS) graphio.o graph.o -o netg
netc:	$(OBJECTS)
	$(CC) $(LDFLAGS) client.o graphio.o ../lib/udp.o -o netc
nett:	$(OBJECTS)
	$(CC) $(LDFLAGS) tracker.o ../lib/udp.o -o nett
netw:	$(OBJECTS)
	$(CC) $(LDFLAGS) timeserv.o ../lib/udp.o -o netw

clean:
	rm -f *.o
	rm -f netg netc nett netw
	rm -f *~
	rm -f \#*\#

