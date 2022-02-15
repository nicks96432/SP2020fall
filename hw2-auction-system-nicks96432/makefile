CC = gcc

all: host player

host: host.c
	$(CC) host.c -o host

player: player.c
	$(CC) player.c -o player

.PHONY: clean
clean:
	rm host player