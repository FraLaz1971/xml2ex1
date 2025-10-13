CC=gcc
LD=gcc
XML2_HOME=/data/data/com.termux/files/usr
CFLAGS=-I$(XML2_HOME)/include/libxml2 -I$(XML2_HOME)/include/libxslt
LDFLAGS=-L$(XML2_HOME)/lib
LIBS=-lxml2 -lxslt
RM=rm -rf
.PHONY: all clean
all: createxml readxml readxml2 testWriter
createxml: createxml.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
readxml: readxml.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
readxml2: readxml2.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
testWriter: testWriter.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
clean:
	$(RM) *.o readxml readxml2 createxml testWriter
