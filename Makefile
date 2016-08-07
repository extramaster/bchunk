all: bchunk

# For systems with GCC (Linux, and others with GCC installed):
CC = gcc
LD = gcc
CFLAGS = -Wall -Wstrict-prototypes -O2

# For systems with a legacy CC:
#CC = cc
#LD = cc
#CFLAGS = -O

# For BSD install: Which install to use and where to put the files
INSTALL = install
PREFIX  = /usr/local
BIN_DIR = $(PREFIX)/bin
MAN_DIR = $(PREFIX)/man

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ *.bak core
distclean: clean
	rm -f bchunk

install: installbin installman
installbin:
	$(INSTALL) -m 755 -s -o root -g root bchunk		$(BIN_DIR)
installman:
	$(INSTALL) -m 644 -o bin -g bin bchunk.1	 	$(MAN_DIR)/man1

BITS = bchunk.o

bchunk: $(BITS)
	$(LD) $(LDFLAGS) -o bchunk $(BITS)

bchunk.o:	bchunk.c

