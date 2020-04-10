CC = gcc

all: dumb_client dumb_Pserver dumb_Tserver 

dumb_client: client.c
	$(CC) -o $@ $<

dumb_Pserver: server.c
	$(CC) -o $@ $< -D PROCESS

dumb_Tserver: server.c
	$(CC) -o $@ $< -D THREAD -lpthread

clean:
	rm dumb_*





