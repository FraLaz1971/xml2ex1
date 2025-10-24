CC=gcc
LD=gcc
XML2_HOME=/usr
CFLAGS=-I$(XML2_HOME)/include/libxml2 -I$(XML2_HOME)/include/libxslt
PNG_CFLAGS=-I$(XML2_HOME)/include/libpng16
LDFLAGS=-L$(XML2_HOME)/lib
PNG_LDFLAGS=-L$(XML2_HOME)/lib
LIBS=-lxml2 -lxslt
PNG_LIBS=-lpng -lz
RM=rm -rf
.PHONY: all clean
all: createxml readxml readxml2 testWriter pngprog001 temp1 temp2 xmltm2ascii readraw readraw16 sval xmlreader readlabel createlabel
createxml: createxml.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
createlabel: createlabel.c
	$(CC) -g -O2 $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
readxml: readxml.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
xmlreader: xmlreader.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
readlabel: readlabel.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
xmltm2ascii: xmltm2ascii.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
readxml2: readxml2.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
sval: sval.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
testWriter: testWriter.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
pngprog001:	pngprog001.c
	$(CC) $(PNG_CFLAGS) $< -o $@ $(LDFLAGS) $(PNG_LIBS)
temp1:	temp1.c
	$(CC) $< -o $@
temp2:	temp2.c
	$(CC) $< -o $@
readraw: readraw.c
	$(CC) $< -o $@
readraw16: readraw16.c
	$(CC) $< -o $@
clean:
	$(RM) *.o readxml readxml2 createxml testWriter pngprog001 temp1 temp2 xmltm2ascii readraw readraw16 sval xmlreader readlabel createlabel
