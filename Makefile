PREFIX ?= $(HOME)/.local
SYSTEMD_DIR ?= $(HOME)/.config/systemd/user
CC = gcc
CFLAGS = -Wall -Wextra -g
PTHREAD = -pthread
APPS = reme remed

install: all
	mkdir -p $(PREFIX)/bin
	cp remed $(PREFIX)/bin/remed
	cp remed.service $(SYSTEMD_DIR)/remed.service
	systemctl --user daemon-reload

enable:
	systemctl --user enable --now remed

disable: 
	systemctl --user disable --now remed

uninstall: 
	systemctl --user disable --now remed || true
	rm -f $(PREFIX)/bin/remed
	rm -f $(SYSTEMD_DIR)/remed.service
	systemctl --user daemon-reload

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
