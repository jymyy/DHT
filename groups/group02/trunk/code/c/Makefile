CFLAGS=-c -g -Wall -std=c99 -D_POSIX_SOURCE -pedantic
LDFLAGS=-lssl -lcrypto
CC = gcc
LD = gcc
OBJS = dhtnode.o dhtpacket.o hash.o socketio.o fileio.o keyring.o cmdpacket.o
PROG = dhtnode

.c.o:
	gcc $< -o $@ $(CFLAGS)

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(OBJS) -o $(PROG) $(LDFLAGS)

dhtnode.o: dhtnode.c
	$(CC) $(CFLAGS) dhtnode.c 

dhtpacket.o: dhtpacket.c
	$(CC) $(CFLAGS) dhtpacket.c

hash.o: hash.c 
	$(CC) $(CFLAGS) hash.c

socketio.o: socketio.c
	$(CC) $(CFLAGS) socketio.c

fileio.o: fileio.c
	$(CC) $(CFLAGS) fileio.c

keyring.o: keyring.c
	$(CC) $(CFLAGS) keyring.c

cmdpacket.o: cmdpacket.c
	$(CC) $(CFLAGS) cmdpacket.c

clean:
	/bin/rm -f *.o dhtnode keyring hash


# These are for testing
hash_test: hash_test.o hash.o
	$(LD) hash_test.o hash.o -o hash $(LDFLAGS)

hash_test.o: hash_test.c
	$(CC) $(CFLAGS) hash_test.c

