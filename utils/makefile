CC=g++
CFLAGS=-g -c -Wall -std=c++11 -I../ -I../lib
LDFLAGS=

all: graph client tracker timeserv
	@echo "-----------------------------------"

graph:
	@echo "-----------------------------------"
	$(CC) $(CFLAGS) graphio.cpp graph.cpp
	$(CC) $(LDFLAGS) graphio.o graph.o -o netg
client:
	@echo "-----------------------------------"
	$(CC) $(CFLAGS) client.cpp graphio.cpp ../lib/udp.cpp
	$(CC) $(LDFLAGS) client.o graphio.o udp.o -o netc
tracker:
	@echo "-----------------------------------"
	$(CC) $(CFLAGS) tracker.cpp ../lib/udp.cpp
	$(CC) $(LDFLAGS) tracker.o udp.o -o nett
timeserv:
	@echo "-----------------------------------"
	$(CC) $(CFLAGS) timeserv.cpp ../lib/udp.cpp
	$(CC) $(LDFLAGS) timeserv.o udp.o -o netw

clean:
	rm -f *.o
	rm -f netg netc nett netw
	rm -f *~
	rm -f \#*\#
