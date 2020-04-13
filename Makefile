CC = gcc

all: makebin dumb_client dumb_Nclient dumb_Pserver dumb_Tserver dumb_Sserver dumb_Nserver

makebin:
	mkdir bin
	
dumb_client: clients/client.c
	$(CC) $< -o bin/$@ 

dumb_Nclient: clients/using_ncurses/client.c
	$(CC)  $< -o bin/$@ -lncurses
	




dumb_Pserver: servers/using_conditional_compilation/server.c
	$(CC) -o bin/$@ $< -D PROCESS

dumb_Tserver: servers/using_conditional_compilation/server.c
	$(CC) -o bin/$@ $< -D THREAD -lpthread

dumb_Sserver:
	cd servers/using_select && make

dumb_Nserver: servers/using_ncurses/server.c
		$(CC) -o bin/$@ $< -lncurses


clean:
	rm bin/dumb_*





