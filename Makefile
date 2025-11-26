CC=gcc -g -O2
LD=gcc
XML2_HOME=/usr
PNG_HOME=/usr
CFLAGS=-I$(XML2_HOME)/include/libxml2 -I$(XML2_HOME)/include/libxslt
PNG_CFLAGS=-I$(PNG_HOME)/include/libpng16
LDFLAGS=-L$(XML2_HOME)/lib
PNG_LDFLAGS=-L$(PNG_HOME)/lib/x86_64-linux-gnu
LIBS=-lxml2 -lxslt
PNG_LIBS=-lpng -lz
RM=rm -rf
.PHONY: all clean
all: createxml readxml readxml2 testWriter pngprog001 temp1 temp2 xmltm2ascii \
	readraw readraw16 readrawf sval createlabel write_png read_png write_gs_png read_gs_png archivepds archivepds2 readlabel
pds.o: pds.c
	$(CC) -c $(CFLAGS) $< -o $@
createxml: createxml.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
createlabel: createlabel.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
readlabel: readlabel.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
archivepds: archivepds.c pds.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)
archivepds2: archivepds2.c pds.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)
readxml: readxml.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
readxml2: readxml2.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
parse1: parse1.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
parse2: parse2.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
sval: sval.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
testWriter: testWriter.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
xmltm2ascii: xmltm2ascii.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
pngprog001: pngprog001.c
	$(CC) $(PNG_CFLAGS) $< -o $@ $(PNG_LDFLAGS) $(PNG_LIBS)
write_png: write_png.c
	$(CC) $(PNG_CFLAGS) $< -o $@ $(PNG_LDFLAGS) $(PNG_LIBS)
read_png: read_png.c
	$(CC) $(PNG_CFLAGS) $< -o $@ $(PNG_LDFLAGS) $(PNG_LIBS)
write_gs_png: write_gs_png.c
	$(CC) $(PNG_CFLAGS) $< -o $@ $(PNG_LDFLAGS) $(PNG_LIBS)
read_gs_png: read_gs_png.c
	$(CC) $(PNG_CFLAGS) $< -o $@ $(PNG_LDFLAGS) $(PNG_LIBS)
temp1: temp1.c
	$(CC) $< -o $@
temp2: temp2.c
	$(CC) $< -o $@
readraw: readraw.c
	$(CC) $< -o $@
readrawf: readrawf.c
	$(CC) $< -o $@
readraw16: readraw16.c
	$(CC) $< -o $@
clean:
	$(RM) *.o readxml readxml2 createxml testWriter pngprog001 temp1 temp2 xmltm2ascii \
	readraw readraw16 parse1 parse2 sval createlabel write_png read_png archivepds \
	write_gs_png read_gs_png archivepds2 readlabel readrawf


