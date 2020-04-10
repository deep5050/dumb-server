
CC = gcc

all: dumb_Nclient dumb_Nserver

dumb_Nserver: ncurses_server.c
	$(CC) -o $@ $< -lncurses

dumb_Nclient: ncurses_client.c
	$(CC) -o $@ $< -lncurses
clean:
	rm dumb_*
