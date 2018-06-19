CC=g++
CFLAGS=-g -c -Wall -std=c++11 -I../
LDFLAGS=
SOURCES=graphio.cpp graph.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=netg

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
