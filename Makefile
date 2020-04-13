CC = gcc

all: clean makebinaries dumb_client dumb_Nclient dumb_Pserver dumb_Tserver dumb_Sserver dumb_Nserver

makebinaries:
	mkdir binaries

dumb_client: clients/client.c
	$(CC) $< -o binaries/$@ 

dumb_Nclient: clients/using_ncurses/client.c
	$(CC)  $< -o binaries/$@ -lncurses
	




dumb_Pserver: servers/using_conditional_compilation/server.c
	$(CC) -o binaries/$@ $< -D PROCESS

dumb_Tserver: servers/using_conditional_compilation/server.c
	$(CC) -o binaries/$@ $< -D THREAD -lpthread

dumb_Sserver:
	cd servers/using_select && make

dumb_Nserver: servers/using_ncurses/server.c
		$(CC) -o binaries/$@ $< -lncurses


clean:
	rm  -r binaries




