all: clear client.out server.out

client.out: src/client.c src/common.c 
	g++ -Wall src/client.c src/common.c -o ./client.out

server.out: src/server.c 
	g++ -Wall src/server.c -o ./server.out

clear:
	clear
	rm *.out