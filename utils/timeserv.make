CC=g++
CFLAGS=-g -c -Wall -std=c++11 -I../ -I../lib
LDFLAGS=
SOURCES=timeserv.cpp ../lib/udp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=netw

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
