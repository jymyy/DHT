CFLAGS=-c -g -Wall -std=c99 -D_POSIX_SOURCE -pedantic
LDFLAGS=-lssl -lcrypto
CC = gcc
LD = gcc
OBJS = dhtnode.o dhtpacket.o hash.o socketio.o
PROG = dhtnode

.c.o:
	gcc $< -o $@ $(CFLAGS)

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

dhtnode.o: dhtnode.c dhtpackettypes.h dhtpacket.h hash.h typedefs.h socketio.h
	$(CC) $(CFLAGS) dhtnode.c 

dhtpacket.o: dhtpacket.c
	$(CC) $(CFLAGS) dhtpacket.c

hash.o: hash.c 
	$(CC) $(CFLAGS) hash.c

socketio.o: socketio.c
	$(CC) $(CFLAGS) socketio.c

clean:
	/bin/rm -f *.o dhtnode

# These are for testing
hash_test: hash_test.o hash.o
	$(LD) hash_test.o hash.o -o hash $(LDFLAGS)

hash_test.o: hash_test.c
	$(CC) $(CFLAGS) hash_test.c

