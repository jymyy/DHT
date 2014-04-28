CFLAGS=-c -g -Wall -std=c99 -D_POSIX_SOURCE -pedantic
LDFLAGS=-lssl -lcrypto
CC = gcc
LD = gcc
OUTDIR = build
PROG = $(OUTDIR)/dhtnode
OBJS = dhtnode.o dhtpacket.o hash.o socketio.o fileio.o keyring.o cmdpacket.o
OUTOBJS = $(addprefix $(OUTDIR)/,$(OBJS))


all: $(PROG)

$(PROG): $(OUTOBJS)
	$(LD) $(OUTOBJS) -o $(PROG) $(LDFLAGS)

$(OUTDIR)/%.o: %.c | mkdir
	$(CC) -c $(CFLAGS) $< -o $@

run: $(PROG)
	$(PROG)

mkdir:
	/bin/mkdir -p $(OUTDIR)

clean:
	/bin/rm -f $(OUTDIR)/*.o $(OUTDIR)/dhtnode