# this make file builds a dummy IO cluster dll

SOURCES=dummy.cpp
# -fpic for position independent code
CFLAGS=-fpic -g -Wall -std=c++11 -I. -I.. -I../lib/
LDFLAGS=
CC=g++

OBJECTS=$(SOURCES:.cpp=.o)
HEADERS=$(SOURCES:.cpp=.h)
LIBDUMMY=../libdummy.so.1 # output DLL for dummy IO cluster

all: $(LIBDUMMY)

.cpp.o: $(SOURCES)
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBDUMMY): $(OBJECTS)
	rm -f $(LIBDUMMY)
	$(CC) -shared -Wl,-soname,$(LIBDUMMY) -o $(LIBDUMMY) $(OBJECTS)

clean:
	rm -f *.o
	rm -f *~
	rm -f \#*\#
