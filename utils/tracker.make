CC=g++
CFLAGS=-g -c -Wall -std=c++11 -I../ -I../lib
# on linux without -pthread an exception is thrown when thread is created (std::thread uses pthread?)
# LDFLAGS= -pthread
SOURCES=tracker.cpp ../lib/udp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=nett

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
