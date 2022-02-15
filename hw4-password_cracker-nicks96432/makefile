CC ?= gcc
CFLAGS ?= -march=native -pthread -lcrypto -Wall -Wno-pointer-sign -Ofast
cracker: cracker.c
	$(CC) $^ -o $@ $(CFLAGS) 

.PHONY:clean
clean:
	@-rm -rf cracker