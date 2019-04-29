CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -std=c99 -g

all: server client

debug_server: CFLAGS+=-DDEBUG
debug_server: server

debug_client: CFLAGS+=-DDEBUG
debug_client: client

server: server.o util_server.o
	$(CC) $(CFLAGS) $^ -o $@

client: client.o util_client.o
	$(CC) $(CFLAGS) $^ -o client

%.o:%.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o
