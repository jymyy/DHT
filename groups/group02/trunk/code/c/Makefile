CFLAGS=-c -g -Wall -Werror -D_POSIX_SOURCE -std=c99 -pedantic
LDFLAGS=
CC = gcc
LD = gcc
OBJS = dhtnode.o dhtpacket.o
PROG = dhtnode

.c.o:
	gcc $< -o $@ $(CFLAGS)

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

dhtnode.o: dhtnode.c dhtpackettypes.h dhtpacket.h hash.h typedefs.h socketio.h
	$(CC) $(CFLAGS) dhtnode.c 

dhtpacket.o: dhtpacket.c
	$(CC) $(CFLAGS) dhtpacket.c

clean:
	/bin/rm -f *.o dhtnode

