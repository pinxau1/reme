CC = gcc
CFLAGS = -Wall -Wextra -g
PTHREAD = -pthread
APPS = reme remed

all: reme remed
	pkill remed || true

remed: remed.o socket.o daemonize.o crud.o sleeper.o
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $^

reme: reme.o crud.o socket.o
	$(CC) $(CFLAGS) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f *.o $(APPS)
