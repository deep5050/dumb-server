--------------------- multi-process server ------------
server: 
	file:  dumb_Pserver
	usage: ./dumb_Pserver <port> <backlog>

client: 
	file:  dumb_client
	usage: ./dumb_client <serveraddress> <port>

		<serveraddreess> type '0' or 'localhost' to connect to the local server else specify


--------------------- multi-threaded server -------------
server:
	file:  dumb_Tserver
	usage: ./dumb_Tserver <port> <backlog>


client:
	file:  dumb_client
	usage: ./dumb_client <serveraddreess> <port>

		<serveraddreess> type '0' or 'localhost' to connect to the local server else specify

	
------------------- realtime server (BUG) ---------------
BUGS:
1. more than 1 clients will try to print on the same terminal causing chaos ( could not make it to force each new child to write on its own terminal)
2. BACKSPACE on the server side not working
3. server prints the character it got during the last recieve() ( 1 char delay )

server:
	file:  dipankar_ncurses_server.c
	usage: ./dip_Nserver <port> <backlog>

client:
	file:  dipankar_ncurses_client.c
	usage: ./dip_Nclient <port>

