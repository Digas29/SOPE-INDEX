CC = gcc
CFLAGS = -c -Wall
LDFLAGS = -Wall
SOURCES = sw.c csc.c index.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = $(OBJECTS:.o=)
OUTFOLDER = bin

.PHONY: all


all: makedir $(OBJECTS) $(EXECUTABLES) remove

.c.o:
	$(CC) $(CFLAGS) $< -o $(OUTFOLDER)/$@
.o:
	$(CC) $(OUTFOLDER)/$< -o $(OUTFOLDER)/$@
remove:
	rm $(OUTFOLDER)/*.o

makedir:
	mkdir -p $(OUTFOLDER)
