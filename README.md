# DUMB-SERVER [![Build Status](https://travis-ci.org/deep5050/dumb-server.svg?branch=master)](https://travis-ci.org/deep5050/dumb-server) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/8053bfdc3a4a4ca8bcbb28a848c3f2f3)](https://www.codacy.com/manual/dipankarpal5050/dumb-server?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=deep5050/dumb-server&amp;utm_campaign=Badge_Grade) 
[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/deep5050/dumb-server)

Simple client-server applications implemented in different ways:

![output](/screenshots/output.png)

1. Using pthread()
2. Using fork()
3. Using select()
4. Using Conditional Compilation Method
5. Using Ncurses Library

# USAGE

## Server using select()
server: 

	file:  dumb_Sserver
	usage: ./dumb_Sserver <port> <backlog>

client: 

	file:  dumb_client
	usage: ./dumb_client <serveraddress> <port>
	options: <serveraddreess> type '0' or 'localhost' to connect to the local server else specify.


## Multi-process server
server: 

	file:  dumb_Pserver
	usage: ./dumb_Pserver <port> <backlog>

client: 

	file:  dumb_client
	usage: ./dumb_client <serveraddress> <port>
	options: <serveraddreess> type '0' or 'localhost' to connect to the local server else specify.


## Multi-thread server 
server:

	file:  dumb_Tserver
	usage: ./dumb_Tserver <port> <backlog>


client:

	file:  dumb_client
	usage: ./dumb_client <serveraddreess> <port>
	options: <serveraddreess> type '0' or 'localhost' to connect to the local server else specify.

	
## Realtime server (BUGS)
A realtime ( charactar by charactar ) server-client system that mimics talnet's behaviour.

>BUGS:
>1. more than 1 clients will try to print on the same >terminal causing chaos ( could not make it to force each >new child to write on its own terminal)
>2. BACKSPACE on the server side not working
>3. server prints the character it got during the last recieve() ( 1 char delay )

server:

	file:  dumb_Nserver
	usage: ./dumb_Nserver <port> <backlog>

client:

	file:  dumb_Nclient
	usage: ./dumb_Nclient <port>

