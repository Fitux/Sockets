#Makefile

all: time_server client

client: client.o
	gcc -Wall -O2 client.o -o client

time_server: time_server.o
	gcc -Wall -O2 time_server.o -o time_server

client.o: client.c
	gcc -Wall -O2 -c client.c -o client.o

time_server.o: time_server.c
	gcc -Wall -O2 -c time_server.c -o time_server.o

clean:
	rm -rf client time_server client.o time_server.o

push:
	git add . 
	git commit -m 'Commit from make'
	git push