# networks
Networks projects

Directory Structure
---------------------------------------------------------------------------------------------
hello: a binary that prints "hello networking"

hello.c: is the code for that

receive: is a subdirectory for receiving files for the client side
	
	This is where the binary gets tranferred to.

	client: is the executable for the client side for TCP
		usage: ./client servername(i.e. blanca,shavano, etc.) filename(i.e. hello)
	

server: is the server executable for TCP

server_side.c: is that code

To compile file: 
---------------------------------------------------------------------------------------------
1. cd to client folder: gcc -o tcp_client tcp_client.o
2. cd to server folder: gcc -o tcp_server tcp_server.c

To test:
---------------------------------------------------------------------------------------------
1. cd to server folder: ./tcp_server
2. In another terminal, cd to client folder: ./tcp_client [hostname] HW.pdf
3. Go to receive folder
4. Verify HW.pdf exists

