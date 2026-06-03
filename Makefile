CC = gcc
CFLAGS = -Wall -Wextra -g
APPS = reme remed

all: reme remed
	pkill remed || true

remed: remed.o socket.o daemonize.o crud.o sleeper.o

reme: reme.o crud.o daemonize.o socket.o sleeper.o
	$(CC) $(CFLAGS) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f *.o $(APPS)

